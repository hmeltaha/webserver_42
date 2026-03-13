#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

class LocationConfig
{
	public:
		std::string root;
		std::string path;
		std::string index;
		std::string redirect;
		std::string upload_path;
		std::string cgi_path;
		std::string cgi_extension;
		std::vector<std::string>allowed_methods;
		bool autoindex;
		LocationConfig();
		LocationConfig(const LocationConfig& other);
		LocationConfig& operator=(const LocationConfig& other);
		~LocationConfig();
};


#endif