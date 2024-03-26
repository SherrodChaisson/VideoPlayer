#include "videoplayer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VideoPlayer w;
    w.resize(1200,1000);
    w.show();

    return a.exec();
}
