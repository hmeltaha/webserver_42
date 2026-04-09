#include "ConfigParser.hpp"
#include "server/server.hpp"
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
	std::string config_file = av[1];
	std::cout << "Reading config: " << config_file << std::endl;	

	signal(SIGINT, signalHandler);
	ConfigParser parser;
	try
	{		
		parser.parse(config_file);
		std::cout << "Config parsed successfully!" << std::endl;
		const std::vector<ServerConfig>& serv = parser.getServers();
		MainLoop mainLoop(serv);
		mainLoop.start();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error parsing config: " << e.what() << std::endl;
		return 1;
	}

	// try
	// {
	// 	std::vector<ServerConfig>& servers = parser.getServers();
	// 	std::vector<Server> serverInstances;
	// 	for (size_t i = 0; i < servers.size(); i++)
	// 	{
	// 		serverInstances.push_back(Server(servers[i]));
	// 		serverInstances[i].setSockets();
	// 		std::cout << "Server " << (i + 1) << " socket set successfully!" << std::endl;
	// 	}
	// }
	// catch(const std::exception& e)
	// {
	// 	std::cerr << e.what() << '\n';
	// }
	
}

// int main(int argc, char** argv)
// {
// 	try {
// 		std::string config_file = "configs/webser.conf";
// 		if (argc >= 2)
// 			config_file = argv[1];
		
// 		std::cout << "Reading config: " << config_file << std::endl;
		
// 		ConfigParser parser;
// 		parser.parse(config_file);
		
// 		const std::vector<ServerConfig>& servers = parser.getServers();
// 		std::cout << "Config parsed successfully!" << std::endl;
// 		std::cout << "Servers found: " << servers.size() << std::endl << std::endl;
		
// 		for (size_t i = 0; i < servers.size(); i++)
// 		{
// 			std::cout << "Server " << (i + 1) << ":" << std::endl;
			
// 			// Listen addresses
// 			std::cout << "  Listen: ";
// 			for (size_t j = 0; j < servers[i].listen_directives.size(); j++)
// 			{
// 				if (j > 0)
// 					std::cout << ", ";
// 				std::cout << servers[i].listen_directives[j].host << ":"
// 				          << servers[i].listen_directives[j].port;
// 			}
// 			std::cout << std::endl;
			
// 			// Server name
// 			if (!servers[i].server_name.empty())
// 				std::cout << "  Server name: " << servers[i].server_name << std::endl;
			
// 			// Root
// 			if (!servers[i].root.empty())
// 				std::cout << "  Root: " << servers[i].root << std::endl;
			
// 			// Index
// 			if (!servers[i].index.empty())
// 				std::cout << "  Index: " << servers[i].index << std::endl;
			
// 			// Max body size
// 			std::cout << "  Max body size: " << servers[i].client_max_body_size;
// 			if (servers[i].client_max_body_size >= 1048576)
// 				std::cout << " (" << (servers[i].client_max_body_size / 1048576) << " MB)";
// 			std::cout << std::endl;
			
// 			// Error pages
// 			if (!servers[i].error_page.empty())
// 				std::cout << "  Error pages: " << servers[i].error_page.size() << std::endl;
			
// 			// Locations
// 			std::cout << "  Locations: " << servers[i].locations.size() << std::endl;
// 			for (size_t k = 0; k < servers[i].locations.size(); k++)
// 			{
// 				std::cout << "    - " << servers[i].locations[k].path;
// 				if (servers[i].locations[k].autoindex)
// 					std::cout << " (autoindex)";
// 				if (!servers[i].locations[k].redirect.empty())
// 					std::cout << " -> " << servers[i].locations[k].redirect;
// 				std::cout << std::endl;
// 			}
// 			std::cout << std::endl;
// 		}
		
// 	} catch (const std::exception& e) {
// 		std::cerr << "Error: " << e.what() << std::endl;
// 		return 1;
// 	}
// 	return 0;
// }
