// #include "MainLoop.hpp"
#include "SignalHandler.hpp"
#include <unistd.h>


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
		const char msg[] = "\nShutting the server down!!!\n";
		write(STDOUT_FILENO, msg, sizeof(msg) - 1);
		running = false;
	}
}

