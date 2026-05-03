#include "ConfigParser.hpp"

ConfigParser::ConfigParser() : current_token(0) {}

ConfigParser::~ConfigParser() {}

void ConfigParser::parse(const std::string& filename)
{
	tokenize(filename);

	int brace_count = 0;
	for (size_t i = 0; i < tokens.size(); i++)
	{
		if (tokens[i] == "{")
		brace_count++;
		else if (tokens[i] == "}")
		brace_count--;
		if (brace_count < 0)
		throw std::runtime_error("Unexpected closing brace '}'");
	}
	if (brace_count != 0)
	throw std::runtime_error("Unclosed block: missing closing brace");

	parseConfig();
	validateServers();
}

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

const std::vector<ServerConfig>& ConfigParser::getServers() const
{
	return servers;
}

void ConfigParser::tokenize(const std::string& filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
	throw std::runtime_error("Can't open config file");
	std::string line;
	while (std::getline(file, line))
	{
		if (!line.empty() && line[0] == '#')
		continue;
		std::istringstream iss(line);
		std::string word;
		while (iss >> word)
		{
			if (!word.empty())
			{
				size_t len = word.length();
				char last = word[len - 1];
				if (last == ';' || last == '{' || last == '}')
				{
					if (len > 1)
					tokens.push_back(word.substr(0, len - 1));
					tokens.push_back(std::string(1, last));
				}
				else
				tokens.push_back(word);
			}
		}
	}
	file.close();
}

bool ConfigParser::hasMoreTokens()
{
	return current_token < tokens.size();
}

std::string ConfigParser::peekToken()
{
	if (!hasMoreTokens())
	return "";
	return tokens[current_token];
}

std::string ConfigParser::getNextToken()
{
	if (!hasMoreTokens())
	throw std::runtime_error("Unexpected end of config file");
	std::string word = tokens[current_token];
	current_token++;
	return word;
}

void ConfigParser::expectToken(const std::string& expected)
{
	std::string token = getNextToken();
	if (token != expected)
	throw std::runtime_error("Expected '" + expected + "' but got '" + token + "'");
}

void ConfigParser::parseConfig()
{
	current_token = 0;
	while (hasMoreTokens())
	{
		std::string token = getNextToken();
		if (token == "server")
		{
			ServerConfig server;
			expectToken("{");
			parseServer(server);
			expectToken("}");
			servers.push_back(server);
		}
		else
		throw std::runtime_error("Unexpected token: " + token);
	}
	if (servers.empty())
	throw std::runtime_error("No server blocks found");
}

void ConfigParser::printTokens() const
{
	for (size_t i = 0; i < tokens.size(); i++)
	{
		std::cout << "[" << i << "] " << tokens[i] << std::endl;
	}
}
