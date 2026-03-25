#include <dlfcn.h>
#include <stdio.h>
#include "Core.hpp"
#include "IDisplayModule.hpp"
#include "DLLoader.hpp"
#include "IGameModule.hpp"
#include "MenuGame.hpp"



int main(void)
{
    Core core;
    core.run();

    return 0;
}

