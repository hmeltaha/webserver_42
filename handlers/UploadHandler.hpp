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
#include "../parser/LocationConfig.hpp"
#include "../parser/ServerConfig.hpp"
#include "../requests/HttpRequest.hpp"
#include "FileHandler.hpp"
#include "../handlers/FileResponse.hpp"



class UploadHandler {
public:
    
    UploadHandler();
    UploadHandler(const UploadHandler& other);
    UploadHandler& operator=(const UploadHandler& other);
    ~UploadHandler();

	FileResponse handleUpload(const HttpRequest& request, const LocationConfig& location, const ServerConfig& server);

private:
    std::string extractFilename(const std::string& content_disposition);
    std::string sanitizeFilename(std::string& filename);
    std::string resolveCollision(const std::string& path);
    bool createDirectoryIfNeeded(const std::string& path);
	std::string extractFileContent(const std::string& body);
};

#endif
