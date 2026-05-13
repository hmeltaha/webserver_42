// #include "CgiHandler.hpp"


// CgiHandler::CgiHandler() {}
// CgiHandler::~CgiHandler() {}



// std::string CgiHandler::find_interpreter(const std::string& script)
// {
//     size_t dot = script.rfind('.');
//     if (dot == std::string::npos)
//         return "";

//     std::string extension = script.substr(dot);
//     if (extension == ".py")
// 		return "/usr/bin/python3";
//     if (extension == ".php")
// 		return "/usr/bin/php-cgi";
//     if (extension == ".pl")
// 		return "/usr/bin/perl";
//     if (extension == ".rb")
// 		return "/usr/bin/ruby";
//     return "";
// }
// // POST /cgi/test.py?name=hala HTTP/1.1
// // Host: localhost
// // Content-Length: 11
// char **CgiHandler::make_env_array(const std::string& method, const std::string& script,
//                                   const std::string& query, const std::string& body,
//                                   const StringMap& extra)
// {
//     std::vector<char *> vars; //tmp vectore to hold env vars before converting to char**

//     vars.push_back(join_key_value("REQUEST_METHOD",method));
//     vars.push_back(join_key_value("QUERY_STRING", query));
//     vars.push_back(join_key_value("SCRIPT_FILENAME",script));
//     vars.push_back(join_key_value("SCRIPT_NAME", script));
//     vars.push_back(join_key_value("SERVER_PROTOCOL","HTTP/1.1"));
//     vars.push_back(join_key_value("SERVER_SOFTWARE", "webserv/1.0"));
//     vars.push_back(join_key_value("GATEWAY_INTERFACE","CGI/1.1"));
//     vars.push_back(join_key_value("PATH_INFO",""));
//     vars.push_back(join_key_value("PATH_TRANSLATED",""));
//     vars.push_back(join_key_value("REDIRECT_STATUS","200")); // required by php-cgi

//     if (method == "POST")
//     {
//         std::ostringstream content_size;
//         content_size << body.size();
//         vars.push_back(join_key_value("CONTENT_LENGTH", content_size.str()));
//         vars.push_back(join_key_value("CONTENT_TYPE",   "application/x-www-form-urlencoded"));
//     }
//     else
//     {
//         vars.push_back(join_key_value("CONTENT_LENGTH", "0"));
//         vars.push_back(join_key_value("CONTENT_TYPE",   ""));
//     }

//     for (StringMap::const_iterator it = extra.begin(); it != extra.end(); it++)
//         vars.push_back(join_key_value(it->first, it->second));

//     vars.push_back(NULL);

//     char **result = (char **)malloc(vars.size() * sizeof(char *));
//     if (!result)
//         return NULL;
//     for (size_t i = 0; i < vars.size(); i++)
//         result[i] = vars[i];
//     return result;
// }



// CgiResult CgiHandler::parse_output(const std::string& raw)
// {
//     CgiResult result;
//     result.status_code = 200;

//     size_t      split   = raw.find("\r\n\r\n");
//     std::string newline = "\r\n";

//     if (split == std::string::npos)
//     {
//         split   = raw.find("\n\n");
//         newline = "\n";
//     }
//     if (split == std::string::npos)
//     {
//         result.body = raw;
//         return result;
//     }

//     std::string header_section = raw.substr(0, split);
//     result.body = raw.substr(split + newline.size() * 2);

//     std::istringstream header_stream(header_section);
//     std::string        line;
//     while (std::getline(header_stream, line))
//     {
//         if (!line.empty() && line[line.size() - 1] == '\r')
//             line.erase(line.size() - 1);
//         if (line.substr(0, 7) == "Status:")
//             result.status_code = std::atoi(line.substr(8).c_str());
//         else
//             result.headers += line + "\r\n";
//     }
//     return result;
// }

// CgiResult CgiHandler::run(const std::string& method, const std::string& script, const std::string& query, const std::string& body, const StringMap& extra_env)
// {
//     CgiResult failure = make_failure();

//     int child_stdin[2];
//     int child_stdout[2];

//     if (!setup_pipes(child_stdin, child_stdout, failure))
//         return failure;

//     char **env = build_env(method, script, query, body, extra_env, child_stdin, child_stdout, failure);
//     if (!env)
//         return failure;

//     pid_t pid = fork_cgi_process(child_stdin, child_stdout, script, env, failure);
//     if (pid == -1)
//         return failure;

//     cleanup_env(env);

//     write_body(child_stdin, body);
//     prepare_parent_pipes(child_stdin, child_stdout);

//     std::string output;
//     bool timed_out = false;

//     if (!collect_output(child_stdout[0], pid, output, timed_out))
//     {
//         if (timed_out)
//             return timeout_failure(pid, failure);
//     }

//     close(child_stdout[0]);

//     return finalize_result(pid, output, failure);
// }

// // CgiResult CgiHandler::run(const std::string& method, const std::string& script,
// //                            const std::string& query, const std::string& body,
// //                            const StringMap& extra_env)
// // {
// //     CgiResult failure;
// //     failure.status_code = 500;
// //     failure.body        = "Internal Server Error: CGI failed.";

// //     int child_stdin[2];
// //     int child_stdout[2];
// //     if (pipe(child_stdin) == -1 || pipe(child_stdout) == -1)
// //     {
// //         std::cerr << "CGI: pipe() failed: " << strerror(errno) << std::endl;
// //         return failure;
// //     }

// //     char **env = make_env_array(method, script, query, body, extra_env);
// //     if (!env)
// //     {
// //        close_pipes(child_stdin, child_stdout);
// // 	   std::cerr << "CGI: Failed to allocate environment variables." << std::endl;
// //     	return (failure);
// //     }

// //     pid_t child_pid = fork();
// //     if (child_pid == -1)
// //     {
// //         std::cerr << "CGI: fork() failed: " << strerror(errno) << std::endl;
// //         cleanup_env(env);
// //         close_pipes(child_stdin, child_stdout);
// //         return failure;
// //     }

// //     if (child_pid == 0)
// //     {
// //         dup2(child_stdin[0],  STDIN_FILENO);
// //         dup2(child_stdout[1], STDOUT_FILENO);
// //         close_pipes(child_stdin, child_stdout);
// //         size_t last_slash = script.rfind('/');
// //         if (last_slash != std::string::npos)
// //             chdir(script.substr(0, last_slash).c_str());

// //         std::string interpreter = find_interpreter(script);

// //         char *argv[3];
// //         if (!interpreter.empty())
// //         {
// //             argv[0] = const_cast<char *>(interpreter.c_str());
// //             argv[1] = const_cast<char *>(script.c_str());
// //             argv[2] = NULL;
// //         }
// //         else
// //         {
// //             argv[0] = const_cast<char *>(script.c_str());
// //             argv[1] = NULL;
// //         }

// //         execve(argv[0], argv, env);
// //         std::cerr << "CGI: execve failed: " << strerror(errno) << std::endl;
// //         _exit(1);
// //     }

// //     cleanup_env(env);
// //     close(child_stdin[0]);
// //     close(child_stdout[1]);
// //     fcntl(child_stdout[0], F_SETFL, O_NONBLOCK);

// //     if (!body.empty())
// //         write(child_stdin[1], body.c_str(), body.size());
// //     close(child_stdin[1]);

// //     std::string   output;
// //     char          buffer[4096];
// //     bool          timed_out    = false;
// //     time_t        start_time   = time(NULL);
// //     struct pollfd watch;

// //     watch.fd     = child_stdout[0];
// //     watch.events = POLLIN;

// //     while (true)
// //     {
// //         int seconds_passed = (int)(time(NULL) - start_time);
// //         int time_left      = (TIMEOUT - seconds_passed) * 1000;

// //         if (seconds_passed >= TIMEOUT)
// //         {
// //             timed_out = true;
// //             break;
// //         }

// //         int ready = poll(&watch, 1, time_left); //waiting for output or timeout
// //         if (ready == -1 && errno == EINTR)
// //             continue;
// //         if (ready <= 0)
// //         {
// //             timed_out = (ready == 0);
// //             break;
// //         }

// //         if (watch.revents & POLLIN)
// //         {
// //             ssize_t bytes_read = read(child_stdout[0], buffer, sizeof(buffer));
// //             if (bytes_read > 0)
// //                 output.append(buffer, bytes_read);
// //             else if (bytes_read == 0)
// //                 break;
// //         }
// //         if (watch.revents & POLLHUP)
// //         {
// //             ssize_t bytes_read;
// //             while ((bytes_read = read(child_stdout[0], buffer, sizeof(buffer))) > 0) // colleecting cgi output like stream until EOF
// //                 output.append(buffer, bytes_read);
// //             break;
// //         }
// //     }

// //     close(child_stdout[0]);

// //     if (timed_out)
// //     {
// //         kill(child_pid, SIGKILL);
// //         waitpid(child_pid, NULL, 0);
// //         failure.status_code = 504;
// //         failure.body        = "504 Gateway Timeout.";
// //         return failure;
// //     }

// //     int exit_code;
// //     waitpid(child_pid, &exit_code, 0);

// //     if (output.empty())
// //     {
// //         failure.body = "500 Internal Server Error: CGI produced no output.";
// //         return failure;
// //     }

// //     return parse_output(output);
// // }
