#include "FileHandler.hpp"
#include <fstream>
#include <sys/stat.h>

FileHandler::FileHandler(){}
FileHandler::FileHandler(const FileHandler& o)
{
	(void)o;
}

FileHandler &FileHandler::operator=(const FileHandler& o)
{
	(void)o;
	return *this;
}

FileHandler::~FileHandler(){}

std::string FileHandler::getFileExtension(const std::string& path)
{
	size_t dot_pos = path.find_last_of('.');
	if (dot_pos == std::string::npos)
		return "";
	return path.substr(dot_pos);
}


std::string FileHandler::getMimeType(const std::string& extension)
{
	static std::map<std::string, std::string> mime_type;

	if (mime_type.empty())
	{
		mime_type[".html"] = "text/html";
		mime_type[".htm"] = "text/html";
		mime_type [".css"] = "text/css";
		mime_type [".js"] = "text/javascript";
		mime_type [".txt"] = "text/plain";

		mime_type [".jpg"] = "image/jpeg";
		mime_type [".jpeg"] = "image/jpeg";
		mime_type [".png"] = "image/png";
		mime_type [".gif"] = "image/gif";
		mime_type [".svg"] = "image/svg+xml";
		mime_type [".ico"] = "image/x-icon";

		mime_type [".pdf"] = "application/pdf";
		mime_type [".json"] = "application/json";
		mime_type [".xml"] = "application/xml";

		mime_type [".mp4"] = "video/mp4";
		mime_type [".mp3"] = "audio/mpeg";
		mime_type [".wav"] = "audio/wav";

 	}
	std::map<std::string, std::string>::iterator it = mime_type.find(extension);
	if (it != mime_type.end())
		return it->second;
	return "application/octet-stream";//unknown default
}

FileResponse FileHandler::serveFile(const std::string& file_path)
{
	FileResponse response;
	struct stat file_info;

	if (stat(file_path.c_str(), &file_info) != 0)
	{
		response.status_code = 404;
		response.body = "<html><body><h1>404 Not Found</h1></body></html>";
		response.mime_type = "text/html";
		return response;
	}

	if (!S_ISREG(file_info.st_mode))
	{
		response.status_code = 403;
		response.body = "<html><body><h1>403 Forbidden</h1></body></html>";
    	response.mime_type = "text/html";
		return response;
	}

	std::string extension = getFileExtension(file_path);
	std::string mime_type = getMimeType(extension);

	std::ifstream file(file_path.c_str(), std::ios::binary);//reads byte by byte
	std::string contents;
	char c;
	while (file.get(c))
	{
		contents += c;
	}

	response.status_code = 200; //success
	response.mime_type = mime_type;
	response.body = contents;
	response.content_length = contents.length();

	return response;
}
