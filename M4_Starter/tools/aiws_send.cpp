#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <poll.h>
#include <string>
#include <sys/un.h>
#include <unistd.h>
#include <sys/socket.h>
#include <cstdlib>
#include <cstdint>
#include <exception>


namespace {
	using Clock = std::chrono::steady_clock;
	constexpr uint32_t max_path = 1024 * 1024;

	bool transfer(int fd, void* buffer, size_t size, bool sending, Clock::time_point deadline)
	{
		auto* p = static_cast<unsigned char*>(buffer);
		while (size)
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

			if (count < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
			{
				continue;
			}
			return false;
		}
		return true;
	}

	std::string default_socket()
	{
		return "/tmp/aiws-" + std::to_string(::getuid()) + ".sock";
	}


}

int main(int argc, char** argv)
{
	std::string socket_path = default_socket();
	int index = 1;
	if (index < argc && std::string(argv[index]) == "--socket")
	{
		if (index + 1 >= argc)
		{
			std::cerr << "Missing socket path\n";
			return 1;
		}
		socket_path = argv[index + 1];
		index += 2;
	}
	if (argc - index != 2 || std::string(argv[index]) != "add")
	{
		std::cerr << "Usage: aiws_send [--socket PATH] add FILE\n";
		return 1;
	}
	std::string path;

	try
	{
		path = std::filesystem::absolute(argv[index + 1]).string();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}
	sockaddr_un address{};
	if (path.empty() || path.size() > max_path || socket_path.size() >= sizeof(address.sun_path))
	{
		std::cerr << "Invalid path length\n";
		return 1;
	}
	int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (fd < 0)
	{
		return 2;
	}
	address.sun_family = AF_UNIX;
	std::memcpy(address.sun_path, socket_path.c_str(), socket_path.size() + 1);
	auto connect_deadline = Clock::now() + std::chrono::seconds(2);
	int rc = ::connect(
		fd,
		reinterpret_cast<sockaddr*>(&address),
		sizeof(address)
	);
	if (rc < 0 && errno == EINPROGRESS)
	{
		pollfd event{ fd, POLLOUT, 0 };

		rc = -1;
		while (Clock::now() < connect_deadline)
		{
			int n = ::poll(&event, 1, 50);
			if (n < 0)
			{
				if (errno == EINTR)
				{
					continue;
				}
				break;
			}
			if (n == 0)
			{
				continue;
			}
			int error = 0;
			socklen_t len = sizeof(error);
			if (::getsockopt(
				fd,
				SOL_SOCKET,
				SO_ERROR,
				&error,
				&len) == 0 && error == 0)
			{
				rc = 0;
			}
			break;
		}
	}
	if (rc < 0)
	{
		std::cerr << "No application could be reached\n";
		::close(fd);
		return 2;
	}
	auto deadline = Clock::now() + std::chrono::seconds(5);
	uint32_t length = htonl(static_cast<uint32_t>(path.size()));
	unsigned char status = 0;
	uint32_t error_length = 0;
	bool ok =
		transfer(fd, &length, 4, true, deadline) &&
		transfer(fd, path.data(), path.size(), true, deadline) &&
		transfer(fd, &status, 1, false, deadline) &&
		transfer(fd, &error_length, 4, false, deadline);
	std::string error;
	if (ok)
	{
		uint32_t size = ntohl(error_length);
		if (size > 65536)
		{
			ok = false;
		}
		else
		{
			error.resize(size);
			if (size)
			{
				ok = transfer(fd, error.data(), size, false, deadline);
			}
		}
	}
	::close(fd);
	if (!ok || status > 1)
	{
		std::cerr << "Communication failed\n";
		return 2;
	}
	if (status == 1)
	{
		std::cerr << (error.empty() ? "Request rejected" : error) << '\n';
		return 1;
	}
	std::cout << "Document accepted\n";
	return 0;
}




