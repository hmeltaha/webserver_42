#include "UploadHandler.hpp"
#include "../utils/MethodValidator.hpp"
#include <iterator>

// Orthodox Canonical Form

UploadHandler::UploadHandler() {
}

UploadHandler::UploadHandler(const UploadHandler& other) {
    (void)other;
}

UploadHandler& UploadHandler::operator=(const UploadHandler& other) {
    if (this != &other) {
    }
    return *this;
}

UploadHandler::~UploadHandler() {
}

std::string UploadHandler::extractFilename(const std::string& content)
{
	size_t pos = content.find("filename=\"");
	if (pos == std::string::npos)
		return "";
	pos += 10;
	size_t end = content.find("\"", pos);
	if (end == std::string::npos)
		return "";
	return content.substr(pos, end - pos);
}

std::string UploadHandler::sanitizeFilename(std::string filename)//protection from hacking
{
	size_t slash_pos = filename.find_last_of("/\\");
	if (slash_pos != std::string::npos)
		filename = filename.substr(slash_pos + 1);

	filename.erase(std::remove(filename.begin(), filename.end(), '\0'), filename.end());
	filename.erase(std::remove(filename.begin(), filename.end(), '\n'), filename.end());
	filename.erase(std::remove(filename.begin(), filename.end(), '\r'), filename.end());

	size_t dots_pos;
	while ((dots_pos = filename.find("..")) != std::string::npos)
		filename.erase(dots_pos, 2);

	if (filename.length() > 255)
		filename = filename.substr(0, 255);

	if (filename.empty())
		filename = "unnamed";

	return filename;
}

bool UploadHandler::createDirectoryIfNeeded(const std::string& path)
{
	struct  stat st;

	if (stat(path.c_str(), &st) == 0)
		return(S_ISDIR(st.st_mode));
	if (mkdir(path.c_str(), 0755) == 0) //0755 = rwxr-xr-x
		return true;
	if (errno == ENOENT) //parent doesnt exist
	{
		size_t last_slash = path.find_last_of('/');
        if (last_slash != std::string::npos)
		{
            std::string parent = path.substr(0, last_slash);
      		if (createDirectoryIfNeeded(parent))
          		return mkdir(path.c_str(), 0755) == 0;
		}
	}
	return false;

}

std::string UploadHandler::resolveCollision(const std::string& path)//for duplicate files
{
	if (access(path.c_str(), F_OK) != 0)//if the file doesnt exist
		return path;

	std::string base;
	std::string ext;
	size_t pos = path.find_last_of('.');
	if (pos != std::string::npos)
	{
		base = path.substr(0, pos);
		ext = path.substr(pos);
	}
	else
	{
		base = path;
		ext = "";
	}

	int counter = 1;
	while (1)
	{
		std::stringstream ss;
		ss << base << "_" << counter << ext;
		std::string new_path = ss.str();
		if (access(new_path.c_str(), F_OK) != 0)
			return new_path;
		counter++;
	}
}

FileResponse UploadHandler::handleUpload(const HttpRequest& request, const LocationConfig& location, const ServerConfig& server)
{
	FileResponse response;

	MethodValidator validator;
	if (!validator.isMethodAllowed("POST", location.allowed_methods))
	{
		response.status_code = 405;//method not allowed
		response.body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
		response.mime_type = "text/html";
		return response;
	}

	if (location.upload_path.empty())
	{
		response.status_code = 500;
		response.body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
		response.mime_type = "text/html";
		return response;
	}

	size_t content_length = 0;
	std::map<std::string, std::string>::const_iterator cl = request.headers.find("content-length");
	if (cl != request.headers.end())
    	content_length = static_cast<size_t>(std::atoi(cl->second.c_str()));

	if (content_length > server.client_max_body_size)
	{
		response.status_code = 413;//payload too large
		response.body = "<html><body><h1>413 Payload Too Large</h1></body></html>";
		response.mime_type = "text/html";

		return response;
	}

	std::string filename = "";

	std::map<std::string, std::string>::const_iterator it = request.headers.find("Content-Disposition");
	if (it != request.headers.end())
		filename = extractFilename(it->second);
//this is error

	if (filename.empty())
	{
		std::string uri = request.path;
		size_t pos = uri.find_last_of('/');
		if (pos != std::string::npos && pos + 1 < uri.size())
			filename = uri.substr(pos + 1);
	}
		// filename = extractFilename(request.body);

	std::string sanitized = sanitizeFilename(filename);

	std::string target_path = location.upload_path + "/" + sanitized;

	target_path = resolveCollision(target_path);

	if (!createDirectoryIfNeeded(location.upload_path))
	{
		response.status_code = 500;
		response.body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
		response.mime_type = "text/html";
		return response;
	}

	std::ofstream file(target_path.c_str(), std::ios::binary);//to save the upload on the disk this is where actually the upload happens
	if (!file.is_open())
	{
		response.status_code = 500;
		response.body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
		response.mime_type = "text/html";
		return response;
	}
	std::string content = extractFileContent(request.body);
	file.write(content.c_str(), content.length());
	file.close();

	chmod(target_path.c_str(), 0644);//for saftey

	response.status_code = 201;//success
	response.body = "<html><body><h1>201 Created</h1></body></html>";
	response.content_length = 0;
	return response;
}

std::string UploadHandler::extractFileContent(const std::string& body)
{

	size_t header_end = body.find("\r\n\r\n");// find end of part headers
	if (header_end == std::string::npos)
		return body;

	size_t content_start = header_end + 4;

	size_t content_end = body.rfind("\r\n--");
	if (content_end == std::string::npos || content_end <= content_start)
		return body.substr(content_start);

	return body.substr(content_start, content_end - content_start);
}
