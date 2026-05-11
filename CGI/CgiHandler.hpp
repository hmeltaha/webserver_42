// #pragma once
#include <string>
#include <map>
#include <vector>

typedef std::map<std::string, std::string> StringMap;

struct CgiResult
{
    int         status_code;
    std::string headers;
    std::string body;
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
    
        static const int TIMEOUT = 10;
    public:
    CgiHandler();
    ~CgiHandler();

    CgiResult run(const std::string& method, const std::string& script,
                  const std::string& query, const std::string& body,
                  const StringMap& env);

};