#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include "../server/Client.hpp"

typedef std::map<std::string, std::string> StringMap;

struct CgiResult
{
	int		 status_code;
	std::string headers;
	std::string body;
};

struct CgiProcess
{
	pid_t	pid;
	int		in_fd;
	int		out_fd;
	std::string output;
	bool	 timed_out;
	int clientFd;
	time_t	start_time;
};


class CgiHandler
{
    private:
	 char**      make_env_array(const std::string& method, const std::string& script,
                                   const std::string& query, const std::string& body,
                                   const StringMap& extra);
        void        cleanup_env(char** env);
        std::string find_interpreter(const std::string& script);
        CgiResult   parse_output(const std::string& raw);
		void close_pipes(int in_pipe[2], int out_pipe[2]);
		char *join_key_value(const std::string& key, const std::string& value);
		bool setup_pipes(int in_pipe[2],int out_pipe[2], CgiResult &failure);
		CgiResult make_failure();
		char **build_env(const std::string& method, const std::string& script, const std::string& query, const std::string& body, const StringMap& extra_env, int in_pipe[2], int out_pipe[2], CgiResult &failure);
		void run_child(int in_pipe[2], int out_pipe[2], const std::string& script, char **env);
		void write_body(int in_pipe[2], const std::string& body);
		void prepare_parent_pipes(int in_pipe[2], int out_pipe[2]);
		pid_t fork_cgi_process(int in_pipe[2], int out_pipe[2], const std::string& script, char **env, CgiResult &failure);
    
	public:

		std::map <int, CgiProcess> active_processes;
		CgiHandler();
		~CgiHandler();
		int startCgi(int clientFd, const std::string& method, const std::string& script,
				const std::string& query, const std::string& body,
					const StringMap& extra_env, int epollFD);
		bool handleCgiOutput(int pipeFd, int epollFD,
						std::map<int, Client>& clients);
};

#endif
