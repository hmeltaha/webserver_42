#ifndef FILE_HANDLER_HPP
#define FILE_HANDLER_HPP

#include "ConfigParser.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
#include "../requests/HttpRequest.hpp"
#include <sys/stat.h>
#include <map>
#include "../handlers/FileResponse.hpp"

class FileHandler
{
	public:
		FileHandler();
		FileHandler(const FileHandler& o);
		FileHandler &operator=(const FileHandler& o);
		~FileHandler();

		FileResponse serveFile(const std::string& file_path);
		
	private:
		std::string getFileExtension(const std::string& path);
		std::string getMimeType(const std::string& extension);
};

#endif
