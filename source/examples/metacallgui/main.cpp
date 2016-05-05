#include "main_window.h"

#include <QApplication>


int main(int argc, char * argv[])
{
    QApplication application(argc, argv);

    main_window mw;

    mw.show();

    return application.exec();
}
