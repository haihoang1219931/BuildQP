#include <iostream>
#include <stdio.h>
#include "blinky.hpp"
#include "bsp.h"
using namespace std;

int main(int argc, char** argv)
{
    std::cout << "QP Version: " << QP_VERSION_STR << std::endl;
    QCoreApplication app(argc, argv);
    static QEvt const *blinkyQSto[10]; // Event queue storage for Blinky

    QF::init(); // initialize the framework and the underlying RT kernel

    // publish-subscribe not used, no call to QActive::psInit()
    // dynamic event allocation not used, no call to QF::poolInit()

    // instantiate and start the active objects...
    AO_Blinky->start(1U,                            // priority
                     blinkyQSto, Q_DIM(blinkyQSto), // event queue
                     nullptr, 0U);                // stack (unused)

    return QF::run(); // run the QF application
}
