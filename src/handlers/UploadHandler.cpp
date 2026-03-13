#include "handlers/UploadHandler.hpp"
#include "utils/MethodValidator.hpp"

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
	return content.substr(pos, end - pos);
}

std::string UploadHandler::sanitizeFilename(std::string& filename)//protection from hacking
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
	if (mkdir(path.c_str(), 0755)) //0755 = rwxr-xr-x
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

FileResponse UploadHandler::handleUpload(const Request& request, const LocationConfig& location, const ServerConfig& server)
{
	FileResponse response;

	MethodValidator validator;
	if (!validator.isMethodAllowed("POST", location.allowed_methods))
	{
		response.status_code = 405;//method not allowed
		return response;
	}

	if (location.upload_path.empty())
	{
		response.status_code = 500;
		return response;
	}

	if (request.content_length > server.client_max_body_size)
	{
		response.status_code = 413;//payload too large
		return response;
	}

	std::map<std::string, std::string>::const_iterator it = request.header.find("Content-Disposition");
	std::string content_disposition = (it != request.header.end()) ? it->second : "";
	std::string filename = extractFilename(content_disposition);

	std::string sanitized = sanitizeFilename(filename);

	std::string target_path = location.upload_path + "/" + sanitized;

	target_path = resolveCollision(target_path);

	if (!createDirectoryIfNeeded(location.upload_path))
	{
		response.status_code = 500;
		return response;
	}

	std::ofstream file(target_path.c_str(), std::ios::binary);//to save the upload on the disk this is where actually the upload happens
	if (!file.is_open())
	{
		response.status_code = 500;
		return response;
	}
	file.write(request.body.c_str(), request.body.length());
	file.close();

	chmod(target_path.c_str(), 0644);//for saftey

	response.status_code = 201;//success
	response.body = "";
	response.content_length = 0;
	return response;
}
