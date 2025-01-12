#include "declarations.h"
#include <iostream>

int Boxable::WriteBoxed()
{
    std::cout << __FUNCTION__ << "\n";
    return 1;
}

int Boxable2::WriteBoxed()
{
    std::cout << __FUNCTION__ << "\n";
    return 1;
}

int VirtualBoxed::WriteBoxed()
{
    std::cout << __FUNCTION__ << "\n";
    return 1;
}

dramcryx::Box<IBoxable> BoxCreator()
{
    return dramcryx::Box<IBoxable>{VirtualBoxed{}};
}
