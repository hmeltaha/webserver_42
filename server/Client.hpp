#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"
#include <string>


enum ClientState
{
	READING,
	PROCESSING,
	WRITING,
	READING_BODY
};

class Client
{
	private:
		int clientFd;
		std::string reqBuff;
		bool headersReceived;
		std::string resBuff;
		ClientState state;


	public:
		Client();
		Client(int fd);
		Client(const Client& other);
		Client& operator=(const Client& other);
		~Client();
		int getClientFd() const;
		void setClientFd(int fd);
		
		void addBodyToReq();
		bool getHeadersReceived() const ;
		void addToReqBuff(const std::string& buff);
		ClientState getState() const;
		void setState(ClientState newState);

		void setReqBuff(const std::string& buff);
		std::string getReqBuff() const;
		void setResBuff(const std::string& buff);
		std::string getResBuff() const;
};




#endif