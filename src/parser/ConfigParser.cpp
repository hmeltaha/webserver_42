#include "parser/ConfigParser.hpp"

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
	setDefaults();
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
	int line_number = 0;
	while (std::getline(file, line))
	{
		line_number++;
		size_t com_pos = line.find('#');
		if (com_pos != std::string::npos)
			line = line.substr(0, com_pos);
		if (line.find_first_not_of(" \t\r\n") == std::string::npos)
			continue;
		int i = 0;
		while (i < line.length())
		{
			while (i < line.length() && isspace(line[i]))
				i++;
			std::string token;
			if (line[i] == '"' || line[i] == '\'')
			{
				char quote = line[i];
				i++;
				while (i < line.length() && line[i] != quote)
				{
					if (line[i] == '\\' && i + 1 < line.length())
					{
						i++;
						token += line[i];
					}
					else
						token += line[i];
					i++;
				}
				if (i >= line.length())
					throw std::runtime_error("Unclosed quote on line" + line_number);
				i++;
				tokens.push_back(token);
			}
			else if (line[i] == ';' || line[i] == '{' || line[i] == '}')
			{
				tokens.push_back(std::string(1, line[i]));
				i++;
			}
			else
			{
				while (i < line.length() && !isspace(line[i])
						&& line[i] != ';' && line[i] != '{' && line[i] != '}')
				{
					token += line[i];
					i++;
				}
				if (!token.empty())
					tokens.push_back(token);

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
