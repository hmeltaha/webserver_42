#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "ServerConfig.hpp"

class ConfigParser
{
	private:
		std::vector<ServerConfig> servers;
		std::vector<std::string> tokens;
		size_t current_token;
		void tokenize(const std::string& filename);
		std::string getNextToken();
		std::string peekToken();
		bool hasMoreTokens();
		void expectToken(const std::string& expected);
		void parseConfig();
		void parseServer(ServerConfig& server);
		void parseLocation(ServerConfig& server);
		void parseServerDirective(ServerConfig& server, const std::string& directive);
		void parseLocationDirective(LocationConfig& loc, const std::string& directive);
		size_t parseSize(const std::string& size_str);
		int parsePort(const std::string& port_str);
		void parseListen(ServerConfig& server, const std::string& value);
		void validateServers();
	
	public:
		ConfigParser();
		~ConfigParser();
		void parse(const std::string& filename);
		const std::vector<ServerConfig>& getServers() const;
		
		void printTokens() const; //for testing
};
#endif