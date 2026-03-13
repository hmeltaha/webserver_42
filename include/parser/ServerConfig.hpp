#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "parser/LocationConfig.hpp"
#include <map>
#include <string>

struct ListenDirective
{
	std::string host;
	int port;
	
	ListenDirective() : host("0.0.0.0"), port(80) {}
	ListenDirective(const std::string& h, int p) : host(h), port(p) {}
};

class ServerConfig
{
	public:
		std::vector<ListenDirective> listen_directives;
		std::string server_name;
		std::string root;
		std::string index;
		std::map<int,  std::string>error_page;
		size_t client_max_body_size;
		std::vector<LocationConfig>locations;
		ServerConfig();
		ServerConfig(const ServerConfig& other);
		ServerConfig& operator=(const ServerConfig& other);
		~ServerConfig();
};

#endif