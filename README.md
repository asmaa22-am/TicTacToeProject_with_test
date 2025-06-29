# Advanced Tic-Tac-Toe Game

A feature-rich, cross-platform Tic-Tac-Toe game built with Qt/C++ that includes user authentication, AI gameplay, game history tracking, and comprehensive performance monitoring.

## 🎮 Features

### Core Gameplay
- **Player vs Player (PvP)**: Two human players can compete against each other
- **Player vs AI (PvAI)**: Challenge an intelligent AI opponent using the Minimax algorithm
- **Game Replay System**: Watch animated replays of previously played games
- **Visual Board Interface**: Clean, intuitive 3x3 grid with color-coded moves

### User Management
- **Secure Authentication**: User registration and login system
- **Password Security**: SHA-256 hashing with salt for password protection
- **Password Recovery**: Built-in password reset functionality after failed login attempts
- **User Sessions**: Persistent user sessions throughout gameplay

### Data Persistence
- **SQLite Database**: Local database storage for users and game records
- **Game History**: Complete tracking of all played games with timestamps
- **Move Recording**: Detailed move-by-move game data for replay functionality

### Performance Monitoring
- **Real-time Metrics**: Monitor database operations, AI decision-making, and login performance
- **System Resources**: Track memory usage and CPU utilization
- **Performance Analytics**: Detailed timing statistics for all major operations
- **Game Statistics**: Win/loss ratios, average game duration, and gameplay metrics
- 
###  CI/CD with GitHub Actions
This project uses GitHub Actions for continuous integration and deployment. Our automated pipeline ensures code quality and cross-platform compatibility.

## 🛠️ Technical Architecture

### Core Classes

#### `MainWindow`
- Main application window and entry point
- Handles user authentication and navigation
- Manages global application state

#### `GameBoard`
- Core game logic and board management
- Minimax AI algorithm implementation
- Move validation and win condition checking

#### `DatabaseManager`
- SQLite database operations
- User authentication and password management
- Game history persistence

#### `PerformanceMonitor`
- Real-time performance tracking
- System resource monitoring (Windows-specific)
- Timing analysis for operations

#### `GameMetrics`
- Game statistics collection
- Win/loss tracking
- Performance analytics

### Dialog Classes
- **`GameDialog`**: Main game interface and mode selection
- **`HistoryDialog`**: Game history viewing
- **`ReplayDialog`**: Animated game replay functionality

## 🚀 Getting Started

### Prerequisites
- Qt 6.x
- C++ compiler
- SQLite support
- qmake build system

## 🎯 How to Use

### Getting Started
1. **Launch the application**
2. **Create an account** or **sign in** with existing credentials
3. **Choose your game mode** from the main menu

### Game Modes

#### Player vs Player (PvP)
- Enter names for both players
- Take turns clicking on the board
- First player uses 'X', second player uses 'O'

#### Player vs AI (PvAI)
- Enter your player name
- You play as 'X', AI plays as 'O'
- AI uses advanced Minimax algorithm for optimal moves

### Additional Features

#### View Game History
- Access complete history of all played games
- See game modes, winners, and timestamps
- Navigate through chronological game records

#### Replay Games
- Select any previous game from the dropdown
- Watch animated replay of moves
- Review game strategies and decision points

## 🧠 AI Algorithm

The AI opponent uses the **Minimax algorithm** with the following characteristics:

- **Perfect Play**: AI makes optimal moves assuming perfect opponent play
- **Evaluation Function**: 
  - +10 for AI win
  - -10 for player win  
  - 0 for draw
- **Game Tree Search**: Explores all possible game states
- **Performance Optimized**: Efficient pruning and move evaluation

## 📊 Performance Monitoring

The application includes comprehensive performance tracking:

### Metrics Tracked
- **Database Operations**: Query execution times
- **AI Decision Making**: Minimax algorithm performance
- **Login Operations**: Authentication timing
- **Game Duration**: Complete game timing
- **System Resources**: Memory and CPU usage (Windows)

### Performance Display
- Real-time metrics shown after each game
- Average, minimum, and maximum timing data
- Resource usage monitoring
- Performance data export capabilities



## 🔧 Configuration

### Build Configuration
- Default database file: `tictactoe.db`
- Performance monitoring: Enabled by default
- Windows-specific features: Automatically detected



