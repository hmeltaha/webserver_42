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

MainLoop::MainLoop()
{
	epollFD = -1;
}

void MainLoop::setServers(const std::vector<ServerConfig>& configs)
{
	for (size_t i = 0; i < configs.size(); ++i)
	{
		servers.push_back(Server(configs[i], i));
		servers[i].setSockets();
	}
}


void MainLoop::addNewClients(int fd, int server_index)
{
	while (true)
	{
		int clientFd = accept(fd, NULL, NULL);
		if (clientFd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK) // no more client are witting right now. i accept all client for now.
				break;
			throw std::runtime_error(strerror(errno));
		}
		// std::cout << "Accept new client....." << std::endl;
		int flags = fcntl(clientFd, F_GETFL, 0);
		if (flags == -1)
			throw std::runtime_error(strerror(errno));
		if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1)
			throw std::runtime_error(strerror(errno));
		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = clientFd;
		clients[clientFd] = Client(clientFd, server_index);
		if (epoll_ctl(epollFD, EPOLL_CTL_ADD, clientFd, &event) == -1)
			throw std::runtime_error(strerror(errno));
	}
}


void MainLoop::handleClientEpollIn(int fd)
{
	char buff[4096];
	int flag = recv(fd, buff, sizeof (buff),0);
	if (flag <= 0)
	{
		if (flag == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return;
		}
		close (fd);
		clients.erase(fd);
		return;
	}
	std::string data(buff, flag);
	clients[fd].addToReqBuff(data, servers[clients[fd].getServerToConnect()].getConfig());
	if (clients[fd].getState() == PROCESSING)
	{
		clients[fd].setStartTime(time(NULL));
		clients[fd].req = clients[fd].parser.parse(clients[fd].getReqBuff());

		Router router;
		router.seeIfPayloadTooLarge(clients[fd]);

		if (clients[fd].getState() == PROCESSING)
		{
			const ServerConfig& config = servers[clients[fd].getServerToConnect()].getConfig();
			std::string normalized_path = router.normalizePath(clients[fd].req.path);
			const LocationConfig* location = router.findMatchingLocation(normalized_path, config);

			bool is_cgi = false;
			if (location)
			{
				std::string script = router.resolvePath(normalized_path, *location, config);
				if (router.isCGIRequest(script, *location))
				{
					is_cgi = true;
					int ok = handle_cgi.startCgi(fd, clients[fd].req.method, script,
												clients[fd].req.query,
												clients[fd].req.body,
												StringMap(), epollFD);
					if (ok != 0)
					{
						clients[fd].res.response = router.serveErrorPage(ok, config);
						clients[fd].setResBuff(clients[fd].res.getHeaders());
						clients[fd].setState(WRITING);
						struct epoll_event ev;
						ev.events  = EPOLLOUT;
						ev.data.fd = fd;
						epoll_ctl(epollFD, EPOLL_CTL_MOD, fd, &ev);
					}
				}
			}

			if (!is_cgi)
			{
				clients[fd].res.response = router.route(clients[fd].req, config);
				clients[fd].setResBuff(clients[fd].res.getHeaders());
				clients[fd].setState(WRITING);
				struct epoll_event ev;
				ev.events  = EPOLLOUT;
				ev.data.fd = fd;
				if (epoll_ctl(epollFD, EPOLL_CTL_MOD, fd, &ev) == -1)
					throw std::runtime_error(strerror(errno));
			}
		}
		else
		{
			//for payload too large case
			clients[fd].setResBuff(clients[fd].res.getHeaders());
			struct epoll_event ev;
			ev.events  = EPOLLOUT;
			ev.data.fd = fd;
			if (epoll_ctl(epollFD, EPOLL_CTL_MOD, fd, &ev) == -1)
				throw std::runtime_error(strerror(errno));
		}
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
		std::cout << "Adding server fd: " << fd << std::endl;
		event.events = EPOLLIN;
		event.data.fd = servers[i].getSocketFd();
		serverTOClient[servers[i].getSocketFd()] = i;
		// EPOLL_CTL_ADD -->  إضافة FD جديد للمراقبة]
		if (epoll_ctl(epollFD, EPOLL_CTL_ADD, servers[i].getSocketFd(), &event) == -1)
			throw std::runtime_error(strerror(errno));
	}
}

/**
 *
 * if the connection fail (EAGAIN or EWOULDBLOCK) --> do nothing and wait for the next EPOLLIN event to try again.
 */

void MainLoop::handleClientEpollOut(int fd)
{
	std::string& res = clients[fd].getResBuff();
	size_t remaining = res.size() -  clients[fd].getBytesSend();
	size_t to_send = std::min(remaining, (size_t)CHUNK_SIZE);
	int sent = send(fd, res.c_str() + clients[fd].getBytesSend() , to_send, 0);

	if (sent == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		close(fd);
		clients.erase(fd);
		return;
	}
	clients[fd].setBytesSend(clients[fd].getBytesSend() + sent);
	if (clients[fd].getBytesSend() >= res.size())
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
		int numEvents = epoll_wait(epollFD, events, MAX_EVENTS, 1000);
		if (numEvents == -1 && running == true)
			throw std::runtime_error(strerror(errno));
		for (int i = 0; i < numEvents; ++i)
		{
			int fd = events[i].data.fd;
			if (handle_cgi.active_processes.count(fd))
				handle_cgi.handleCgiOutput(fd, epollFD, clients);
			
			else if (serverfds.find(fd) != serverfds.end())
				addNewClients(fd, serverTOClient[fd]);
			else
			{
				if (events[i].events & (EPOLLHUP | EPOLLERR))
				{
					close(fd);
					clients.erase(fd);
					continue;
				}
				if (events[i].events & EPOLLIN)
					handleClientEpollIn(fd);

				if (events[i].events & EPOLLOUT)
					handleClientEpollOut(fd);
			}
		}
		checkTimeout();
	}
	closeFds();
}

void MainLoop::checkTimeout()
{

	//for clients
	time_t time_now = time(NULL);
	std::map<int, Client>::iterator it = clients.begin();
	while	 ( it != clients.end())
	{
		if (it->second.getState() == READING || it->second.getState() == READING_BODY)
		{
			if (time_now - it->second.getStartTime() > TIMEOUT)
			{
				close(it->first);
				std::map<int, Client>::iterator tmp = it;
				 ++it;
				clients.erase(tmp);
				continue;
			}
		}
		++it;
	}

	//for CGI processes
	std::map<int, CgiProcess>::iterator cgiIt = handle_cgi.active_processes.begin();
	while (cgiIt != handle_cgi.active_processes.end())
	{
		if (time_now - cgiIt->second.start_time > TIMEOUT)
		{
			kill(cgiIt->second.pid, SIGKILL);
			waitpid(cgiIt->second.pid, NULL, WNOHANG);

			HttpResponse result;
			result.response.status_code = 504;
			result.response.body = "<h1>504 Gateway Timeout</h1>\n<p>CGI script timed out.</p>";
			result.response.mime_type = "text/html";
			std::map<int, Client>::iterator clientIt = clients.find(cgiIt->second.clientFd);
		if (clientIt != clients.end())
		{
			clientIt->second.setResBuff(result.getHeaders());
			clientIt->second.setState(WRITING);
			struct epoll_event ev;
			ev.events = EPOLLOUT;
			ev.data.fd = cgiIt->second.clientFd;
			epoll_ctl(epollFD, EPOLL_CTL_MOD, cgiIt->second.clientFd, &ev);
		}

		epoll_ctl(epollFD, EPOLL_CTL_DEL, cgiIt->first, NULL);
		close(cgiIt->first);
		std::map<int, CgiProcess>::iterator tmp = cgiIt;
		++cgiIt;
		handle_cgi.active_processes.erase(tmp);
		continue;

		}
		else
			++cgiIt;
	}
}

void MainLoop::closeFds()
{
	for (size_t i = 0; i < servers.size(); i++)
		close(servers[i].getSocketFd());
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
		close(it->first);
	clients.clear();
	close(epollFD);
}

