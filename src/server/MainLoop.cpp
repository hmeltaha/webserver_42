#include "server/MainLoop.hpp"
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <sys/socket.h>

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
        int flag = read(fd, buff, sizeof(buff) - 1);
        if (flag <= 0)
        {
                close(fd);
                clients.erase(fd);
                return;
        }
        buff[flag] = '\0';
        clients[fd].setState(READING);
        clients[fd].addToReqBuff(buff);
        if (clients[fd].getState() == READING_BODY)
                clients[fd].addBodyToReq();
        if (clients[fd].getState() == PROCESSING)
        {
                std::cout << "Received request:\n" << clients[fd].getReqBuff() << std::endl;

                /**
                 * TODO: parse the request -> route -> call appropriate handler -> build response
                 * (nora's Router + FileHandler/UploadHandler/DeleteHandler)
                 */
                std::string response =
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Length: 12\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "hi from heba";
                clients[fd].setResBuff(response);
                clients[fd].setState(WRITING);
                struct epoll_event ev;
                ev.events = EPOLLOUT;
                ev.data.fd = fd;
                if (epoll_ctl(epollFD, EPOLL_CTL_MOD, fd, &ev) == -1)
                        throw std::runtime_error("Failed to modify client socket to epoll\n");
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
                int fd = servers[i].getSocketFd();
                std::cout << "Trying to add server fd: " << fd << std::endl;
                event.events = EPOLLIN;
                event.data.fd = servers[i].getSocketFd();
                if (epoll_ctl(epollFD, EPOLL_CTL_ADD, servers[i].getSocketFd(), &event) == -1)
                        throw std::runtime_error("Failed to add server socket to epoll\n");
        }
}

void MainLoop::handleClientEpollOut(int fd)
{
        std::string res = clients[fd].getResBuff();
        ssize_t sent = send(fd, res.c_str(), res.size(), 0);
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
        for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
                close(it->second.getClientFd());
        clients.clear();
        close(epollFD);
}
