#include "parser/ConfigParser.hpp"
#include "server/server.hpp"
#include "server/MainLoop.hpp"
#include <iostream>
#include <signal.h>

bool running = true;

int main(int ac, char** av)
{
        if (ac < 2)
        {
                std::cout << "No config file provided." << std::endl;
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
                std::cerr << "Error: " << e.what() << std::endl;
                return 1;
        }
        return 0;
}
