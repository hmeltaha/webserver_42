#include "MainLoop.hpp"


MainLoop::MainLoop(const std::vector<ServerConfig>& configs)
{
	for (size_t i = 0; i < configs.size(); ++i)
	{
		servers.push_back(Server(configs[i], i));
		servers[i].loopSetSockets();
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
		servers[i].loopSetSockets();
	}
}


void MainLoop::addNewClients(int fd, int server_index)
{
	while (true)
	{
		int clientFd = accept(fd, NULL, NULL);
		if (clientFd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			std::cerr << "Failed to accept new client: " << strerror(errno) << std::endl;
			continue;
			// throw std::runtime_error(strerror(errno));
		}
		if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
		{
			close(clientFd);
			std::cerr << "Failed to set client socket to non-blocking: " << strerror(errno) << std::endl;
			// throw std::runtime_error(strerror(errno));
			continue;
		}
		if (fcntl(clientFd, F_SETFD, FD_CLOEXEC) == -1) // Set FD_CLOEXEC to ensure the client socket is closed on execve
		{
			close(clientFd);
			std::cerr << "Error: Failed to set client FD as FD_CLOEXEC\n";
			continue;
		}

		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = clientFd;
		clients[clientFd] = Client(clientFd, server_index);
		if (epoll_ctl(epollFD, EPOLL_CTL_ADD, clientFd, &event) == -1)
		{
			close(clientFd);
			std::cerr << "Failed to add client fd to epoll: " << strerror(errno) << std::endl;
			// throw std::runtime_error(strerror(errno));
		}
	}
}


void MainLoop::handleClientEpollIn(int fd)
{
	char buff[4096];
	int flag = recv(fd, buff, sizeof (buff),0);
	if (flag <= 0)
	{
		epoll_ctl(epollFD, EPOLL_CTL_DEL, fd, NULL);
		close (fd);
		clients.erase(fd);
		return;
	}
	std::string data(buff, flag);
	clients[fd].addToReqBuff(data, servers[clients[fd].getServerToConnect()].getConfig());
	if (clients[fd].getState() == PROCESSING)
	{
		clients[fd].setStartTime(time(NULL));
		std::string fullRequest = clients[fd].getReqBuff();
		if (!clients[fd].body.empty())
		{
			fullRequest += clients[fd].body;
		}
		clients[fd].req = clients[fd].parser.parse(fullRequest);
		Router router;

		if (clients[fd].req.status_code != 200)
		{
			clients[fd].res.response = router.serveErrorPage(
				clients[fd].req.status_code,
				servers[clients[fd].getServerToConnect()].getConfig()
			);

			clients[fd].setResBuff(clients[fd].res.getHeaders());
			clients[fd].setState(WRITING);

			struct epoll_event ev;
			ev.events = EPOLLOUT;
			ev.data.fd = fd;
			epoll_ctl(epollFD, EPOLL_CTL_MOD, fd, &ev);

			return;
		}


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
						clients[fd].setStartTime(time(NULL));
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
				clients[fd].setStartTime(time(NULL));
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
		// add all socket fds for each server to epoll
		std::vector<int> fds = servers[i].getSocketFds();
		for (size_t j = 0; j < fds.size(); j++)
		{
			int fd = fds[j];
			// std::cout << "Adding server fd: " << fd << std::endl;
			event.events = EPOLLIN;
			event.data.fd = fd;
			socketFdToServerIndex[fd] = i;
			// EPOLL_CTL_ADD -->  إضافة FD جديد للمراقبة
			if (epoll_ctl(epollFD, EPOLL_CTL_ADD, fd, &event) == -1)
				throw std::runtime_error(strerror(errno));
		}
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

	if (sent <= 0)
	{
		epoll_ctl(epollFD, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		clients.erase(fd);
		return;
	}
	clients[fd].setBytesSend(clients[fd].getBytesSend() + sent);
	if (clients[fd].getBytesSend() >= res.size())
	{
		epoll_ctl(epollFD, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		clients.erase(fd);
	}
}

void MainLoop::start()
{
	createEpoll();
	std::set<int> serverfds;
	for (size_t i = 0; i < servers.size(); i++)
	{
		std::vector<int> fds = servers[i].getSocketFds();
		for (size_t j = 0; j < fds.size(); j++)
		{
			int fd = fds[j];
			serverfds.insert(fd);
		}
	}
	while (running)
	{
		int numEvents = epoll_wait(epollFD, events, MAX_EVENTS, 100);
		if (numEvents == -1 && running == true)
			throw std::runtime_error(strerror(errno));
		for (int i = 0; i < numEvents; ++i)
		{
			int fd = events[i].data.fd;
			if (handle_cgi.active_processes.count(fd))
				handle_cgi.handleCgiOutput(fd, epollFD, clients);

			else if (serverfds.find(fd) != serverfds.end())
				addNewClients(fd, socketFdToServerIndex[fd]);
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
	time_t time_now = time(NULL);
	std::map<int, Client>::iterator it = clients.begin();
	while	 ( it != clients.end())
	{
		if (it->second.getState() == READING || it->second.getState() == READING_BODY || it->second.getState() == WRITING)
		{
			if (time_now - it->second.getStartTime() >= TIMEOUT)
			{
				epoll_ctl(epollFD, EPOLL_CTL_DEL, it->first, NULL);
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
		if (time_now - cgiIt->second.start_time >= TIMEOUT)
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
	{
		std::vector<int> fds = servers[i].getSocketFds();
		for (size_t j = 0; j < fds.size(); j++)
		{
			int fd = fds[j];
			close(fd);
		}
	}
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
		close(it->first);
	clients.clear();
	close(epollFD);
}

