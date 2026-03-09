#ifndef MAIN_LOOP_HPP
#define MAIN_LOOP_HPP

#include "Server.hpp"
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>
#include "Client.hpp"
#define MAX_EVENTS 2000

class MainLoop
{
	private:
		int epollFD;
		std::vector<Server> servers;
		std::map<int, Client> clients;
		struct epoll_event events[MAX_EVENTS];
	public:
		MainLoop(const std::vector<ServerConfig>& configs);
		~MainLoop();
		void addNewClients(int fd);
		void createEpoll();
		void handleClientEpollIn(int fd);
		void handleClientEpollOut(int fd);
		void start();
};


#endif