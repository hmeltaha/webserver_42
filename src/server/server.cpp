#include "server/server.hpp"

Server::Server(const ServerConfig& config, int index)
{
        this->config = config;
        this->index = index;
}

Server::~Server()
{
}

/***
 * inet_addr --> converts string IP into 32-bit integer in network byte order
 * htons     --> Host TO Network Short, converts port to network byte order
 */
void Server::setSockets()
{
        serverFd = socket(AF_INET, SOCK_STREAM, 0);
        if (serverFd == -1)
                throw std::runtime_error("Socket failure\n");

        int opt = 1;
        if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0)
                throw std::runtime_error("setsockopt SO_REUSEADDR failure\n");

        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(config.listen_directives[index].port);
        address.sin_addr.s_addr = INADDR_ANY;

        if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0)
                throw std::runtime_error("bind failure\n");
        if (listen(serverFd, 5000) < 0)
                throw std::runtime_error("listen failure\n");

        int flags = fcntl(serverFd, F_GETFL, 0);
        if (flags == -1)
                throw std::runtime_error("fcntl failure\n");
        if (fcntl(serverFd, F_SETFL, flags | O_NONBLOCK) == -1)
                throw std::runtime_error("fcntl set non-blocking failure\n");
}

int Server::getSocketFd() const
{
        return serverFd;
}

int Server::getIndex() const
{
        return index;
}
