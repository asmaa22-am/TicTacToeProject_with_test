#include "mainwindow.h"

#include <QApplication>
#include "test_gameboard.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    /*MainWindow w;
    w.show();*/
    TestGameBoard test;
    return QTest::qExec(&test, argc, argv);
}
