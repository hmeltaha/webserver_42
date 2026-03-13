#include "server/MainLoop.hpp"

void signalHandler(int sig)
{
        (void)sig;
        running = false;
}
