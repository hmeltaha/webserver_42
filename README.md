# *This project has been created as part of the 42 curriculum by Haleassa, Hmeltaha, Nagha

# Webserv

## Description

`webserv` is a custom HTTP/1.1 web server built from scratch in C++. The goal of this project is to understand how web servers work internally by recreating the core behavior of servers such as NGINX or Apache.

This project focuses on low-level network programming, socket management, request parsing, response generation, CGI execution, configuration handling, and multiplexing multiple client connections without blocking the server.

The server is capable of:

* Handling multiple client connections simultaneously
* Serving static files
* Processing HTTP requests and generating responses
* Supporting different HTTP methods
* Managing custom configuration files
* Executing CGI scripts
* Handling errors and timeouts
* Supporting uploads and file serving
* Managing multiple virtual servers

The project was developed following the constraints and standards of the 42 curriculum, with a strong focus on:

* Unix system programming
* Network communication
* Process management
* Non-blocking I/O
* Clean architecture and modular design
* Performance and stability

---

## Features

### Core HTTP Features

* HTTP/1.1 support
* Persistent connections
* Request parsing
* Response generation
* Custom error pages
* MIME type handling
* Directory handling

### Supported HTTP Methods

* `GET`
* `POST`
* `DELETE`

### Configuration System

* Multiple server blocks
* Multiple locations
* Custom ports and hosts
* Route configuration
* Root configuration
* Index files
* Upload paths
* Maximum body size
* Autoindex support
* CGI configuration
* Redirections

### CGI Support

* Python CGI execution
* CGI environment variables
* Timeout handling
* Output collection
* Error management

### Networking

* Socket programming using BSD sockets
* Non-blocking sockets
* `poll()` based multiplexing
* Multiple simultaneous clients
* Connection management

---

## Project Structure

```bash
webserv/
.
├── CGI
│   ├── CgiHandler.cpp
│   ├── CgiHandler.hpp
│   └── Cgi_utils.cpp
├── configs
│   └── webser.conf
├── handlers
│   ├── DeleteHandler.cpp
│   ├── DeleteHandler.hpp
│   ├── DirectoryLister.cpp
│   ├── DirectoryLister.hpp
│   ├── FileHandler.cpp
│   ├── FileHandler.hpp
│   ├── FileResponse.hpp
│   ├── UploadHandler.cpp
│   └── UploadHandler.hpp
├── main.cpp
├── Makefile
├── parser
│   ├── ConfigParser.cpp
│   ├── ConfigParser.hpp
│   ├── ConfigParserUtils.cpp
│   ├── LocationConfig.cpp
│   ├── LocationConfig.hpp
│   ├── ServerConfig.cpp
│   └── ServerConfig.hpp
├── README.md
├── requests
│   ├── HttpRequest.cpp
│   ├── HttpRequest.hpp
│   ├── RequestParser.cpp
│   ├── RequestParser.hpp
│   └── test.cpp
├── response
│   ├── HttpResponse.cpp
│   └── HttpResponse.hpp
├── router
│   ├── Router.cpp
│   └── Router.hpp
├── server
│   ├── Client.cpp
│   ├── Client.hpp
│   ├── MainLoop.cpp
│   ├── MainLoop.hpp
│   ├── Server.cpp
│   └── Server.hpp
├── signals
│   ├── SignalHandler.cpp
│   └── SignalHandler.hpp
├── test
│   ├── a.txt
│   ├── big.bin
│   ├── bigfile.bin
│   ├── big.txt
│   ├── cgi_tester
│   ├── checklist.txt
│   ├── empty.txt
│   ├── hack.txt
│   ├── hello.txt
│   └── test.txt
├── upload
│   ├── bigfile.bin
│   ├── empty.txt
│   ├── folder1
│   │   └── a.txt
│
└── utils
    ├── MethodValidator.cpp
    └── MethodValidator.hpp

---

## Instructions

### Requirements

Before compiling the project, make sure you have:

* `c++`
* `make`
* Linux or macOS environment
* C++98 compatible compiler

---

### Compilation

Clone the repository:

```bash
git clone <repository_url>
cd webserv
```

Compile the project:

```bash
   make
```

Available Makefile rules:

```bash
make        # Compile the project
make clean  # Remove object files
make fclean # Remove object files and executable
make re     # Recompile everything
```

---

### Running the Server

Run the server with the default configuration or with a custom configuration file:

```bash
./webserv
```

```bash
./webserv config/default.conf
```

---

## Example Configuration

```conf
server {
    listen 8080;
    server_name localhost;

    root ./www;
    index index.html;

    client_max_body_size 1000000;

    error_page 404 ./errors/404.html;

    location / {
        allowed_methods GET POST DELETE;
    }

    location /cgi-bin {
        cgi_extension .py;
        cgi_path /usr/bin/python3;
    }
}
```

---

## Usage Examples

### Access the Server

Open your browser:

```text
http://localhost:8080
```

---

### Test with curl

GET request:

```bash
curl http://localhost:8080
```

POST request:

```bash
curl -X POST -d "hello" http://localhost:8080/upload
```

DELETE request:

```bash
curl -X DELETE http://localhost:8080/file.txt
```

CGI request:

```bash
curl http://localhost:8080/cgi-bin/test.py
```

---

## Technical Choices

### Why `poll()`?

`poll()` was used to handle multiple client connections simultaneously while keeping the server non-blocking. This allows the server to efficiently monitor many file descriptors and react to incoming events.

### Why Non-Blocking I/O?

Non-blocking sockets prevent the server from freezing while waiting for a single client or CGI process. This improves scalability and responsiveness.

### CGI Execution Strategy

CGI scripts are executed using `fork()` and `execve()`, while pipes are used to communicate between the server and the CGI process.

Timeouts are implemented to prevent hanging CGI scripts from blocking server resources.

---

## Testing

The server was tested using:

* Browsers
* `curl`
* Custom CGI scripts
* Stress tests
* Invalid requests
* Large request bodies
* Multiple simultaneous clients

Example:

```bash
curl -v http://localhost:8080
```

---

## Resources

### HTTP and Networking

* RFC 7230 — Hypertext Transfer Protocol (HTTP/1.1)
* Beej's Guide to Network Programming
* MDN Web Docs — HTTP
* NGINX documentation
* Apache HTTP Server documentation
* IBM Socket Programming Tutorials

### CGI

* CGI/1.1 Specification
* Python CGI Documentation
* Unix process management documentation

---

## AI Usage

Artificial Intelligence tools were used as learning assistants during the development of this project.

AI was primarily used for:

* Understanding HTTP concepts
* Clarifying socket behavior
* Debugging specific issues
* Learning CGI execution flow
* Reviewing architecture ideas
* Generating test ideas and edge cases
* Explaining low-level networking concepts

---

## Challenges Faced

Some of the most challenging parts of the project included:

* Designing a reliable HTTP parser
* Managing partial requests
* Handling multiple clients correctly
* Implementing CGI communication
* Preventing blocking behavior
* Managing timeouts and process cleanup
* Correctly handling edge cases in HTTP requests

---

## Future Improvements

Possible future improvements include:

* HTTPS support
* Chunked transfer encoding
* Better logging system
* Advanced caching
* HTTP/2 support
* Improved configuration validation
* More extensive testing framework

---

## Authors

* Hmeltaha
* Nagha
* Haleassa
---

## License

This project was developed for educational purposes as part of the 42 curriculum.

