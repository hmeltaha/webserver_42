#ifndef UPLOADHANDLER_HPP
#define UPLOADHANDLER_HPP

#include <string>
#include <map>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <errno.h>
#include "parser/LocationConfig.hpp"
#include "parser/ServerConfig.hpp"
#include "handlers/FileHandler.hpp"

struct Request //what the browser sends to the server
{
	std::string method;
	std::string body;
	std::string uri;
	size_t content_length;
	std::map<std::string, std::string> header;

	Request() : content_length(0){}
};


class UploadHandler {
public:
    // Orthodox Canonical Form
    UploadHandler();
    UploadHandler(const UploadHandler& other);
    UploadHandler& operator=(const UploadHandler& other);
    ~UploadHandler();

	FileResponse handleUpload(const Request& request, const LocationConfig& location, const ServerConfig& server);

private:
    std::string extractFilename(const std::string& content_disposition);
    std::string sanitizeFilename(std::string& filename);
    std::string resolveCollision(const std::string& path);
    bool createDirectoryIfNeeded(const std::string& path);
};

#endif
