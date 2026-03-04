#ifndef MAIN_LOOP_HPP
#define MAIN_LOOP_HPP

#include "Server.hpp"
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>

class MainLoop
{
	private:
		int epollFD;
		std::vector<Server> servers;
		// std::vector<Client> clients;
	public:
		MainLoop(const std::vector<ServerConfig>& configs);
		~MainLoop();
		void start();
};


#endif