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


void MainLoop::addNewClients(int fd)
{
	int clientFd = accept(fd, NULL, NULL);
	if (clientFd == -1)
		throw std::runtime_error("Failed to accept client connection\n");
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = clientFd;
	clients[clientFd] = Client(clientFd);
	if (epoll_ctl(epollFD, EPOLL_CTL_ADD, clientFd, &event) == -1)
		throw std::runtime_error("Failed to add client socket to epoll\n");
	int flags = fcntl(clientFd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error("fcntl failure\n");
	if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error("fcntl set non-blocking failure\n");
}


void MainLoop::handleClientEpollIn(int fd)
{
	char buff[1025];
	int flag = read(fd, buff, sizeof (buff));
	if (flag <= 0)
	{
		close (fd);
		clients.erase(fd);
		return;
	}
	buff[flag] = '\0';
	clients[fd].setState(READING);

	//if the req have a body "Content-Length"
	clients[fd].addToReqBuff(buff);
	if (clients[fd].getState() == READING_BODY)
		clients[fd].addBodyToReq();
	std::cout << "Received request:\n" << clients[fd].getReqBuff() << std::endl;
	
	if (clients[fd].getState() == PROCESSING)
	{
		////////////////////////////////////////////////////////
		// std::cout << "Received request:\n" << clients[fd].getReqBuff() << std::endl;
		RequestParser parser;
		HttpRequest req = parser.parse(clients[fd].getReqBuff());

		std::cout << "Method: " << req.method << std::endl;
		std::cout << "Path: " << req.path << std::endl;
		std::cout << "Version: " << req.version << std::endl;

		for (std::map<std::string, std::string>::iterator it = req.headers.begin();
			it != req.headers.end(); ++it)
		{
			std::cout << it->first << " -> " << it->second << std::endl;
		}

		std::cout << "just for testing" << std::endl;
		std::string response =
				"HTTP/1.1 200 OK\r\n"
				"Content-Length: 12\r\n"
				"Connection: close\r\n"
				"\r\n"
				"hi from heba";
				std::cout << response;
			clients[fd].setResBuff(response);
		////////////////////////////////////////////////////////
		/**
		 * parse the request and routing then (hala and nora)
		 * generate the response then
		 * send response
		 */
		
		clients[fd].setState(WRITING);
		// clients[fd].setResBuff(response);
		struct epoll_event ev;
		ev.events = EPOLLOUT ;
		ev.data.fd = fd;
		if (epoll_ctl(epollFD, EPOLL_CTL_MOD, fd, &ev) == -1)
			throw std::runtime_error("Failed to modify client socket to epoll\n");
		// std::cout << "changing the event to epolout: " << fd << std::endl;
	}
}


void MainLoop::createEpoll()
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
}

void MainLoop::handleClientEpollOut(int fd)
{
	std::cout << "enter the epollout " << std::endl;
	std::string res = clients[fd].getResBuff();
	int sent = send(fd, res.c_str(), res.size(), 0);
	// std::cout << "Sent response:\n" << res << std::endl;
	if (sent == -1)
	{
		close(fd);
		return;
	}
	res.erase(0, sent);
	if (res.empty())
	{
		close(fd);
		clients.erase(fd);
	}
}


/**
 * EPOLLIN -->  جديد حاول الاتصال بالسيرفر Client
 * يوجد اتصال جديد ينتظر accept()
 */

void MainLoop::start()
{
	createEpoll();
	std::set<int> serverfds;
	for (size_t i = 0; i < servers.size(); i++)
		serverfds.insert(servers[i].getSocketFd());
	while (running)
	{
		int numEvents = epoll_wait(epollFD, events, MAX_EVENTS, -1);
		if (numEvents == -1)
			throw std::runtime_error("Failed to wait on epoll\n");
		for (int i = 0; i < numEvents; ++i)
		{
			int fd = events[i].data.fd;
			if (serverfds.find(fd) != serverfds.end())
				addNewClients(fd);
			else
			{
				if (events[i].events & (EPOLLHUP | EPOLLERR))
				{
					close(fd);
					continue;
				}
				if (events[i].events & EPOLLIN)
					handleClientEpollIn(fd);

				if (events[i].events & EPOLLOUT)
					handleClientEpollOut(fd);
			}
		}
	}
	closeFds();
}


void MainLoop::closeFds()
{
	for (size_t i = 0; i < servers.size(); i++)
		close(servers[i].getSocketFd());
	for (size_t i = 0; i < clients.size(); i++)
		close(clients[i].getClientFd());
	clients.clear();
	close(epollFD);
}

