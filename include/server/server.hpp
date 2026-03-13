#ifndef SERVER_HPP
#define SERVER_HPP

#include "parser/ServerConfig.hpp"
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
#include <unistd.h>
#include <stdio.h>

class Server
{
        private:
                ServerConfig config;
                int serverFd;
                sockaddr_in address;
                int index;
        public:
                Server(const ServerConfig& config, int index);
                ~Server();
                int getSocketFd() const;
                int getIndex() const;
                void setSockets();
};

#endif
