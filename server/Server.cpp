#include "Server.hpp"

Server::Server(const ServerConfig& config, int index)
{
	this->config = config;
	this->index = index;
}
Server::~Server()
{

}

const ServerConfig& Server::getConfig() const
{
	return config;
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
		throw std::runtime_error(strerror(errno));
	 /**
	 * setsocket() --> This allows the program to reuse a port immediately after it closes.
	 * becouse affter cloesing the server the port go to state called "TIME_WAIT".
	 * it is like : setsockopt(socket, WHERE, WHAT_OPTION, VALUE, SIZE)
	 */
	int opt = 1;
	// Set SO_REUSEADDR to allow reuse of ports in TIME_WAIT state
	if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error(strerror(errno));

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(config.listen_directives[index].port);
	// address.sin_addr.s_add = INADDR_ANY;
	address.sin_addr.s_addr = INADDR_ANY;//inet_addr(config.listen_directives[index].host.c_str());
	if (bind(serverFd,(struct sockaddr*) &address, sizeof(address)) < 0)
	{
		close(serverFd);
		throw std::runtime_error(strerror(errno));
	}
	if (listen(serverFd, 5000) < 0)
		throw std::runtime_error(strerror(errno));

	// Make the socket non-blocking
	int flags = fcntl(serverFd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error(strerror(errno));
	if (fcntl(serverFd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error(strerror(errno));

	// int newFlags = fcntl(serverFd, F_GETFL, 0);
	// if (newFlags == -1)
	//     throw std::runtime_error("fcntl get after set failure\n");

	// if (!(newFlags & O_NONBLOCK)) {
	//     throw std::runtime_error("Socket is still blocking!");
	// }
}

int Server::getSocketFd() const
{
	return serverFd;
}

int Server::getIndex() const
{
	return index;
}
