#include "handlers/DeleteHandler.hpp"
#include "utils/MethodValidator.hpp"
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib>



DeleteHandler::DeleteHandler() {
}

DeleteHandler::DeleteHandler(const DeleteHandler& other) {
    (void)other;
}

DeleteHandler& DeleteHandler::operator=(const DeleteHandler& other) {
    if (this != &other) {
    }
    return *this;
}

DeleteHandler::~DeleteHandler() {
}

FileResponse DeleteHandler::handleDelete(const Request& request, 
                                          const std::string& file_path,
                                          const LocationConfig& location,
                                          const ServerConfig& server)
{
	(void)request;
    FileResponse response;

    MethodValidator validator;
	if (!validator.isMethodAllowed("DELETE", location.allowed_methods))
	{
		response.status_code = 405;
		return response;
	}

	struct stat file_info;
	if (stat(file_path.c_str(), &file_info) != 0)
	{
		response.status_code = 404;
		return response;
	}
	if (!S_ISREG(file_info.st_mode))
	{
		response.status_code = 403;
		return response;
	}
	
	char* resolved_file = realpath(file_path.c_str(), NULL);//resolves all symlinks
	
	std::string root;
	if (!location.root.empty())
		root = location.root;
	else if (!server.root.empty())
		root = server.root;
	char* resolved_root = realpath(root.c_str(), NULL);
	if (!resolved_file || !resolved_root)
	{
		if (resolved_file)
			free(resolved_file);
		if (resolved_root)
			free(resolved_root);
		response.status_code = 403;
		return response;
	}
	std::string file_str(resolved_file);
	std::string root_str(resolved_root);

	if (file_str.find(root_str) != 0)
	{
		free(resolved_file);
		free(resolved_root);
		response.status_code = 403;
		return response;
	}
	free(resolved_file);
	free(resolved_root);

	if (unlink(file_path.c_str()) != 0)
	{
		response.status_code = 500;
		return response;
	}
	
	response.status_code = 204;
	response.body = "";
	response.content_length = 0;
    
    return response;
}
