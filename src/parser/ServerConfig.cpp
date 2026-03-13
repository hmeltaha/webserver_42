#include "parser/ServerConfig.hpp"

ServerConfig::ServerConfig():
	listen_directives(),
	server_name(""),
	root(""),
	index(""),
	error_page(),
	client_max_body_size(1048576),  //default: 1MB
	locations()
{}

ServerConfig::ServerConfig(const ServerConfig& other):
	listen_directives(other.listen_directives),
	server_name(other.server_name),
	root(other.root),
	index(other.index),
	error_page(other.error_page),
	client_max_body_size(other.client_max_body_size),
	locations(other.locations)
{}

ServerConfig& ServerConfig::operator=(const ServerConfig& other)
{
	if (this != &other)
	{
		listen_directives = other.listen_directives;
		server_name = other.server_name;
		root = other.root;
		index = other.index;
		error_page = other.error_page;
		client_max_body_size = other.client_max_body_size;
		locations = other.locations;
	}
	return *this;
}

ServerConfig::~ServerConfig(){}