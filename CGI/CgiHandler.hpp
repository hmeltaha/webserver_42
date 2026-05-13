// // #pragma once
// #ifndef CGI_HANDLER_HPP
// #define CGI_HANDLER_HPP
// #include <string>
// #include <map>
// #include <vector>
// #include <iostream>
// #include <sstream>
// #include <cstdlib>
// #include <cstring>
// #include <cerrno>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <poll.h>
// #include <signal.h>
// #include <fcntl.h>

// typedef std::map<std::string, std::string> StringMap;

// struct CgiResult
// {
//     int         status_code;
//     std::string headers;
//     std::string body;
// };

// class CgiHandler
// {
//     private:
//         char**      make_env_array(const std::string& method, const std::string& script,
//                                    const std::string& query, const std::string& body,
//                                    const StringMap& extra);
//         void        cleanup_env(char** env);
//         std::string find_interpreter(const std::string& script);
//         CgiResult   parse_output(const std::string& raw);
// 		void close_pipes(int in_pipe[2], int out_pipe[2]);
// 		char *CgiHandler::join_key_value(const std::string& key, const std::string& value);
// 		bool CgiHandler::setup_pipes(int in_pipe[2],int out_pipe[2], CgiResult &failure);
// 		CgiResult CgiHandler::make_failure();
// 		char **CgiHandler::build_env(const std::string& method, const std::string& script, const std::string& query, const std::string& body, const StringMap& extra_env, int in_pipe[2], int out_pipe[2], CgiResult &failure);
// 		void CgiHandler::run_child(int in_pipe[2], int out_pipe[2], const std::string& script, char **env);
// 		void CgiHandler::write_body(int in_pipe[2], const std::string& body);
// 		void CgiHandler::prepare_parent_pipes(int in_pipe[2], int out_pipe[2]);
// 		bool CgiHandler::collect_output(int fd, pid_t pid, std::string &output, bool &timed_out);
// 		CgiResult CgiHandler::timeout_failure(pid_t pid, CgiResult failure);
// 		CgiResult CgiHandler::finalize_result(pid_t pid, std::string &output, CgiResult failure);
// 		pid_t CgiHandler::fork_cgi_process(int in_pipe[2], int out_pipe[2], const std::string& script, char **env, CgiResult &failure);

//         static const int TIMEOUT = 10;
//     public:
//     CgiHandler();
//     ~CgiHandler();

//     CgiResult run(const std::string& method, const std::string& script,
//                   const std::string& query, const std::string& body,
//                   const StringMap& env);

// };

// #endif
