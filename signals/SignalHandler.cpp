// #include "MainLoop.hpp"
#include "SignalHandler.hpp"


void signalHandler(int sig)
{
	(void)sig;
	if (sig == SIGINT)
	{
		std::cout << "Shutting the server down!!!" << std::endl;
		running = false;
	}
}

