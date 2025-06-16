#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QStackedWidget>
#include <QScrollBar>
#include <QSqlRecord>

// ------------------------------------------------------------------
// DatabaseManager Implementation

DatabaseManager::DatabaseManager() : dbPerformanceMonitor("Database Operations")
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("tictactoe.db");
}

DatabaseManager::~DatabaseManager()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool DatabaseManager::initializeDatabase()
{
    dbPerformanceMonitor.startMeasurement();

    if (!db.open()) {
        qDebug() << "Error opening database:" << db.lastError().text();
        dbPerformanceMonitor.stopMeasurement();
        return false;
    }

    QSqlQuery query;

    bool success = query.exec(
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password_hash TEXT NOT NULL, "
        "salt TEXT NOT NULL, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)"
        );

    if (!success) {
        qDebug() << "Error creating users table:" << query.lastError().text();
        dbPerformanceMonitor.stopMeasurement();
        return false;
    }

    success = query.exec(
        "CREATE TABLE IF NOT EXISTS game_history ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT NOT NULL, "
        "game_mode TEXT NOT NULL, "
        "winner TEXT NOT NULL, "
        "moves TEXT NOT NULL, "
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY(username) REFERENCES users(username))"
        );

    if (!success) {
        qDebug() << "Error creating game_history table:" << query.lastError().text();
        dbPerformanceMonitor.stopMeasurement();
        return false;
    }

    dbPerformanceMonitor.stopMeasurement();
    return true;
}

QString DatabaseManager::generateSalt()
{
    QByteArray salt;
    for (int i = 0; i < 32; ++i) {
        salt.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
    }
    return salt.toHex();
}

QString DatabaseManager::hashPassword(const QString& password, const QString& salt)
{
    QString saltedPassword = password + salt;
    QByteArray hash = QCryptographicHash::hash(saltedPassword.toUtf8(), QCryptographicHash::Sha256);
    return hash.toHex();
}

bool DatabaseManager::saveUser(const QString& username, const QString& password)
{
    dbPerformanceMonitor.startMeasurement();

    QString salt = generateSalt();
    QString hashedPassword = hashPassword(password, salt);

    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(hashedPassword);
    query.addBindValue(salt);

    bool result = query.exec();
    if (!result) {
        qDebug() << "Error saving user:" << query.lastError().text();
    }

    dbPerformanceMonitor.stopMeasurement();
    return result;
}

bool DatabaseManager::verifyUser(const QString& username, const QString& password)
{
    dbPerformanceMonitor.startMeasurement();

    QSqlQuery query;
    query.prepare("SELECT password_hash, salt FROM users WHERE username = ?");
    query.addBindValue(username);

    bool result = false;
    if (query.exec() && query.next()) {
        QString storedHash = query.value(0).toString();
        QString salt = query.value(1).toString();
        QString inputHash = hashPassword(password, salt);
        result = (storedHash == inputHash);
    }

    dbPerformanceMonitor.stopMeasurement();
    return result;
}

bool DatabaseManager::updateUserPassword(const QString& username, const QString& newPassword)
{
    dbPerformanceMonitor.startMeasurement();

    QString salt = generateSalt();
    QString hashedPassword = hashPassword(newPassword, salt);

    QSqlQuery query;
    query.prepare("UPDATE users SET password_hash = ?, salt = ? WHERE username = ?");
    query.addBindValue(hashedPassword);
    query.addBindValue(salt);
    query.addBindValue(username);

    bool result = query.exec() && query.numRowsAffected() > 0;
    if (!result) {
        qDebug() << "Error updating password:" << query.lastError().text();
    }

    dbPerformanceMonitor.stopMeasurement();
    return result;
}

bool DatabaseManager::saveGameRecord(const QString& username, const GameRecord& record)
{
    dbPerformanceMonitor.startMeasurement();

    QString movesStr;
    for (size_t i = 0; i < record.moves.size(); ++i) {
        const Move& m = record.moves[i];
        movesStr += QString::number(m.row) + "-" + QString::number(m.col) + "-" + QString(m.player);
        if (i < record.moves.size() - 1) {
            movesStr += ";";
        }
    }

    QSqlQuery query;
    query.prepare("INSERT INTO game_history (username, game_mode, winner, moves) VALUES (?, ?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(QString::fromStdString(record.mode));
    query.addBindValue(QString::fromStdString(record.winner));
    query.addBindValue(movesStr);

    bool result = query.exec();
    if (!result) {
        qDebug() << "Error saving game record:" << query.lastError().text();
    }

    dbPerformanceMonitor.stopMeasurement();
    return result;
}

std::vector<GameRecord> DatabaseManager::loadGameHistory(const QString& username)
{
    dbPerformanceMonitor.startMeasurement();

    std::vector<GameRecord> history;

    QSqlQuery query;
    query.prepare("SELECT game_mode, winner, moves, timestamp FROM game_history WHERE username = ? ORDER BY timestamp DESC");
    query.addBindValue(username);

    if (query.exec()) {
        while (query.next()) {
            GameRecord record;
            record.mode = query.value(0).toString().toStdString();
            record.winner = query.value(1).toString().toStdString();
            record.timestamp = query.value(3).toString();

            QString movesStr = query.value(2).toString();
            if (!movesStr.isEmpty()) {
                QStringList moveTokens = movesStr.split(";");
                for (const QString& token : moveTokens) {
                    QStringList parts = token.split("-");
                    if (parts.size() == 3) {
                        Move m;
                        m.row = parts[0].toInt();
                        m.col = parts[1].toInt();
                        m.player = parts[2].at(0).toLatin1();
                        record.moves.push_back(m);
                    }
                }
            }

            history.push_back(record);
        }
    } else {
        qDebug() << "Error loading game history:" << query.lastError().text();
    }

    dbPerformanceMonitor.stopMeasurement();
    return history;
}

// ------------------------------------------------------------------
// Helper functions for minimax evaluation

static bool evalIsWinner(const std::vector<std::vector<char>> &board, char player) {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == player && board[i][1] == player && board[i][2] == player)
            return true;
        if (board[0][i] == player && board[1][i] == player && board[2][i] == player)
            return true;
    }
    if (board[0][0] == player && board[1][1] == player && board[2][2] == player)
        return true;
    if (board[0][2] == player && board[1][1] == player && board[2][0] == player)
        return true;
    return false;
}

static bool evalIsFull(const std::vector<std::vector<char>> &board) {
    for (const auto &row : board)
        for (char cell : row)
            if (cell == ' ')
                return false;
    return true;
}

// ------------------------------------------------------------------
// GameBoard Implementation

GameBoard::GameBoard(QWidget *parent, int mode)
    : QWidget(parent), currentPlayer('X'), gameActive(true), gameMode(mode), aiPerformanceMonitor("AI Decision Making")
{
    mainLayout = new QGridLayout(this);
    mainLayout->setSpacing(0);
    initializeBoard();
}

GameBoard::~GameBoard()
{
    for (auto& row : buttons) {
        for (auto& button : row) {
            if (button) {
                delete button;
            }
        }
        row.clear();
    }
    buttons.clear();
    if (mainLayout) {
        delete mainLayout;
    }
}

void GameBoard::initializeBoard()
{
    board.assign(3, std::vector<char>(3, ' '));
    buttons.assign(3, std::vector<QPushButton*>(3, nullptr));
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            buttons[row][col] = new QPushButton(this);
            buttons[row][col]->setFixedSize(80, 80);
            buttons[row][col]->setStyleSheet("font: 24px;");
            mainLayout->addWidget(buttons[row][col], row, col);
            connect(buttons[row][col], &QPushButton::clicked, this, &GameBoard::onCellClicked);
        }
    }
    resetBoard();
}

void GameBoard::resetBoard()
{
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
        {
            board[row][col] = ' ';
            buttons[row][col]->setText("");
            buttons[row][col]->setEnabled(true);
            QString style = "QPushButton { background-color: #f0f0f0; border: 1px solid #ccc; }";
            buttons[row][col]->setStyleSheet(style);
        }
    currentPlayer = 'X';
    gameActive = true;
}

bool GameBoard::makeMove(int row, int col, char player)
{
    if (row >= 0 && row < 3 && col >= 0 && col < 3 &&
        board[row][col] == ' ' && gameActive)
    {
        board[row][col] = player;
        updateButtonText(row, col, player);
        return true;
    }
    return false;
}

bool GameBoard::checkWinner(char player)
{
    for (int i = 0; i < 3; ++i)
    {
        if (board[i][0] == player && board[i][1] == player && board[i][2] == player)
            return true;
        if (board[0][i] == player && board[1][i] == player && board[2][i] == player)
            return true;
    }
    if (board[0][0] == player && board[1][1] == player && board[2][2] == player)
        return true;
    if (board[0][2] == player && board[1][1] == player && board[2][0] == player)
        return true;
    return false;
}

bool GameBoard::isFull()
{
    for (const auto &row : board)
        for (char cell : row)
            if (cell == ' ')
                return false;
    return true;
}

void GameBoard::switchPlayer()
{
    currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
    if (gameMode == 2 && currentPlayer == 'O' && gameActive)
        triggerAiMove();
}

char GameBoard::getCurrentPlayer() const { return currentPlayer; }

std::vector<std::vector<char>> GameBoard::getBoard() const { return board; }

bool GameBoard::isEmpty(int row, int col) const
{
    return row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == ' ';
}

void GameBoard::updateButtonText(int row, int col, char text)
{
    buttons[row][col]->setText(QString(QChar(text)));
    buttons[row][col]->setEnabled(false);
    QString style;
    if (text == 'X')
        style = "QPushButton { background-color: #87CEFA; border: 1px solid #ccc; font: 24px; }";
    else
        style = "QPushButton { background-color: #FFA07A; border: 1px solid #ccc; font: 24px; }";
    buttons[row][col]->setStyleSheet(style);
}

void GameBoard::disableBoard()
{
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
            buttons[row][col]->setEnabled(false);
    gameActive = false;
}

void GameBoard::enableBoard()
{
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
            if (board[row][col] == ' ')
                buttons[row][col]->setEnabled(true);
    gameActive = true;
}

void GameBoard::onCellClicked()
{
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (!clickedButton || !gameActive)
        return;

    int row = -1, col = -1;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            if (buttons[i][j] == clickedButton)
            {
                row = i;
                col = j;
                break;
            }
        }
        if (row != -1)
            break;
    }
    if (row == -1 || col == -1)
        return;

    if (makeMove(row, col, currentPlayer))
    {
        emit moveMade(row, col, currentPlayer);
        if (checkWinner(currentPlayer))
        {
            QString winnerName = (currentPlayer == 'X') ? "You" : "AI";
            if (gameMode == 1)
                winnerName = (currentPlayer == 'X') ? "Player 1" : "Player 2";
            emit gameOver(winnerName);
            disableBoard();
            gameActive = false;
        }
        else if (isFull())
        {
            emit gameOver("Draw");
            disableBoard();
            gameActive = false;
        }
        else
        {
            switchPlayer();
        }
    }
}

void GameBoard::triggerAiMove()
{
    if (!gameActive || currentPlayer != 'O' || gameMode != 2)
        return;
    QTimer::singleShot(100, this, &GameBoard::aiMove);
}

void GameBoard::aiMove()
{
    if (!gameActive || currentPlayer != 'O' || gameMode != 2)
        return;

    QPoint bestMove = findBestMove();
    if (bestMove.x() != -1 && bestMove.y() != -1)
    {
        makeMove(bestMove.x(), bestMove.y(), currentPlayer);
        emit moveMade(bestMove.x(), bestMove.y(), currentPlayer);
        if (checkWinner(currentPlayer))
        {
            emit gameOver("AI");
            disableBoard();
            gameActive = false;
        }
        else if (isFull())
        {
            emit gameOver("Draw");
            disableBoard();
            gameActive = false;
        }
        else
        {
            switchPlayer();
        }
    }
}

// Original Minimax Algorithm
int GameBoard::minimax(std::vector<std::vector<char>> currentBoard, char player) {
    if (evalIsWinner(currentBoard, 'O'))
        return 10;
    if (evalIsWinner(currentBoard, 'X'))
        return -10;
    if (evalIsFull(currentBoard))
        return 0;

    if (player == 'O') {
        int bestScore = -1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (currentBoard[i][j] == ' ') {
                    currentBoard[i][j] = 'O';
                    int score = minimax(currentBoard, 'X');
                    currentBoard[i][j] = ' ';
                    bestScore = std::max(bestScore, score);
                }
            }
        }
        return bestScore;
    } else {
        int bestScore = 1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (currentBoard[i][j] == ' ') {
                    currentBoard[i][j] = 'X';
                    int score = minimax(currentBoard, 'O');
                    currentBoard[i][j] = ' ';
                    bestScore = std::min(bestScore, score);
                }
            }
        }
        return bestScore;
    }
}

QPoint GameBoard::findBestMove() {
    aiPerformanceMonitor.startMeasurement();

    int bestScore = -1000;
    QPoint bestMove = { -1, -1 };
    std::vector<std::vector<char>> boardCopy = board;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (boardCopy[i][j] == ' ') {
                boardCopy[i][j] = 'O';
                int score = minimax(boardCopy, 'X');
                boardCopy[i][j] = ' ';
                if (score > bestScore) {
                    bestScore = score;
                    bestMove = { i, j };
                }
            }
        }
    }

    aiPerformanceMonitor.stopMeasurement();
    return bestMove;
}

std::vector<QPoint> GameBoard::getAvailableMoves(const std::vector<std::vector<char>>& b)
{
    std::vector<QPoint> moves;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (b[i][j] == ' ') {
                moves.push_back(QPoint(i, j));
            }
        }
    }
    return moves;
}

// ------------------------------------------------------------------
// GameDialog Implementation

GameDialog::GameDialog(QWidget *parent)
    : QDialog(parent), gameBoard(nullptr), gameMode(0)
{
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mainLayout = new QGridLayout(this);
    verticalLayout = new QVBoxLayout();
    buttonLayout = new QHBoxLayout();

    pvpButton = new QPushButton("PvP (Two Players)", this);
    pvaiButton = new QPushButton("PvAI (Play against AI)", this);
    replayButton = new QPushButton("Replay Game", this);

    comboBoxGameList = new QComboBox(this);
    comboBoxGameList->addItem("Select a game...");
    connect(comboBoxGameList, QOverload<int>::of(&QComboBox::activated),
            this, &GameDialog::onComboBoxActivated);

    verticalLayout->addWidget(comboBoxGameList);
    verticalLayout->addWidget(replayButton);
    verticalLayout->addLayout(buttonLayout);
    buttonLayout->addWidget(pvpButton);
    buttonLayout->addWidget(pvaiButton);
    mainLayout->addLayout(verticalLayout, 0, 0);

    connect(pvpButton, &QPushButton::clicked, this, &GameDialog::on_pvpButton_clicked);
    connect(pvaiButton, &QPushButton::clicked, this, &GameDialog::on_pvaiButton_clicked);
    connect(replayButton, &QPushButton::clicked, this, &GameDialog::on_replayButton_clicked);

    player1Name = "Player 1";
    player2Name = "Player 2";
}

GameDialog::~GameDialog()
{
    if (gameBoard) {
        delete gameBoard;
        gameBoard = nullptr;
    }
    if (pvpButton) delete pvpButton;
    if (pvaiButton) delete pvaiButton;
    if (replayButton) delete replayButton;
    if (comboBoxGameList) delete comboBoxGameList;
    if (buttonLayout) delete buttonLayout;
    if (verticalLayout) delete verticalLayout;
    if (mainLayout) delete mainLayout;
}

void GameDialog::on_pvpButton_clicked()
{
    player1Name = QInputDialog::getText(this, "Player 1 Name", "Enter name for Player 1:", QLineEdit::Normal, "Player 1");
    if (player1Name.isEmpty())
        player1Name = "Player 1";
    player2Name = QInputDialog::getText(this, "Player 2 Name", "Enter name for Player 2:", QLineEdit::Normal, "Player 2");
    if (player2Name.isEmpty())
        player2Name = "Player 2";
    startGame(1);
}

void GameDialog::on_pvaiButton_clicked()
{
    player1Name = QInputDialog::getText(this, "Player Name", "Enter your name:", QLineEdit::Normal, "Player");
    if (player1Name.isEmpty())
        player1Name = "Player";
    startGame(2);
}

void GameDialog::startGame(int mode)
{
    gameMode = mode;
    if (gameBoard) {
        delete gameBoard;
    }
    gameBoard = new GameBoard(this, gameMode);
    connect(gameBoard, &GameBoard::moveMade, this, &GameDialog::recordMove);
    connect(gameBoard, &GameBoard::gameOver, this, &GameDialog::onGameOver);
    mainLayout->addWidget(gameBoard, 1, 0);
    gameBoard->show();
    moves.clear();

    MainWindow::gameMetrics.startGame();

    this->adjustSize();
}

void GameDialog::recordMove(int row, int col, char player)
{
    Move m;
    m.row = row;
    m.col = col;
    m.player = player;
    moves.push_back(m);
}

void GameDialog::onGameOver(const QString& winner)
{
    QString message = (winner == "Draw") ? "It's a draw!" : winner + " wins!";
    QMessageBox::information(this, "Game Over", message);

    GameRecord record;
    record.mode = (gameMode == 1) ? "PvP" : "PvAI";
    record.winner = winner.toStdString();
    record.moves = moves;
    record.timestamp = QDateTime::currentDateTime().toString();

    MainWindow::gameHistory.push_back(record);
    MainWindow::saveGameHistory();

    MainWindow::gameMetrics.endGame(winner);

    gameBoard->resetBoard();
    gameBoard->enableBoard();
    moves.clear();
}

void GameDialog::on_replayButton_clicked()
{
    comboBoxGameList->clear();
    if (MainWindow::gameHistory.empty())
    {
        QMessageBox::information(this, "Replay", "No games have been played yet.");
        return;
    }
    comboBoxGameList->addItem("Select a game...");
    for (size_t i = 0; i < MainWindow::gameHistory.size(); ++i)
    {
        QString itemText = QString("Game %1").arg(static_cast<int>(i + 1));
        comboBoxGameList->addItem(itemText);
    }
    comboBoxGameList->showPopup();
}

void GameDialog::onComboBoxActivated(int index)
{
    if (index <= 0 || index > static_cast<int>(MainWindow::gameHistory.size()))
    {
        QMessageBox::warning(this, "Replay", "Please select a valid game number.");
        return;
    }
    int selectedGameIndex = index - 1;
    const GameRecord &record = MainWindow::gameHistory[static_cast<size_t>(selectedGameIndex)];
    if (record.moves.empty())
    {
        QMessageBox::warning(this, "Replay", "No move data available for this game.");
        return;
    }
    ReplayDialog* replayDialog = new ReplayDialog(record.moves, this);
    replayDialog->exec();
    delete replayDialog;
}

// ------------------------------------------------------------------
// HistoryDialog Implementation

HistoryDialog::HistoryDialog(QWidget *parent)
    : QDialog(parent)
{
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint);
    mainLayout = new QVBoxLayout(this);
    historyTextEdit = new QTextEdit(this);
    historyTextEdit->setReadOnly(true);
    historyTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    closeButton = new QPushButton("Close", this);
    titleLabel = new QLabel("Game History", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(historyTextEdit);
    mainLayout->addWidget(closeButton);
    connect(closeButton, &QPushButton::clicked, this, &HistoryDialog::on_closeButton_clicked);
    displayGameHistory();
}

HistoryDialog::~HistoryDialog()
{
    if (historyTextEdit) delete historyTextEdit;
    if (closeButton) delete closeButton;
    if (titleLabel) delete titleLabel;
    if (mainLayout) delete mainLayout;
}

void HistoryDialog::setGameHistory(const std::vector<GameRecord>& history)
{
    gameHistory = history;
    displayGameHistory();
}

void HistoryDialog::displayGameHistory()
{
    historyTextEdit->clear();
    if (gameHistory.empty())
    {
        historyTextEdit->append("No games have been played yet.");
        return;
    }
    for (size_t i = 0; i < gameHistory.size(); ++i)
    {
        const GameRecord &record = gameHistory[i];
        QString gameInfo = QString("Game %1: Mode: %2, Winner: %3, Time: %4")
                               .arg(static_cast<int>(i + 1))
                               .arg(QString::fromStdString(record.mode))
                               .arg(QString::fromStdString(record.winner))
                               .arg(record.timestamp);
        historyTextEdit->append(gameInfo);
    }
    historyTextEdit->verticalScrollBar()->setValue(historyTextEdit->verticalScrollBar()->maximum());
}

void HistoryDialog::on_closeButton_clicked()
{
    this->close();
}

// ------------------------------------------------------------------
// ReplayDialog Implementation

ReplayDialog::ReplayDialog(const std::vector<Move>& moves, QWidget *parent)
    : QDialog(parent), movesToReplay(moves), moveIndex(0)
{
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setWindowTitle("Animated Replay");
    boardLayout = new QGridLayout(this);
    initializeBoard();
    closeButton = new QPushButton("Close", this);
    boardLayout->addWidget(closeButton, 3, 0, 1, 3);
    connect(closeButton, &QPushButton::clicked, this, &ReplayDialog::on_closeButton_clicked);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ReplayDialog::playNextMove);
    timer->start(500);
}

ReplayDialog::~ReplayDialog()
{
    if (timer) {
        timer->stop();
        delete timer;
    }
    for (QLabel* label : cellLabels) {
        if (label) delete label;
    }
    cellLabels.clear();
    if (closeButton) delete closeButton;
    if (boardLayout) delete boardLayout;
}

void ReplayDialog::initializeBoard()
{
    cellLabels.resize(9);
    for (int i = 0; i < 9; ++i)
    {
        cellLabels[static_cast<size_t>(i)] = new QLabel("", this);
        cellLabels[static_cast<size_t>(i)]->setFixedSize(80, 80);
        cellLabels[static_cast<size_t>(i)]->setFrameStyle(QFrame::Box | QFrame::Plain);
        cellLabels[static_cast<size_t>(i)]->setAlignment(Qt::AlignCenter);
        cellLabels[static_cast<size_t>(i)]->setStyleSheet("font: 24px; background-color: #f0f0f0;");
    }
    int index = 0;
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
            boardLayout->addWidget(cellLabels[static_cast<size_t>(index++)], row, col);
}

void ReplayDialog::playNextMove()
{
    if (moveIndex >= static_cast<int>(movesToReplay.size()))
    {
        timer->stop();
        return;
    }
    Move m = movesToReplay[static_cast<size_t>(moveIndex)];
    int index = m.row * 3 + m.col;
    if (index >= 0 && index < static_cast<int>(cellLabels.size()))
    {
        cellLabels[static_cast<size_t>(index)]->setText(QString(QChar(m.player)));
        if (m.player == 'X')
            cellLabels[static_cast<size_t>(index)]->setStyleSheet("font: 24px; background-color: #87CEFA; border: 1px solid #ccc;");
        else
            cellLabels[static_cast<size_t>(index)]->setStyleSheet("font: 24px; background-color: #FFA07A; border: 1px solid #ccc;");
    }
    moveIndex++;
}

void ReplayDialog::on_closeButton_clicked()
{
    this->close();
}

// ------------------------------------------------------------------
// MainWindow Implementation

std::vector<GameRecord> MainWindow::gameHistory;
QString MainWindow::currentUser = "";
DatabaseManager* MainWindow::dbManager = nullptr;
GameMetrics MainWindow::gameMetrics;
PerformanceMonitor MainWindow::loginPerformanceMonitor("Login Operations");

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), gameDialog(nullptr), historyDialog(nullptr)
{
    ui = new Ui::MainWindow();
    ui->setupUi(this);

    dbManager = new DatabaseManager();
    if (!dbManager->initializeDatabase()) {
        QMessageBox::critical(this, "Database Error", "Failed to initialize database!");
    }

    connect(ui->SignIn, &QPushButton::clicked, this, &MainWindow::signInButtonClicked);
    connect(ui->SignUp, &QPushButton::clicked, this, &MainWindow::signUpButtonClicked);
    connect(ui->playGameButton, &QPushButton::clicked, this, &MainWindow::playGameButtonClicked);
    connect(ui->viewHistoryButton, &QPushButton::clicked, this, &MainWindow::viewHistoryButtonClicked);

    gameDialog = new GameDialog(this);
    historyDialog = new HistoryDialog(this);
}

MainWindow::~MainWindow()
{
    if (ui) delete ui;
    if (gameDialog) delete gameDialog;
    if (historyDialog) delete historyDialog;
    if (dbManager) delete dbManager;
}

void MainWindow::signInButtonClicked()
{
    static int signInAttempts = 0;

    loginPerformanceMonitor.startMeasurement();

    QString username = ui->Username->text();
    QString password = ui->Password->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Sign In", "Username and password cannot be empty.");
        loginPerformanceMonitor.stopMeasurement();
        return;
    }

    if (dbManager->verifyUser(username, password))
    {
        signInAttempts = 0;
        loginPerformanceMonitor.stopMeasurement();

        QMessageBox::information(this, "Sign In", "Sign in successful!");
        currentUser = username;
        loadGameHistory();
        ui->stackedWidget->setCurrentIndex(1);
    }
    else
    {
        loginPerformanceMonitor.stopMeasurement();
        signInAttempts++;
        if (signInAttempts >= 3)
        {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, "Reset Password",
                "Would you like to reset your password?",
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes)
            {
                bool ok;
                QString newPassword = QInputDialog::getText(this, "Reset Password",
                                                            "Enter new password:", QLineEdit::Password, "", &ok);
                if (ok && !newPassword.isEmpty())
                {
                    if (dbManager->updateUserPassword(username, newPassword)) {
                        QMessageBox::information(this, "Password Updated", "Your password has been updated successfully.");
                        signInAttempts = 0;
                        currentUser = username;
                        loadGameHistory();
                        ui->stackedWidget->setCurrentIndex(1);
                    } else {
                        QMessageBox::critical(this, "Error", "Failed to update password.");
                    }
                }
                else
                {
                    QMessageBox::warning(this, "Warning", "Password was not updated. Please try signing in again.");
                }
            }
            else
            {
                QMessageBox::warning(this, "Sign In", "Incorrect username or password.");
            }
        }
        else
        {
            QMessageBox::warning(this, "Sign In", "Incorrect username or password.");
        }
    }
}

void MainWindow::signUpButtonClicked()
{
    loginPerformanceMonitor.startMeasurement();

    QString username = ui->Username->text();
    QString password = ui->Password->text();

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "Sign Up", "Username and password cannot be empty.");
        loginPerformanceMonitor.stopMeasurement();
        return;
    }

    if (dbManager->saveUser(username, password))
    {
        loginPerformanceMonitor.stopMeasurement();

        QMessageBox::information(this, "Sign Up", "Account created successfully!");
        currentUser = username;
        loadGameHistory();
        ui->stackedWidget->setCurrentIndex(1);
    }
    else
    {
        loginPerformanceMonitor.stopMeasurement();
        QMessageBox::warning(this, "Sign Up", "Username already exists or database error occurred.");
    }
}

void MainWindow::loadGameHistory()
{
    if (!currentUser.isEmpty() && dbManager) {
        gameHistory = dbManager->loadGameHistory(currentUser);
    }
}

void MainWindow::saveGameHistory()
{
    if (!currentUser.isEmpty() && dbManager && !gameHistory.empty()) {
        const GameRecord& lastRecord = gameHistory.back();
        dbManager->saveGameRecord(currentUser, lastRecord);
    }
}

void MainWindow::playGameButtonClicked()
{
    // Check if the user is signed in
    if (currentUser.isEmpty()) {
        QMessageBox::warning(this, "Sign In Required", "Please sign in or sign up before playing the game.");
        return;
    }
    if (!gameDialog)
        gameDialog = new GameDialog(this);
    gameDialog->exec();
}


void MainWindow::viewHistoryButtonClicked()
{
    if (!historyDialog)
        historyDialog = new HistoryDialog(this);
    historyDialog->setGameHistory(gameHistory);
    historyDialog->exec();
}
