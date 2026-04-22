#include "DirectoryLister.hpp"

DirectoryLister::DirectoryLister(){}

DirectoryLister::DirectoryLister(const DirectoryLister& other)
{
	(void)other;
}

DirectoryLister& DirectoryLister::operator=(const DirectoryLister& other)
{
	(void)other;
	return *this;
}

DirectoryLister::~DirectoryLister(){}

static bool compare_entries(const DirEntry& a, const DirEntry& b)
{
	if (a.is_directory && !b.is_directory)
		return true;
	if (!a.is_directory && b.is_directory)
		return false;
	
	return a.name < b.name;
}

std::string DirectoryLister::generateHTML(const std::vector<DirEntry> &entries, const std::string& uri)
{
	std::stringstream html;
	html << "<!DOCTYPE html>\n";
	html << "<html>\n";
	html << "<head>\n";
	html << "<title>Index of " << uri <<"</title>\n";
	html << "<style>\n";
    html << "body { font-family: Arial, sans-serif; margin: 20px; }\n";
    html << "table { border-collapse: collapse; width: 100%; }\n";
    html << "th, td { text-align: left; padding: 8px; border-bottom: 1px solid #ddd; }\n";
    html << "th { background-color: #f2f2f2; }\n";
    html << "a { text-decoration: none; color: #0066cc; }\n";
    html << "a:hover { text-decoration: underline; }\n";
    html << "</style>\n";
    html << "</head>\n";
    html << "<body>\n";

	html << "<h1>Index of " << uri << "</h1>\n";

	html << "<table>\n";
	html << "<tr><th>Name</th> <th>Size</th> <th>Modifiend</th></tr>\n";

	if (uri != "/")
	{
		html << "<tr>";
		html << "<td> <a href=\"../\">../</a></td>";
		html << "<td>-</td>";
        html << "<td>-</td>";
    	html << "</tr>\n";
	}

	for (size_t i = 0; i < entries.size(); i++)
	{
		const DirEntry& entry = entries[i];

		std::string link = uri; //building a clickable link
		if (link[link.length() - 1] != '/')
			link += "/";
		link += entry.name;
		if (entry.is_directory)
			link += "/";
		html << "<tr>";

		html << "<td>";
		if (entry.is_directory)
			html << "📁 ";
		html << "<a href=\"" << link << "\">" << entry.name;
		if (entry.is_directory)
			html << "/";
		html << "</a></td>";
		
		html << "<td>";
		if (entry.is_directory)
			html << "-"; //no size for a directory only files
		else
			html << formatSize(entry.size);
		html << "</td>";
	
		html << "<td>" << formatTime(entry.modified_time);
		html << "</tr>\n";	
	}
	html << "</table>\n";
    html << "</body>\n";
    html << "</html>\n";
    
    return html.str();
}
std::string DirectoryLister::formatTime(time_t time)//computers store time as timestamp = number of seconds since Jan 1, 1970 (since the unix started)
{
	char buffer[80];
	struct tm* timeinfo = localtime(&time);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
	return std::string(buffer);
}
// 1 Byte (B)  = 1 byte
// 1 Kilobyte (KB) = 1024 bytes
// 1 Megabyte (MB) = 1024 KB = 1,048,576 bytes
// 1 Gigabyte (GB) = 1024 MB = 1,073,741,824 bytes

std::string DirectoryLister::formatSize(size_t bytes)//the size is stored in bytes
{
	std::stringstream ss;
	if (bytes < 1024)
		ss << bytes << " B";
	else if (bytes < 1024 * 1024)
		ss << (bytes / 1024) << " KB";
	else if (bytes < 1024 * 1024 * 1024)
		ss << (bytes / (1024 * 1024)) << " MB";
	else
		ss << (bytes / (1024 * 1024 * 1024)) << " GB";
	return ss.str();
}

FileResponse DirectoryLister::generateDirectoryListing(const std::string& directory_path, const std::string& uri)

{
	FileResponse response;
	
	DIR *dir = opendir(directory_path.c_str());
	if (dir == NULL)
	{
		response.status_code = 404;
		response.body = "Directory not found";
		return response;
	}

	std::vector<DirEntry> entries;

	struct dirent* entry;//system struct for holding info about a dir
	while ((entry = readdir(dir)) != NULL) 
	{
		std::string name = entry->d_name;
		if (name == "." || name == "..")
			continue;
		std::string path = directory_path + "/" + name;
		struct stat file_info;
		if (stat(path.c_str(), &file_info) != 0)
			continue;
		DirEntry dir_entry;
		dir_entry.name = name;
		dir_entry.is_directory = S_ISDIR(file_info.st_mode);
		dir_entry.size = file_info.st_size;
		dir_entry.modified_time = file_info.st_mtime;
		
		entries.push_back(dir_entry);
	}
	std::sort(entries.begin(), entries.end(), compare_entries);
	closedir(dir);

	std::string html = generateHTML(entries, uri);
	
	response.status_code = 200;
	response.mime_type = "text/html";
	response.body = html;
	response.content_length = html.length();
	
	return response;
}