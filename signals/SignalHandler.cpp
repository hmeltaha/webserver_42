// #include "MainLoop.hpp"
#include "SignalHandler.hpp"


void signalHandler(int sig)
{
	if (sig == SIGINT || sig == SIGTSTP)
	{
		std::cout << std::endl << "Shutting the server down!!!" << std::endl;
		running = false;
	}
}

