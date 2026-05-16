#ifndef MAIN_LOOP_HPP
#define MAIN_LOOP_HPP

#define CHUNK_SIZE 1000
#define MAX_EVENTS 2000
#define TIMEOUT 30
#include "Server.hpp"
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdexcept>
#include "Client.hpp"
#include <cerrno>
#include "../requests/HttpRequest.hpp"
#include "../requests/RequestParser.hpp"
#include "../signals/SignalHandler.hpp"
#include "../handlers/FileHandler.hpp"
#include "../router/Router.hpp"
#include "../handlers/FileResponse.hpp"
#include "../response/HttpResponse.hpp"
extern bool running;


class MainLoop
{
	private:
		int epollFD;
		std::vector<Server> servers;
		std::map<int, Client> clients;
		std::map<int, int> serverTOClient;
		struct epoll_event events[MAX_EVENTS];
	public:
		MainLoop();
		MainLoop(const std::vector<ServerConfig>& configs);
		~MainLoop();
		void addNewClients(int fd, int server_index);
		void createEpoll();
		void handleClientEpollIn(int fd);
		void handleClientEpollOut(int fd);
		void setServers(const std::vector<ServerConfig>& configs);
		void closeFds();
		void checkTimeout();
		void start();
};


#endif
