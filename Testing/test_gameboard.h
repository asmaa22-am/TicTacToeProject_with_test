#ifndef TEST_GAMEBOARD_H
#define TEST_GAMEBOARD_H

#include <QObject>
#include <QtTest>
#include "mainwindow.h"

class TestGameBoard : public QObject
{
    Q_OBJECT

private slots:
    void testMakeMoveValid();
    void testMakeMoveInvalid();
    void testCheckWinnerRows();
    void testCheckWinnerColumns();
    void testCheckWinnerDiagonals();
    void testIsFull();
    void testFullTurnCycle();
    void testAIIntegration();
    void testReplayIntegration();
};
#endif // TEST_GAMEBOARD_H
