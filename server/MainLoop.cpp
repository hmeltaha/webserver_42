#include "MainLoop.hpp"


MainLoop::MainLoop(const std::vector<ServerConfig>& configs)
{
	for (size_t i = 0; i < configs.size(); ++i)
	{
		servers.push_back(Server(configs[i], i));
		servers[i].setSockets();
	}
	epollFD = -1;
}

MainLoop::~MainLoop()
{

}

/**
 * EPOLLIN -->  جديد حاول الاتصال بالسيرفر Client
 * يوجد اتصال جديد ينتظر accept()
 */

void MainLoop::start()
{
	epollFD = epoll_create1(0);
	if (epollFD == -1)
		throw std::runtime_error("Failed to create epoll instance\n");
	struct epoll_event event;
	
	for (size_t i = 0; i < servers.size(); ++i)
	{
		// add sockets fds to epoll
		// EPOLLIN = يوجد شخص يدق الباب if server fd 
		// EPOLLIN = الشخص يتكلم (يرسل request) if client fd
		int fd = servers[i].getSocketFd();
		std::cout << "Trying to add server fd: " << fd << std::endl;
		event.events = EPOLLIN;
		event.data.fd = servers[i].getSocketFd();
		// EPOLL_CTL_ADD --> إضافة fd جديد للمراقبة
		if (epoll_ctl(epollFD, EPOLL_CTL_ADD, servers[i].getSocketFd(), &event) == -1)
		{
			// perror("epoll_ctl failed");
			throw std::runtime_error("Failed to add server socket to epoll\n");
		}
	}
	int num = 2000;//servers.size() * 100;
	std::set<int> serverfds;
	for (size_t i = 0; i < servers.size(); i++)
		serverfds.insert(servers[i].getSocketFd());
	while (true)
	{
		struct epoll_event events[num];
		int numEvents = epoll_wait(epollFD, events, num, -1);
		if (numEvents == -1)
			throw std::runtime_error("Failed to wait on epoll\n");

		for (int i = 0; i < numEvents; ++i)
		{
			int fd = events[i].data.fd;
			if (serverfds.find(fd) != serverfds.end())
			{
				int clientFd = accept(fd, NULL, NULL);
				if (clientFd == -1)
					throw std::runtime_error("Failed to accept client connection\n");
				event.events = EPOLLIN;
				event.data.fd = clientFd;

				// clients.push_back(Client(clientFd));

				clients[clientFd] = Client(clientFd);
				if (epoll_ctl(epollFD, EPOLL_CTL_ADD, clientFd, &event) == -1)
					throw std::runtime_error("Failed to add client socket to epoll\n");
				int flags = fcntl(clientFd, F_GETFL, 0);
				if (flags == -1)
					throw std::runtime_error("fcntl failure\n");
				if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1)
					throw std::runtime_error("fcntl set non-blocking failure\n");
				continue;
			}
			else
			{
				if (events[i].events & (EPOLLHUP | EPOLLERR))
				{
					close(fd);
					continue;
				}
				if (events[i].events & EPOLLIN)
				{
					char buff[1025];
					int flag = read(fd, buff, sizeof (buff));
					if (flag <= 0)
					{
						close (fd);
						continue;
					}
					buff[flag] = '\0';
					clients[fd].setState(READING_HEADERS_BODY);
						clients[fd].addToReqBuff(buff);
					// std::cout << "hi\n";
					if (clients[fd].getHeadersReceived())
					{
						std::cout << "Received request:\n" << clients[fd].getReqBuff() << std::endl;
						// clients[fd].setState(PROCESSING);
						// std::string response =
						// 		"HTTP/1.1 200 OK\r\n"
						// 		"Content-Length: 13\r\n"
						// 		"Connection: close\r\n"
						// 		"\r\n"
						// 		"Hello, world!";
						// 	clients[fd].setResBuff(response);
						
						// clients[fd].setState(SENDING_RESPONSE);
						// clients[fd].setResBuff(response);
						// event.events = EPOLLOUT | EPOLLET; // Edge-triggered for writing
						// event.data.fd = fd;
						// if (epoll_ctl(epollFD, EPOLL_CTL_MOD, fd, &event) == -1)
						// 	throw std::runtime_error("Failed to modify client socket to epoll\n");
						// clients.erase(fd);
						// close(fd);
						/**
						 * parse the request and 
						 * generate the response then
						 * send response
						 */
					}

					// if (events[i].events & EPOLLOUT)
					// {
					// 	std::string res = clients[fd].getResBuff();
					// 	int sent = send(fd, res.c_str(), res.size(), 0);
					// 	if (sent == -1)
					// 	{
					// 		close(fd);
					// 		continue;
					// 	}
					// res.erase(0, sent);
					// 	if (res.empty())
						// {
						// 	// all data sent
						// 	close(fd);
						// 	clients.erase(fd);
						// }
					// }
				}
			}
		}
	}
}