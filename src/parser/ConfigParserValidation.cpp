#include "parser/ConfigParser.hpp"

void ConfigParser::validateServers()
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		for (size_t j = 0; j < servers[i].listen_directives.size(); j++)
		{
			std::string host_a = servers[i].listen_directives[j].host;
			int port_a = servers[i].listen_directives[j].port;

			for (size_t k = i + 1; k < servers.size(); k++)
			{
				for (size_t m = 0; m < servers[k].listen_directives.size(); m++)
				{
					std::string host_b = servers[k].listen_directives[m].host;
					int port_b = servers[k].listen_directives[m].port;

					if (port_a == port_b)
					{
						if (host_a == host_b || host_a == "0.0.0.0" || host_b == "0.0.0.0")
						{
							std::stringstream ss;
							ss << port_a;
							std::string port_str = ss.str();

							throw std::runtime_error("Duplicate listen: " + host_b + ":" + port_str);
						}
					}
				}
			}
		}
	}
}

void ConfigParser::validateDirectiveContext(std::string& directive_name, bool in_server, bool in_location)
{
	std::vector<std::string> server_only;
	server_only.push_back("listen");
	server_only.push_back("server_name");
	server_only.push_back("client_max_body_size");
	server_only.push_back("error_page");
	
	std::vector<std::string> location_only;
	location_only.push_back("allowed_methods");
	location_only.push_back("upload_path");
	location_only.push_back("cgi_handler");
	location_only.push_back("autoindex");
	location_only.push_back("return");
	
	std::vector<std::string> both_allowed;
	both_allowed.push_back("root");
	both_allowed.push_back("index");
	
	if (in_server && !in_location)
	{
		if (std::find(location_only.begin(), location_only.end(), directive_name) != location_only.end())
			throw std::runtime_error("Directive '" + directive_name + "' not allowed in server context");
	}
	
	if (in_location && !in_server)
	{
		if (std::find(server_only.begin(), server_only.end(), directive_name) != server_only.end())
			throw std::runtime_error("Directive '" + directive_name + "' not allowed in location context");
	}
}

void ConfigParser::validatePath(const std::string& path)
{
	if (path.empty())
		throw std::runtime_error("Path cannot be empty");
	
	for (size_t i = 0; i < path.length(); i++)
	{
		if (path[i] == '\0')
			throw std::runtime_error("Path contains null byte");
	}
}

void ConfigParser::validateCGIExtension(const std::string& extension)
{
	if (extension.empty())
		throw std::runtime_error("CGI extension cannot be empty");
	
	if (extension[0] != '.')
		throw std::runtime_error("CGI extension must start with '.'");
	
	if (extension.length() > 10)
		throw std::runtime_error("CGI extension too long (max 10 characters)");
	
	for (size_t i = 0; i < extension.length(); i++)
	{
		if (extension[i] == '*' || extension[i] == '?')
			throw std::runtime_error("CGI extension cannot contain wildcards (* or ?)");
		
		if (extension[i] == '\0')
			throw std::runtime_error("CGI extension contains null byte");
		
		if (extension[i] == ' ' || extension[i] == '\t')
			throw std::runtime_error("CGI extension cannot contain whitespace");
	}
}

void ConfigParser::setDefaults()
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		if (servers[i].listen_directives.empty())
			servers[i].listen_directives.push_back(ListenDirective("0.0.0.0", 80));
		
		if (servers[i].client_max_body_size == 0)
			servers[i].client_max_body_size = 1048576;
		
		if (servers[i].root.empty())
			servers[i].root = "/var/www/html";
		
		if (servers[i].index.empty())
			servers[i].index = "index.html";
		
		for (size_t j = 0; j < servers[i].locations.size(); j++)
		{
			if (servers[i].locations[j].root.empty())
				servers[i].locations[j].root = servers[i].root;
			
			if (servers[i].locations[j].index.empty())
				servers[i].locations[j].index = servers[i].index;
			
			if (servers[i].locations[j].allowed_methods.empty())
				servers[i].locations[j].allowed_methods.push_back("GET");
		}
	}
}
