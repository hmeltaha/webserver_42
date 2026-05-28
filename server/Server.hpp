#ifndef SERVER_HPP
#define SERVER_HPP

#include "../parser/ServerConfig.hpp"
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <poll.h>
#include <set>
#include <sys/epoll.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <stdio.h>

class Server
{
	private:
		ServerConfig config;
		std::vector<int> serverFds;
		sockaddr_in address;
		int index;
	public:
		Server(const ServerConfig& config, int index);
		~Server();
		int getSocketFd() const;
		std::vector<int> getSocketFds() const;
		int getIndex() const;
		void loopSetSockets();
		void setSockets(int port);
		const ServerConfig& getConfig() const;
};

#endif