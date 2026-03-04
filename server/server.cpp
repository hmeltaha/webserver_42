#include "Server.hpp"

Server::Server(const ServerConfig& config, int index)
{
	this->config = config;
	this->index = index;
}
Server::~Server()
{

}

/***
 * inet_addr --> this function convert from the string IP into 
 * 32-bits integer in network byte order
 * 
 * ex: Length: "192.168.1.10" → 11 characters + 1 null terminator → 12 bytes
 * After inet_addr(ip.c_str()): Converts to a 32-bit integer (IPv4) → 4 bytes
 * always stored in network (big-endian) order.
 * 
 * 
 * htons --> Host TO Network Short, int convert from machine host byte 
 * order to network byte order;
 * 
 * ex: before 16bit -> 2 bytes
 * After htons : still 2 bytes but the order is diff, it swapped if the host is little-endian
 * 
 */

void Server::setSockets()
{
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd == -1)
		throw std::runtime_error("Socket failure\n");
	 /**
	 * setsocket() --> This allows the program to reuse a port immediately after it closes.
	 * becouse affter cloesing the server the port go to state called "TIME_WAIT". 
	 * it is like : setsockopt(socket, WHERE, WHAT_OPTION, VALUE, SIZE)
	 */
	int opt = 1;
	// Set SO_REUSEADDR to allow reuse of ports in TIME_WAIT state
	if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0)
		throw std::runtime_error("setsockopt SO_REUSEADDR failure\n");
	
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(config.listen_directives[index].port);
	// address.sin_addr.s_add = INADDR_ANY;
	address.sin_addr.s_addr = INADDR_ANY;//inet_addr(config.listen_directives[index].host.c_str());
	if (bind(serverFd,(struct sockaddr*) &address, sizeof(address)) < 0)
	{
		perror("bind failed");
		throw std::runtime_error("bind failure\n");
	}
	if (listen(serverFd, 5000) < 0)
		throw std::runtime_error("listen failure\n");
	
	// Make the socket non-blocking
	int flags = fcntl(serverFd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("fcntl failure\n");
	if (fcntl(serverFd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("fcntl set non-blocking failure\n");

	// int clientFd = accept(serverFd,(struct sockaddr*) &address, sizeof(address) );
	// if (clientFd < 0)
	// 	throw std::runtime_error("accept failure\n");
}

int Server::getSocketFd() const
{
	return serverFd;
}

int Server::getIndex() const
{
	return index;
}

// void Server::serverLoop()
// {
// 	//strat epoll loop to handle multiple clients on the same server socket
// 	int epollFd = epoll_create1(0);
// 	if (epollFd == -1)
// 		throw std::runtime_error("epoll failure\n");
// 	struct epoll_event event;
// 	event.events = EPOLLIN;
// 	event.data.fd = serverFd;
// 	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &event) == -1)
// 		throw std::runtime_error("epoll_ctl failure\n");

// }