#include "ServerConfig.hpp"

ServerConfig::ServerConfig():
	listen_directives(),
	server_name(""),
	root(""),
	index(""),
	error_page(),
	client_max_body_size(1048576),  //default: 1MB
	locations()
{}

ServerConfig::~ServerConfig(){}