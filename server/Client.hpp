#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include "../requests/HttpRequest.hpp"
#include "../requests/RequestParser.hpp"
#include "../response/HttpResponse.hpp"
#include "Server.hpp"

enum ClientState
{
	READING,	  // --> the start state of the client
	READING_BODY, // --> when i see "Content-Length"
	PROCESSING,	  // --> when reading the request is done
	WRITING		  // --> after reading the body
};

class Client
{
	private:
		int clientFd;
		size_t len_body;
		std::string reqBuff;
		std::string resBuff;
		ClientState state;
		std::string body;
		size_t bytes_send; //for sending response in chunks
		public:
			HttpRequest req;		// private or public ???????????????
			RequestParser parser;	// private or public ???????????????
			HttpResponse res;		// private or public ???????????????

			Client();
			Client(int fd);
			Client(const Client& other);
			Client& operator=(const Client& other);
			~Client();

			size_t getBytesSend() const;
			void setBytesSend(size_t size);

			int getClientFd() const;
			void setClientFd(int fd);

			ClientState getState() const;
			void setState(ClientState newState);

			void setReqBuff(const std::string& buff);
			std::string getReqBuff() const;

			void setResBuff(const std::string& buff);
			std::string& getResBuff() const;

			void addBodyToReq(const std::string& buff);
			void addToReqBuff(const std::string& buff);
};




#endif
