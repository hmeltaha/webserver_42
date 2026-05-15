// #include "MainLoop.hpp"
#include "SignalHandler.hpp"


void addSignals()
{
	signal(SIGINT, signalHandler);
	signal(SIGTSTP, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGPIPE, SIG_IGN);
}

void signalHandler(int sig)
{
	if (sig == SIGINT || sig == SIGTSTP)
	{
		std::cout << std::endl << "Shutting the server down!!!" << std::endl;
		running = false;
	}
}

