#include "router/Router.hpp"
#include "handlers/DeleteHandler.hpp"
#include "handlers/DirectoryLister.hpp"
#include "handlers/FileHandler.hpp"
#include "handlers/UploadHandler.hpp"
#include "utils/MethodValidator.hpp"
#include <fstream>
#include <iterator>
#include <sstream>

Router::Router(){}

Router::Router(const Router& o)
{
	(void)o;
}

Router& Router::operator=(const Router& o)
{
	(void)o;
	return *this;
}

Router::~Router(){}

const LocationConfig* Router::findMatchingLocation(const std::string& uri, const ServerConfig& server) const
{
	const LocationConfig* best_match = NULL;
	size_t best_length = 0;

	for (size_t i = 0; i < server.locations.size(); i++)
	{
		const LocationConfig& location = server.locations[i];
		const std::string& location_path = location.path;
		
		if (uri.length() >= location_path.length())
		{
			if (uri.substr(0, location_path.length()) == location_path)
			{
				if (location_path == "/" || uri.length() == location_path.length() || 
				    uri[location_path.length()] == '/')
				{
					size_t match_length = location_path.length();
					if (match_length > best_length)
					{
						best_match = &location;
						best_length = match_length;
					}
				}
			}
		}
	}
	
	return best_match;
}

bool Router::isRegularFile(const std::string& index_path) const
{
	struct stat file;
	if (stat(index_path.c_str(), &file) != 0)
		return false;
	return S_ISREG(file.st_mode); //file type and permission
}

bool Router::fileExists(const std::string& index_path) const
{
	struct stat file_info;
	int res = stat(index_path.c_str(), &file_info);
	if (res == 0)
		return true;
	else
		return false;
}
std::string Router::resolveIndex(const std::string& directory_path, const LocationConfig& location, const ServerConfig& server)const
{
	std::string index_file;
	if (!location.index.empty())
		index_file = location.index;
	else if (!server.index.empty())
		index_file = server.index;
	std::string index_path = directory_path + "/" + index_file;
	if (fileExists(index_path) && isRegularFile(index_path))
		return index_path;
	return "";
}


std::string Router::resolvePath(const std::string& uri, const LocationConfig& location , const ServerConfig& server)
{
	std::string root;
	if (!location.root.empty())
		root = location.root;
	else if (!server.root.empty())
		root = server.root;
	else
		root = "";
	
	std::string relative_path;
	if (uri.length() > location.path.length())
		relative_path = uri.substr(location.path.length());
	else
		relative_path = "/";
	
	if (!relative_path.empty() && relative_path[0] != '/')
		relative_path = "/" + relative_path;
	
	std::string filesystem_path = root;
	if (!filesystem_path.empty() && filesystem_path[filesystem_path.length() - 1] == '/' && relative_path[0] == '/')
		filesystem_path += relative_path.substr(1);

	else if (!filesystem_path.empty() && filesystem_path[filesystem_path.length() - 1] != '/' && relative_path[0] != '/')
		filesystem_path += "/" + relative_path;
	else
		filesystem_path += relative_path;
	
	return filesystem_path;
}

bool Router::isCGIRequest(const std::string& file_path, const LocationConfig& location) const
{
	if (location.cgi_path.empty() || location.cgi_extension.empty())
		return false;

	size_t dot_pos = file_path.find_last_of('.');
	if (dot_pos == std::string::npos)
		return false;
	std::string extension = file_path.substr(dot_pos);
	if (extension == location.cgi_extension)
		return true;
	return false;
}

std::string Router::normalizePath(const std::string& path)
{
	std::vector<std::string> stack;
	std::istringstream stream(path);
	std::string part;

	while (std::getline(stream, part, '/'))
	{
		if (part.empty() || part == ".")
			continue;
		else if (part == "..")
		{
			if (!stack.empty())
				stack.pop_back();
		}
		else
			stack.push_back(part);
	}

	std::string res = "/";
	for (size_t i = 0; i < stack.size(); i++)
	{
		res += stack[i];
		if (i < stack.size() -1)
			res += "/";
	}
	return res;
}
bool Router::isDirectory(const std::string& path) const
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
		return false;
	return S_ISDIR(info.st_mode);
}



FileResponse Router::route(const Request& request, const ServerConfig& server)
{
	FileResponse response;
	
	const LocationConfig *location = findMatchingLocation(request.uri, server);
	if (location == NULL)//return to server root
	{
		if (server.root.empty())
			return serveErrorPage(404, server);
		std::string path = server.root;
		if (!path.empty() && path[path.size() -1] != '/')
			path += "/";

		path += server.index;
		if (!server.index.empty() && fileExists(path) && isRegularFile(path))
		{
			FileHandler handler;
			FileResponse fr = handler.serveFile(path);
			if (fr.status_code == 404)
    			return serveErrorPage(404, server);
			return fr;
		}
		return serveErrorPage(404, server);

	}
	
	if (!location->redirect.empty())
	{
		response.status_code = 301;
		response.mime_type = "text/html";
		response.body = location->redirect;
		return response;
	}


	MethodValidator validator;
	if (!validator.isMethodAllowed(request.method, location->allowed_methods))
	{
		response.status_code = 405;
		response.body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
		response.mime_type = "text/html";
		return response;
	}
	
	if (request.method == "POST" && !location->upload_path.empty())
	{
		UploadHandler upload;
		return upload.handleUpload(request, *location, server);
	}
	std::string path = resolvePath(request.uri, *location, server);
	
	if (isDirectory(path))
	{
		std::string index = resolveIndex(path, *location, server);
		if (!index.empty())
		{
			FileHandler handler;
			FileResponse fr = handler.serveFile(index);
			if (fr.status_code == 404)
    			return serveErrorPage(404, server);
			return fr;
		}

		if (location->autoindex)
		{
			DirectoryLister lister;
			return lister.generateDirectoryListing(path, request.uri);
		}
		else
			return serveErrorPage(403, server);

	}

	if (isCGIRequest(path, *location))
		{
			response.status_code = 0; //special code
			response.body = path;
			return response;
		}

	if (request.method == "GET")
	{
		FileHandler file;
    	FileResponse fr = file.serveFile(path);
    	if (fr.status_code == 404)
        	return serveErrorPage(404, server);
    	return fr;
	}

	else if (request.method == "POST")
	{
		UploadHandler upload;
		return upload.handleUpload(request, *location, server);
	}

	else if (request.method == "DELETE")
	{
		DeleteHandler del;
		return del.handleDelete(request, path, *location, server);
	}

	response.status_code = 405;
	return response;


}

FileResponse Router::serveErrorPage(int code, const ServerConfig& server)
{
	FileResponse response;
	response.status_code = code;
	response.mime_type = "text/html";
	std::map<int, std::string>::const_iterator it = server.error_page.find(code);
	if (it != server.error_page.end())
	{
		std::string page_path = server.root;
		if (!page_path.empty() && page_path[page_path.size()-1] != '/')
            page_path += "/";

		std::string uri = it->second;
		if (!uri.empty() && uri[0] == '/')
    	uri = uri.substr(1);
		page_path += uri;

		std::ifstream f(page_path.c_str());
		if (f.good())
		{
			std::string content((std::istreambuf_iterator<char>(f)),
                                 std::istreambuf_iterator<char>());
            response.body = content;
            return response;
		}
	}

	if (code == 404)
		response.body = "<html><body><h1>404 Not Found</h1></body></html>";
    else if (code == 403)
        response.body = "<html><body><h1>403 Forbidden</h1></body></html>";
    return response;
}
