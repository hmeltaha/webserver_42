#ifndef SERVER_HPP
#define SERVER_HPP

#include "parser/ServerConfig.hpp"
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>

class Server
{
	private:
		ServerConfig config;
		int serverFd;
		// address server_address;
		sockaddr_in address;
		int index;
	public:
		Server(const ServerConfig& config, int index);
		~Server();
		// void start();
		int getSocket() const;
		int getIndex() const;
		void setSockets();
		void serverLoop();
};

#endif