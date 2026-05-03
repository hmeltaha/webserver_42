#ifndef DIRECTORYLISTER_HPP
#define DIRECTORYLISTER_HPP

#include "FileHandler.hpp"
#include "../parser/ConfigParser.hpp"
#include <dirent.h>
#include <sys/stat.h> 
#include <vector>
#include <algorithm>
#include <sstream> 

struct DirEntry
{
	std::string name;
	bool is_directory;
	size_t size;
	time_t modified_time;

	DirEntry() : is_directory(false), size(0), modified_time(0){}
};


class DirectoryLister
{
	private:
		std::string generateHTML(const std::vector<DirEntry>& entries, 
                              const std::string& uri);
    	std::string formatSize(size_t bytes);
    	std::string formatTime(time_t time);
	
	public:
		DirectoryLister();
		DirectoryLister(const DirectoryLister& other);
		DirectoryLister& operator=(const DirectoryLister& other);
		~DirectoryLister();
		
		FileResponse generateDirectoryListing(const std::string& directory_path, 
                                       const std::string& uri);
};

#endif
