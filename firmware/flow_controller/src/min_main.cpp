/* min_main.cpp  – now just shells into ctrl module */
#include "ctrl/_ctrl.hpp"               // bring in ctrlSetup/ctrlLoop

void mainSetup()        { ctrlSetup(); }   // single init path
void mainLoop()         { ctrlLoop();  }   // reuse fully-featured loop
