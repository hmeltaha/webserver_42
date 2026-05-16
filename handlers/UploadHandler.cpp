#include "UploadHandler.hpp"
#include "../utils/MethodValidator.hpp"
#include <iterator>
#include <ctime>
#include <unistd.h>

UploadHandler::UploadHandler() {}
UploadHandler::UploadHandler(const UploadHandler& other) { (void)other; }
UploadHandler& UploadHandler::operator=(const UploadHandler& other) { (void)other; return *this; }
UploadHandler::~UploadHandler() {}

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

std::string UploadHandler::sanitizeFilename(std::string filename)
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

	return filename;
}

bool UploadHandler::createDirectoryIfNeeded(const std::string& path)
{
	struct stat st;
	if (stat(path.c_str(), &st) == 0)
		return S_ISDIR(st.st_mode);
	if (mkdir(path.c_str(), 0755) == 0)
		return true;
	if (errno == ENOENT)
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

std::string UploadHandler::resolveCollision(const std::string& path)
{
	if (access(path.c_str(), F_OK) != 0)
		return path;

	std::string base;
	std::string ext;
	// Find the last dot, but only in the filename part (after the last /)
	size_t last_slash = path.find_last_of('/');
	size_t search_start = (last_slash != std::string::npos) ? last_slash : 0;
	size_t pos = path.find_last_of('.', path.length());

	// Only treat as extension if the dot is after the last slash
	if (pos != std::string::npos && pos > search_start)
	{
		base = path.substr(0, pos);
		ext  = path.substr(pos);
	}
	else
	{
		base = path;
		ext  = "";
	}

	int counter = 1;
	while (true)
	{
		std::stringstream ss;
		ss << base << "_" << counter << ext;
		std::string new_path = ss.str();
		if (access(new_path.c_str(), F_OK) != 0)
			return new_path;
		counter++;
	}
}

FileResponse UploadHandler::handleUpload(const HttpRequest& request,
                                         const LocationConfig& location,
                                         const ServerConfig& server)
{
	FileResponse response;

	MethodValidator validator;
	const std::vector<std::string>& methods = !location.allowed_methods.empty()
		? location.allowed_methods : server.allowed_methods;
	if (!validator.isMethodAllowed("POST", methods))
	{
		response.status_code = 405;
		response.body        = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
		response.mime_type   = "text/html";
		return response;
	}

	if (location.upload_path.empty())
	{
		response.status_code = 500;
		response.body        = "<html><body><h1>500 Internal Server Error</h1></body></html>";
		response.mime_type   = "text/html";
		return response;
	}

	size_t content_length = 0;
	std::map<std::string, std::string>::const_iterator cl =
		request.headers.find("content-length");
	if (cl != request.headers.end())
		content_length = static_cast<size_t>(std::atoi(cl->second.c_str()));
	if (content_length > server.client_max_body_size)
	{
		response.status_code = 413;
		response.body        = "<html><body><h1>413 Payload Too Large</h1></body></html>";
		response.mime_type   = "text/html";
		return response;
	}

	std::string filename;
	std::string target_dir = location.upload_path;

	// Extract nested path from request.path (e.g., /upload/hba/jjj -> hba/jjj)
	std::string nested_path = "";
	if (request.path.length() > location.path.length())
	{
		nested_path = request.path.substr(location.path.length());
		if (!nested_path.empty() && nested_path[0] == '/')
			nested_path = nested_path.substr(1);

		// Remove trailing slash if present
		if (!nested_path.empty() && nested_path[nested_path.length() - 1] == '/')
			nested_path = nested_path.substr(0, nested_path.length() - 1);
	}

	// If nested_path is provided, use it as the target directory/filename
	if (!nested_path.empty())
	{
		// Find the last slash to separate directory from filename
		size_t last_slash = nested_path.find_last_of('/');
		if (last_slash != std::string::npos)
		{
			// Has nested directories: hba/jjj -> target_dir += /hba, filename = jjj
			target_dir += "/" + nested_path.substr(0, last_slash);
			filename = nested_path.substr(last_slash + 1);
		}
		else
		{
			// Just a filename: jjj -> filename = jjj
			filename = nested_path;
		}
	}

	std::map<std::string, std::string>::const_iterator cd =
		request.headers.find("content-disposition");
	if (cd != request.headers.end() && filename.empty())
		filename = extractFilename(cd->second);

	if (filename.empty())
		filename = extractFilename(request.body);

	filename = sanitizeFilename(filename);
	if (filename.empty())
	{
		// Generate default filename with counter: upload_1, upload_2, etc.
		filename = "upload";
	}

	if (!createDirectoryIfNeeded(target_dir))
	{
		response.status_code = 500;
		response.body        = "<html><body><h1>500 Internal Server Error</h1></body></html>";
		response.mime_type   = "text/html";
		return response;
	}

	std::string target_path = target_dir + "/" + filename;

	target_path = resolveCollision(target_path);

	std::ofstream file(target_path.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		response.status_code = 500;
		response.body        = "<html><body><h1>500 Internal Server Error</h1></body></html>";
		response.mime_type   = "text/html";
		return response;
	}

	std::string content = extractFileContent(request.body);
	file.write(content.c_str(), content.length());
	file.close();
	chmod(target_path.c_str(), 0644);

	response.status_code    = 201;
	response.body           = "<html><body><h1>201 Created</h1></body></html>";
	response.mime_type   = "text/html";
	response.content_length = 0;
	return response;
}

std::string UploadHandler::extractFileContent(const std::string& body)
{
	std::string separator = "\r\n\r\n";
	size_t header_end = body.find(separator);
	if (header_end == std::string::npos)
	{
		separator  = "\n\n";
		header_end = body.find(separator);
	}
	if (header_end == std::string::npos)
		return body;

	size_t content_start = header_end + separator.length();

	size_t content_end = body.rfind("\r\n--");
	if (content_end == std::string::npos)
		content_end = body.rfind("\n--");

	if (content_end == std::string::npos || content_end <= content_start)
		return body.substr(content_start);

	return body.substr(content_start, content_end - content_start);
}
