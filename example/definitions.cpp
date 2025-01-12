#include "declarations.h"
#include <iostream>

int Boxable::WriteBoxed()
{
    std::cout << __PRETTY_FUNCTION__ << "\n";
    return 1;
}

int Boxable2::WriteBoxed()
{
    std::cout << __PRETTY_FUNCTION__ << "\n";
    return 1;
}

int VirtualBoxed::WriteBoxed()
{
    std::cout << __PRETTY_FUNCTION__ << "\n";
    return 1;
}

IBoxableBox BoxCreator()
{
    return IBoxableBox{VirtualBoxed{}};
}
