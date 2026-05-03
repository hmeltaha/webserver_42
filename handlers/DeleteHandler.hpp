#ifndef DELETEHANDLER_HPP
#define DELETEHANDLER_HPP

#include <string>
#include "FileHandler.hpp"
#include "../parser/LocationConfig.hpp"
#include "../parser/ServerConfig.hpp"
#include "../utils/MethodValidator.hpp"
#include "../requests/HttpRequest.hpp"
#include "UploadHandler.hpp"
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib> 

class DeleteHandler {
public:
    DeleteHandler();
    DeleteHandler(const DeleteHandler& other);
    DeleteHandler& operator=(const DeleteHandler& other);
    ~DeleteHandler();

    FileResponse handleDelete(const HttpRequest& request, 
                              const std::string& file_path,
                              const LocationConfig& location,
                              const ServerConfig& server);

private:
};

#endif
