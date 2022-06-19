#include <iostream>
#include <string>

#include "widget.h"
#include <QApplication>
using namespace pxr;

int main(int argc, char* argv[])
{
    QApplication q(argc, argv);
    OpenGLWidget w;
    w.show();

    return q.exec();
}
