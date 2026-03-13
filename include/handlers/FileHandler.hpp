#ifndef FILE_HANDLER_HPP
#define FILE_HANDLER_HPP

#include "parser/ConfigParser.hpp"
#include "parser/LocationConfig.hpp"
#include "parser/ServerConfig.hpp"
#include "router/Router.hpp"
#include <sys/stat.h>
#include <map>

struct FileResponse
{
	int status_code;
	std::string mime_type;
	std::string body;
	size_t content_length;

	FileResponse():status_code(500), content_length(0){}
};

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
