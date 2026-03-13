#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <string>
#include <sys/stat.h>
#include "parser/ServerConfig.hpp"
#include "parser/LocationConfig.hpp"

class Router
{
	public:
		Router();
		Router(const Router& other);
		Router& operator=(const Router& other);
		~Router();

		const LocationConfig* findMatchingLocation(const std::string& uri, const ServerConfig& server) const;
		std::string resolvePath(const std::string& uri, const LocationConfig& location, const ServerConfig& server);
		std::string resolveIndex(const std::string& directory_path, const LocationConfig& location, const ServerConfig& server) const;
		bool isCGIRequest(const std::string& file_path, const LocationConfig& location) const;

	private:
		std::string normalizePath(const std::string& path);
		bool fileExists(const std::string& path) const;
		bool isRegularFile(const std::string& path) const;
};

#endif
