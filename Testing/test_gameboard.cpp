#include "test_gameboard.h"

void TestGameBoard::testMakeMoveValid()
{
    GameBoard board(nullptr, 1);
    QVERIFY(board.makeMove(0, 0, 'X'));
    QCOMPARE(board.getBoard()[0][0], 'X');
}

void TestGameBoard::testMakeMoveInvalid()
{
    GameBoard board(nullptr, 1);
    board.makeMove(0, 0, 'X');
    QVERIFY(!board.makeMove(0, 0, 'O')); // already occupied
}

void TestGameBoard::testCheckWinnerRows()
{
    GameBoard board(nullptr, 1);
    board.makeMove(0, 0, 'X');
    board.makeMove(0, 1, 'X');
    board.makeMove(0, 2, 'X');
    QVERIFY(board.checkWinner('X'));
}
void TestGameBoard::testCheckWinnerColumns()
{
    GameBoard board(nullptr, 1); // PvP mode

    // Fill the first column with 'X'
    board.makeMove(0, 0, 'X');
    board.makeMove(1, 0, 'X');
    board.makeMove(2, 0, 'X');

    QVERIFY(board.checkWinner('X'));  // Expect 'X' to win
}
void TestGameBoard::testCheckWinnerDiagonals()
{
    GameBoard board(nullptr, 1);
    board.makeMove(0, 0, 'O');
    board.makeMove(1, 1, 'O');
    board.makeMove(2, 2, 'O');
    QVERIFY(board.checkWinner('O'));
}

void TestGameBoard::testIsFull()
{
    GameBoard board(nullptr, 1);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            board.makeMove(i, j, 'X');
    QVERIFY(board.isFull());
}
void TestGameBoard::testFullTurnCycle()
{
    GameBoard board(nullptr, 1); // PvP mode

    QVERIFY(board.makeMove(0, 0, 'X'));
    QVERIFY(!board.checkWinner('X')); // not yet winning
    board.switchPlayer();
    QCOMPARE(board.getCurrentPlayer(), 'O');

    QVERIFY(board.makeMove(0, 1, 'O'));
    QVERIFY(!board.checkWinner('O'));
    board.switchPlayer();
    QCOMPARE(board.getCurrentPlayer(), 'X');
}
void TestGameBoard::testAIIntegration()
{
    GameBoard board(nullptr, 2); // PvAI mode

    board.makeMove(0, 0, 'X');
    board.switchPlayer(); // should trigger AI move

    QTest::qWait(300); // wait for QTimer to fire and AI to move

    const auto b = board.getBoard();
    int countO = 0;
    for (auto& row : b)
        for (char cell : row)
            if (cell == 'O') countO++;

    QVERIFY(countO == 1); // AI must have played one move
    QCOMPARE(board.getCurrentPlayer(), 'X');
}
void TestGameBoard::testReplayIntegration()
{
    GameDialog dialog;

    GameRecord record;
    record.mode = "PvP";
    record.winner = "Player 1";
    record.moves = {{0, 0, 'X'}, {0, 1, 'O'}, {1, 1, 'X'}};

    MainWindow::gameHistory.clear();
    MainWindow::gameHistory.push_back(record);

    QComboBox* comboBox = dialog.findChild<QComboBox*>();
    QVERIFY(comboBox != nullptr);

    dialog.on_replayButton_clicked(); // should fill comboBox
    QCOMPARE(comboBox->count(), 2); // "Select..." + "Game 1"
}
