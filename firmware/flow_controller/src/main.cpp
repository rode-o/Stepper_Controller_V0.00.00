/*  main.cpp – shells straight into MainCtrl
 *  ---------------------------------------
 *  The sketch (.ino) just calls mainSetup / mainLoop.
 */

#include "ctrl/main_ctrl/main_ctrl.hpp"   // MainCtrl::setup / loop100Hz

void mainSetup()
{
    MainCtrl::setup();        // initialise controllers, sensors, state, …
}

void mainLoop()
{
    MainCtrl::loop100Hz();    // 100 Hz scheduler; returns immediately if <10 ms
}
