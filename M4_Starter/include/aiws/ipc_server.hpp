#pragma once
#include <atomic>
#include <string>
#include <thread>
#include <cstdint>
#include <functional>


namespace aiws {
	class IpcServer
	{
	public:
		using AddCallback = std::function<void(const std::string&)>;
		IpcServer() = default;
		
		~IpcServer();

		IpcServer(const IpcServer&) = delete;
		IpcServer& operator=(const IpcServer&) = delete;
		bool start(const std::string& socket_path, AddCallback callback);
		void stop();
	private:
		void run();
		void handle_client(int client_fd);
		std::thread thread_;
		std::atomic<bool> running_{ false };
		int listen_fd_{ -1 };
		std::string socket_path_;
		AddCallback callback_;

	};
}