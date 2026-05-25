# *This project has been created as part of the 42 curriculum by [Haleassa], [Hmeltaha], [Nagha]

# Webserv

## Quick Start

```bash
git clone <repository_url>
cd webserv
make
./webserv
```

Server runs by default on:

```text
http://localhost:8080
```
## Description

`webserv` is a non-blocking HTTP/1.1 web server written entirely in C++98.

The project recreates core web server behavior similar to NGINX and Apache in order to explore:

- HTTP protocol internals
- Socket programming
- Event-driven architectures
- CGI execution
- Request parsing
- Process and resource management

The server supports static file serving, CGI execution, uploads, multiple virtual servers, custom routing, and concurrent client handling using `poll()`.

The project was developed under the constraints of the 42 curriculum, using only low-level system calls and standard C++98 features.
---


## Features

- HTTP/1.1 support
- Non-blocking I/O
- `poll()`-based multiplexing
- Multiple simultaneous clients
- Static file serving
- File uploads
- CGI execution
- Custom configuration parsing
- Multiple virtual servers
- Route and location handling
- Custom error pages
- Directory listing (autoindex)
- Request parsing and response generation
- Persistent connections
- Timeout handling
- MIME type support

---

## Architecture Overview

The server follows an event-driven architecture using non-blocking sockets and `poll()` for multiplexing.

Main components include:

- **Parser** — Parses server configuration files
- **Server** — Manages listening sockets and client connections
- **Request Parser** — Parses incoming HTTP requests
- **Router** — Matches requests to configured locations
- **Handlers** — Process HTTP methods and generate responses
- **CGI Handler** — Executes CGI scripts using `fork()` and `execve()`
- **Response System** — Builds and sends HTTP responses

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

AI tools were used as supplementary learning resources during development.

They were primarily used for:
- Clarifying HTTP and networking concepts
- Understanding CGI behavior
- Exploring debugging strategies
- Reviewing architectural approaches
- Generating edge-case testing ideas

All design decisions, implementation, debugging, and integration were completed by the project authors.

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

