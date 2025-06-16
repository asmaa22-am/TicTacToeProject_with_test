#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QTimer>
#include <QTextEdit>
#include <QComboBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLineEdit>
#include <QElapsedTimer>
#include <QDebug>
#include <QDateTime>
#include <QFileInfo>
#include <QCoreApplication>
#include <chrono>
#include <vector>
#include <QString>
#include <string>
#include <fstream>
#include <algorithm>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Forward declarations
class GameBoard;

// --- Performance Monitor Class ---
class PerformanceMonitor {
private:
    QElapsedTimer timer;
    std::vector<double> measurements;
    QString operationName;

public:
    PerformanceMonitor(const QString& name = "") : operationName(name) {}

    void startMeasurement() {
        timer.start();
    }

    double stopMeasurement() {
        double elapsed = timer.nsecsElapsed() / 1000000.0;
        measurements.push_back(elapsed);
        qDebug() << operationName << "execution time:" << elapsed << "ms";
        return elapsed;
    }

    double getAverageTime() const {
        if (measurements.empty()) return 0.0;
        double sum = 0.0;
        for (double time : measurements) {
            sum += time;
        }
        return sum / measurements.size();
    }

    double getMaxTime() const {
        if (measurements.empty()) return 0.0;
        return *std::max_element(measurements.begin(), measurements.end());
    }

    double getMinTime() const {
        if (measurements.empty()) return 0.0;
        return *std::min_element(measurements.begin(), measurements.end());
    }

    size_t getMeasurementCount() const {
        return measurements.size();
    }

    void saveToFile(const QString& filename) const {
        std::ofstream file(filename.toStdString());
        file << "Operation: " << operationName.toStdString() << "\n";
        file << "Total measurements: " << measurements.size() << "\n";
        file << "Average time: " << getAverageTime() << " ms\n";
        file << "Maximum time: " << getMaxTime() << " ms\n";
        file << "Minimum time: " << getMinTime() << " ms\n";
        file.close();
    }
};

// --- Game Metrics Class ---
class GameMetrics {
private:
    int totalGames;
    int playerWins;
    int aiWins;
    int draws;
    std::vector<double> gameDurations;
    PerformanceMonitor gameTimer;

public:
    GameMetrics() : totalGames(0), playerWins(0), aiWins(0), draws(0), gameTimer("Game Duration") {}

    void startGame() {
        gameTimer.startMeasurement();
    }

    void endGame(const QString& winner) {
        double duration = gameTimer.stopMeasurement();
        gameDurations.push_back(duration);
        totalGames++;

        if (winner == "You" || winner == "Player 1" || winner == "Player 2") {
            playerWins++;
        } else if (winner == "AI") {
            aiWins++;
        } else {
            draws++;
        }
    }

    double getAverageGameDuration() const {
        if (gameDurations.empty()) return 0.0;
        double sum = 0.0;
        for (double duration : gameDurations) {
            sum += duration;
        }
        return sum / gameDurations.size();
    }

    int getTotalGames() const { return totalGames; }
    int getPlayerWins() const { return playerWins; }
    int getAiWins() const { return aiWins; }
    int getDraws() const { return draws; }
};

// --- Move Struct Definition ---
struct Move {
    int row;
    int col;
    char player;
};

// --- GameRecord Struct Definition ---
struct GameRecord {
    std::string mode;
    std::string winner;
    std::vector<Move> moves;
    QString timestamp;
};

// --- DatabaseManager Class Definition ---
class DatabaseManager
{
private:
    QSqlDatabase db;
    PerformanceMonitor dbPerformanceMonitor;

public:
    DatabaseManager();
    ~DatabaseManager();

    bool initializeDatabase();
    bool saveUser(const QString& username, const QString& password);
    bool verifyUser(const QString& username, const QString& password);
    bool updateUserPassword(const QString& username, const QString& newPassword);
    bool saveGameRecord(const QString& username, const GameRecord& record);
    std::vector<GameRecord> loadGameHistory(const QString& username);

    PerformanceMonitor& getPerformanceMonitor() { return dbPerformanceMonitor; }

private:
    QString hashPassword(const QString& password, const QString& salt);
    QString generateSalt();
};

// --- GameBoard Class Definition ---
class GameBoard : public QWidget
{
    Q_OBJECT

public:
    GameBoard(QWidget *parent = nullptr, int mode = 0);
    ~GameBoard();

    void initializeBoard();
    void resetBoard();
    bool makeMove(int row, int col, char player);
    bool checkWinner(char player);
    bool isFull();
    void switchPlayer();
    char getCurrentPlayer() const;
    std::vector<std::vector<char>> getBoard() const;
    bool isEmpty(int row, int col) const;
    void updateButtonText(int row, int col, char text);
    void disableBoard();
    void enableBoard();

    PerformanceMonitor& getAiPerformanceMonitor() { return aiPerformanceMonitor; }

public slots:
    void onCellClicked();
    void aiMove();
    void triggerAiMove();

signals:
    void gameOver(const QString& winner);
    void moveMade(int row, int col, char player);

private:
    std::vector<std::vector<char>> board;
    std::vector<std::vector<QPushButton*>> buttons;
    QGridLayout* mainLayout;
    char currentPlayer;
    bool gameActive;
    int gameMode;
    PerformanceMonitor aiPerformanceMonitor;

    QPoint findBestMove();
    int minimax(std::vector<std::vector<char>> currentBoard, char player);
    std::vector<QPoint> getAvailableMoves(const std::vector<std::vector<char>>& b);
};

// --- GameDialog Class Definition ---
class GameDialog : public QDialog
{
    Q_OBJECT

public:
    GameDialog(QWidget *parent = nullptr);
    ~GameDialog();

    GameBoard* gameBoard;

public slots:
    void on_pvpButton_clicked();
    void on_pvaiButton_clicked();
    void on_replayButton_clicked();
    void onComboBoxActivated(int index);
    void onGameOver(const QString& winner);
    void recordMove(int row, int col, char player);

private:
    void startGame(int mode);

    QGridLayout* mainLayout;
    QVBoxLayout* verticalLayout;
    QHBoxLayout* buttonLayout;
    QPushButton* pvpButton;
    QPushButton* pvaiButton;
    QPushButton* replayButton;
    QComboBox* comboBoxGameList;
    QString player1Name;
    QString player2Name;
    int gameMode;
    std::vector<Move> moves;
};

// --- HistoryDialog Class Definition ---
class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    HistoryDialog(QWidget *parent = nullptr);
    ~HistoryDialog();
    void setGameHistory(const std::vector<GameRecord>& history);

private slots:
    void on_closeButton_clicked();

private:
    void displayGameHistory();

    QVBoxLayout* mainLayout;
    QPushButton* closeButton;
    std::vector<GameRecord> gameHistory;
    QTextEdit* historyTextEdit;
    QLabel* titleLabel;
};

// --- ReplayDialog Class Definition ---
class ReplayDialog : public QDialog
{
    Q_OBJECT
public:
    ReplayDialog(const std::vector<Move>& moves, QWidget* parent = nullptr);
    ~ReplayDialog();

private slots:
    void playNextMove();
    void on_closeButton_clicked();

private:
    void initializeBoard();

    QGridLayout* boardLayout;
    std::vector<QLabel*> cellLabels;
    QTimer* timer;
    std::vector<Move> movesToReplay;
    int moveIndex;
    QPushButton* closeButton;
};

// --- MainWindow Class Definition ---
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static std::vector<GameRecord> gameHistory;
    static QString currentUser;
    static DatabaseManager* dbManager;
    static GameMetrics gameMetrics;
    static PerformanceMonitor loginPerformanceMonitor;

    static void saveGameHistory();

private slots:
    void signInButtonClicked();
    void signUpButtonClicked();
    void playGameButtonClicked();
    void viewHistoryButtonClicked();

private:
    Ui::MainWindow *ui;
    GameDialog* gameDialog;
    HistoryDialog* historyDialog;

    static void loadGameHistory();
};

#endif // MAINWINDOW_H
