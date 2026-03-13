#include "parser/ConfigParser.hpp"

size_t ConfigParser::parseSize(const std::string& size_str)
{
	if (size_str.empty())
		throw std::runtime_error("Empty size string");
	
	if (size_str[0] == '-')
		throw std::runtime_error("Size cannot be negative");
	
	std::string num_pos = "";
	std::string bytes_part = "";
	size_t i = 0;

	while (i < size_str.length() && isdigit(size_str[i]))
		i++;

	num_pos = size_str.substr(0, i);
	bytes_part = size_str.substr(i);
	
	if (num_pos.empty())
		throw std::runtime_error("No number in size string");
	
	long long num = 0;
	for (size_t j = 0; j < num_pos.length(); j++)
	{
		if (num_pos[j] == '.')
			throw std::runtime_error("Decimal sizes not supported");
		num = num * 10 + (num_pos[j] - '0');
		if (num > 10000)
			throw std::runtime_error("Size value too large");
	}
	
	size_t multiplier;
	if (bytes_part.empty() || bytes_part == "B")
		multiplier = 1;
	else if (bytes_part == "K" || bytes_part == "k" || bytes_part == "KB" || bytes_part == "kb")
		multiplier = 1024;
	else if (bytes_part == "M" || bytes_part == "m" || bytes_part == "MB" || bytes_part == "mb")
		multiplier = 1024 * 1024;
	else if (bytes_part == "G" || bytes_part == "g" || bytes_part == "GB" || bytes_part == "gb")
		multiplier = 1024 * 1024 * 1024;
	else
		throw std::runtime_error("Invalid size format");
	
	if (num > 0 && multiplier > (size_t)-1 / (size_t)num)
		throw std::runtime_error("Size value causes overflow");
	
	return (size_t)(num * multiplier);
}

int parse_port(std::string port)
{
	if (port.empty())
		throw std::runtime_error("Port cannot be empty");
	for (size_t i = 0; i < port.length(); i++)
		{
			if (!isdigit((unsigned char)port[i]))
				throw std::runtime_error("Port must be numeric");	
		}
	int port_int = atoi(port.c_str());
	if (port_int == 0)
		throw std::runtime_error("Port 0 is not allowed");
	if (port_int < 1 || port_int > 65535)
		throw std::runtime_error("Invalid port number");
	return port_int;
}

void ConfigParser::parseListen(ServerConfig& server, const std::string& value)
{
	size_t colon_pos = value.find(':');
	std::string host_part;
	std::string port_part;

	if (colon_pos != std::string::npos)
	{
		host_part = value.substr(0, colon_pos);
		port_part = value.substr(colon_pos + 1);
	}
	else
	{
		host_part = "0.0.0.0";
		port_part = value;
	}
	
	if (port_part.empty())
		throw std::runtime_error("No port specified");
	
	int port = parse_port(port_part);
	
	for (size_t i = 0; i < server.listen_directives.size(); i++)
	{
		if (server.listen_directives[i].host == host_part && 
		    server.listen_directives[i].port == port)
			return;
	}
	
	server.listen_directives.push_back(ListenDirective(host_part, port));
}

void ConfigParser::parseServer(ServerConfig& server)
{
	while (peekToken() != "}")
	{
		if (!hasMoreTokens())
			throw std::runtime_error("Unexpected end of file in server block");
		
		std::string directive = getNextToken();
		if (directive == "location")
			parseLocation(server);
		else
			parseServerDirective(server, directive);
	}
}

void ConfigParser::parseServerDirective(ServerConfig& server, const std::string& directive)
{
	validateDirectiveContext(const_cast<std::string&>(directive), true, false);
	
	if (directive == "listen")
	{
		std::string value = getNextToken();
		parseListen(server, value);
		expectToken(";");
	}
	else if (directive == "server_name")
	{
		std::string value = getNextToken();
		server.server_name = value;
		expectToken(";");
	}
	else if (directive == "root")
	{
		std::string value = getNextToken();
		validatePath(value);
		server.root = value;
		expectToken(";");
	}
	else if (directive == "index")
	{
		std::string value = getNextToken();
		validatePath(value);
		server.index = value;
		expectToken(";");
	}
	else if (directive == "client_max_body_size")
	{
		std::string value = getNextToken();
		server.client_max_body_size = parseSize(value);
		expectToken(";");
	}
	else if (directive == "error_page")
	{
		std::string error_code = getNextToken();
		int error = atoi(error_code.c_str());
		
		if (error < 100 || error > 599)
			throw std::runtime_error("Invalid HTTP error code: " + error_code);
		
		std::string page_path = getNextToken();
		validatePath(page_path);
		server.error_page[error] = page_path;
		expectToken(";");
	}
	else
		throw std::runtime_error("Unknown server directive: " + directive);
}

void ConfigParser::parseLocation(ServerConfig& server)
{
	std::string path = getNextToken();
	
	if (path.empty() || path[0] != '/')
		throw std::runtime_error("Location path must start with '/': " + path);
	
	if (path != "/" && path[path.length() - 1] == '/')
		path = path.substr(0, path.length() - 1);
	
	for (size_t i = 0; i < server.locations.size(); i++)
	{
		if (server.locations[i].path == path)
			throw std::runtime_error("Duplicate location path: " + path);
	}
	
	expectToken("{");
	
	LocationConfig location;
	location.path = path;
	
	while (peekToken() != "}")
	{
		if (!hasMoreTokens())
			throw std::runtime_error("Unexpected end of file in location block");
		
		std::string directive = getNextToken();
		parseLocationDirective(location, directive);
	}
	
	expectToken("}");
	server.locations.push_back(location);
}

void ConfigParser::parseLocationDirective(LocationConfig& location, const std::string& directive)
{
	validateDirectiveContext(const_cast<std::string&>(directive), false, true);
	
	if (directive == "root")
	{
		std::string value = getNextToken();
		validatePath(value);
		location.root = value;
		expectToken(";");
	}
	else if (directive == "index")
	{
		std::string value = getNextToken();
		validatePath(value);
		location.index = value;
		expectToken(";");
	}
	else if (directive == "autoindex")
	{
		std::string value = getNextToken();
		if (value == "on")
			location.autoindex = true;
		else if (value == "off")
			location.autoindex = false;
		else
			throw std::runtime_error("Invalid autoindex value: " + value);
		expectToken(";");
	}
	else if (directive == "allowed_methods")
	{
		std::vector<std::string> valid_methods;
		valid_methods.push_back("GET");
		valid_methods.push_back("POST");
		valid_methods.push_back("DELETE");
		valid_methods.push_back("PUT");
		valid_methods.push_back("HEAD");
		while (peekToken() != ";")
		{
			std::string method = getNextToken();
			if (std::find(valid_methods.begin(), valid_methods.end(), method) == valid_methods.end())
				throw std::runtime_error("Invalid HTTP method: " + method);
			else
				location.allowed_methods.push_back(method);
		}
		
		if (location.allowed_methods.empty())
			throw std::runtime_error("allowed_methods cannot be empty");
		
		expectToken(";");
	}
	else if (directive == "return")
	{
		std::string url = getNextToken();
		location.redirect = url;
		expectToken(";");
	}
	else if (directive == "upload_path")
	{
		std::string path = getNextToken();
		validatePath(path);
		location.upload_path = path;
		expectToken(";");
	}
	else if (directive == "cgi_handler")
	{
		std::string extension = getNextToken();
		std::string cgi_path = getNextToken();
		
		validateCGIExtension(extension);
		validatePath(cgi_path);
		
		location.cgi_extension = extension;
		location.cgi_path = cgi_path;
		expectToken(";");
	}
	else
		throw std::runtime_error("Unknown location directive: " + directive);
}

