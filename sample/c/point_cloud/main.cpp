#include <iostream>
#include <sstream>

#include "orbbec.hpp"

int main( int argc, char* argv[] )
{
    try{
        orbbec orbbec;
        orbbec.run();
    }
    catch( const std::runtime_error& error ){
        std::cout << error.what() << std::endl;
    }

    return 0;
}