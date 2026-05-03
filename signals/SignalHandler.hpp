#ifndef SIGNALHANDLER_HPP
#define SIGNALHANDLER_HPP

#include <signal.h>
#include <iostream>

#include "../server/MainLoop.hpp"

void signalHandler(int sig);

#endif