# include "CgiHandler.hpp"

void CgiHandler::close_pipes(int in_pipe[2], int out_pipe[2])
{
    close(in_pipe[0]);
    close(in_pipe[1]);
    close(out_pipe[0]);
    close(out_pipe[1]);
}

void CgiHandler::cleanup_env(char **env)
{
    if (!env)
        return;
    for (int i = 0; env[i]; i++)
        free(env[i]);
    free(env);
}

char *CgiHandler::join_key_value(const std::string& key, const std::string& value)
{
    return strdup((key + "=" + value).c_str());
}

CgiResult CgiHandler::make_failure()
{
    CgiResult f;
    f.status_code = 500;
    f.body = "Internal Server Error: CGI failed.";
    return f;
}

bool CgiHandler::setup_pipes(int in_pipe[2],
                             int out_pipe[2],
                             CgiResult &failure)
{
    (void)failure;
    if (pipe(in_pipe) == -1 || pipe(out_pipe) == -1)
    {
        std::cerr << "CGI: pipe() failed: " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

char **CgiHandler::build_env(const std::string& method,
                             const std::string& script,
                             const std::string& query,
                             const std::string& body,
                             const StringMap& extra_env,
                             int in_pipe[2],
                             int out_pipe[2],
                             CgiResult &failure)
{
    char **env = make_env_array(method, script, query, body, extra_env);

    if (!env)
    {
        close_pipes(in_pipe, out_pipe);
        std::cerr << "CGI: env allocation failed\n";
        failure.status_code = 500;
        return NULL;
    }

    return env;
}

void CgiHandler::run_child(int in_pipe[2], int out_pipe[2], const std::string& script, char **env)
{
    dup2(in_pipe[0], STDIN_FILENO);
    dup2(out_pipe[1], STDOUT_FILENO);

    close_pipes(in_pipe, out_pipe);

    // Get absolute path before changing directory
    char abs_path[PATH_MAX];
    if (!realpath(script.c_str(), abs_path))
    {
        std::cerr << "CGI: realpath failed for " << script << ": " << strerror(errno) << std::endl;
        _exit(1);
    }

    std::string abs_path_str(abs_path);
    size_t last_slash = abs_path_str.rfind('/');
    if (last_slash != std::string::npos)
    {
        std::string script_dir = abs_path_str.substr(0, last_slash);
        chdir(script_dir.c_str());
    }

    std::string interpreter = find_interpreter(script);

    char *argv[3];
    if (!interpreter.empty())
    {
        argv[0] = (char*)interpreter.c_str();
        argv[1] = abs_path;
        argv[2] = NULL;
    }
    else
    {
        argv[0] = abs_path;
        argv[1] = NULL;
    }

    execve(argv[0], argv, env);

    std::cerr << "CGI execve failed: " << strerror(errno) << std::endl;
    _exit(1);
}

void CgiHandler::write_body(int in_pipe[2], const std::string& body)
{
    if (body.empty())
	{
		close(in_pipe[1]);
        return;
	}

    size_t sent = 0;
    while (sent < body.size())
    {
        ssize_t w = write(in_pipe[1],
                          body.c_str() + sent,
                          body.size() - sent);
        if (w <= 0)
            break;
        sent += w;
    }

    close(in_pipe[1]);
	
}

void CgiHandler::prepare_parent_pipes(int in_pipe[2], int out_pipe[2])
{
    close(in_pipe[0]);
    close(out_pipe[1]);
    fcntl(out_pipe[0], F_SETFL, O_NONBLOCK);
}

pid_t CgiHandler::fork_cgi_process(int in_pipe[2], int out_pipe[2], const std::string& script, char **env, CgiResult &failure)
{
    (void)failure;
    pid_t pid = fork();

    if (pid == -1)
    {
        std::cerr << "CGI: fork failed: " << strerror(errno) << std::endl;
        cleanup_env(env);
        close_pipes(in_pipe, out_pipe);
        return -1;
    }

    if (pid == 0)
        run_child(in_pipe, out_pipe, script, env);

    return pid;
}

