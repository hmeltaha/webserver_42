#ifndef DELETEHANDLER_HPP
#define DELETEHANDLER_HPP

#include <string>
#include "FileHandler.hpp"
#include "../parser/LocationConfig.hpp"
#include "../parser/ServerConfig.hpp"
#include "../utils/MethodValidator.hpp"
#include "UploadHandler.hpp"
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib> 

class DeleteHandler {
public:
    // Orthodox Canonical Form
    DeleteHandler();
    DeleteHandler(const DeleteHandler& other);
    DeleteHandler& operator=(const DeleteHandler& other);
    ~DeleteHandler();

    FileResponse handleDelete(const Request& request, 
                              const std::string& file_path,
                              const LocationConfig& location,
                              const ServerConfig& server);

private:
};

#endif
