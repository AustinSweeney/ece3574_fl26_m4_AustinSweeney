#include "aiws/ipc_server.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <utility>


namespace aiws {
	namespace {
		using Clock = std::chrono::steady_clock;
		constexpr uint32_t max_path = 1024 * 1024;
		bool transfer(int fd, void* buffer, size_t size, bool sending,
			const std::atomic<bool>& running, Clock::time_point deadline)
		{
			auto* p = static_cast<unsigned char*>(buffer);

			while (size && running.load())
			{
				if (Clock::now() >= deadline)
				{
					return false;
				}

				pollfd event{ fd, static_cast<short>(sending ? POLLOUT : POLLIN), 0 };
				int n = ::poll(&event, 1, 50);

				if (n < 0)
				{
					if (errno == EINTR)
					{
						continue;
					}
					return false;
				}

				if (n == 0)
				{
					continue;
				}

				if (event.revents & (POLLERR | POLLNVAL))
				{
					return false;
				}

				ssize_t count = sending
					? ::send(fd, p, size, MSG_NOSIGNAL | MSG_DONTWAIT)
					: ::recv(fd, p, size, MSG_DONTWAIT);

				if (count > 0)
				{
					p += count;
					size -= static_cast<size_t>(count);
					continue;
				}

				if (count < 0 &&
					(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
				{
					continue;
				}

				return false;
			}

			return size == 0;

		}
		bool readable_regular_file(const std::string& path)
		{
			struct stat st {};
			if (::stat(path.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
			{
				return false;
			}
			int fd = ::open(path.c_str(), O_RDONLY | O_CLOEXEC | O_NONBLOCK);
			if (fd < 0)
			{
				return false;
			}
			::close(fd);
			return true;
		}
	}
		IpcServer::~IpcServer()
		{
			stop();
		}

		bool IpcServer::start(const std::string& socket_path, AddCallback callback)
		{
			sockaddr_un address{};

			if (running_ || socket_path.empty() ||
				socket_path.size() >= sizeof(address.sun_path))
			{
				return false;
			}

			int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);

			if (fd < 0)
			{
				return false;
			}

			address.sun_family = AF_UNIX;
			std::memcpy(address.sun_path, socket_path.c_str(), socket_path.size() + 1);

			if (::bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0)
			{
				::close(fd);
				return false;
			}

			if (::listen(fd, SOMAXCONN) != 0)
			{
				::close(fd);
				::unlink(socket_path.c_str());
				return false;
			}

			socket_path_ = socket_path;
			callback_ = std::move(callback);
			listen_fd_ = fd;
			running_ = true;

			thread_ = std::thread(&IpcServer::run, this);

			return true;
		}
		void IpcServer::stop() {
			running_ = false;
			if (thread_.joinable())
			{
				thread_.join();
			}
			if (listen_fd_ >= 0)
			{
				::close(listen_fd_);
				listen_fd_ = -1;
			}

			if (!socket_path_.empty())
			{
				::unlink(socket_path_.c_str());
				socket_path_.clear();
			}

		}
		void IpcServer::run()
		{
			while (running_)
			{
				pollfd event
				{
					listen_fd_, POLLIN, 0
				};
				int n = ::poll(&event, 1, 50);
				if (n <= 0)
				{
					continue;
				}
				if (!(event.revents & POLLIN))
				{
					continue;
				}
				int client = ::accept4(listen_fd_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
				if (client < 0)
				{
					continue;
				}
				handle_client(client);
				::close(client);
			}
		}
		void IpcServer::handle_client(int client)
		{
			const auto deadline = Clock::now() + std::chrono::seconds(5);
			uint32_t network_size = 0;
			if (!transfer(client, &network_size, sizeof(network_size), false, running_, deadline))
			{
				return;
			}
			uint32_t size = ntohl(network_size);
			if (size == 0 || size > max_path)
			{
				return;
			}
			std::string path(size, '\0');
			if (!transfer(client, path.data(), size, false, running_, deadline))
			{
				return;
			}

			bool valid = path.find('\0') == std::string::npos && readable_regular_file(path);
			const std::string error = valid ? "" : "File is not readable";
			unsigned char status = valid ? 0 : 1;
			uint32_t  error_size = htonl(static_cast<uint32_t>(error.size()));
			if (!transfer(client, &status, 1, true, running_, deadline) ||
				!transfer(client, &error_size, 4, true, running_, deadline) ||
				(!error.empty() && !transfer(client, const_cast<char*>(error.data()), error.size(), true, running_, deadline)))
			{
				return;

			}
			if (valid && running_)
			{
				callback_(path);
			}
		}
	

}