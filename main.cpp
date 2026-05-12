#include "ConfigParser.hpp"
#include "server/Server.hpp"
#include <iostream>
#include "server/MainLoop.hpp"
#include <signal.h>

bool running = true;

int main(int ac, char** av)
{
	if (ac < 2)
	{
		std::cout << "No config file provided."<< std::endl;
		return 1;
	}
	MainLoop mainLoop;
	try
	{
		std::string config_file = av[1];
		signal(SIGINT, signalHandler);
		signal(SIGTSTP, signalHandler);
		ConfigParser parser;
		parser.parse(config_file);
		const std::vector<ServerConfig>& serv = parser.getServers();
		// MainLoop mainLoop(serv);
		mainLoop.setServers(serv);
		mainLoop.start();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error : " << e.what() << std::endl;
		mainLoop.closeFds();
		return 1;
	}
}


//valgrind --leak-check=full --track-fds=yes --show-leak-kinds=all ./webserv configs/webser.conf
