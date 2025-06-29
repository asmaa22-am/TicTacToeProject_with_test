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
- Qt 5.12+ or Qt 6.x
- C++17 compatible compiler
- SQLite support (included with Qt)
- CMake or qmake build system

### Dependencies
- Qt Core
- Qt Widgets
- Qt SQL
- Qt GUI

### Installation

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd tictactoe-game
   ```

2. **Build with CMake**
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

   **Or build with qmake**
   ```bash
   qmake TicTacToe.pro
   make
   ```

3. **Run the application**
   ```bash
   ./TicTacToe
   ```

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

## 🗄️ Database Schema

### Users Table
```sql
CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    salt TEXT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

### Game History Table
```sql
CREATE TABLE game_history (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL,
    game_mode TEXT NOT NULL,
    winner TEXT NOT NULL,
    moves TEXT NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(username) REFERENCES users(username)
);
```

## 🔧 Configuration

### Build Configuration
- Default database file: `tictactoe.db`
- Performance monitoring: Enabled by default
- Windows-specific features: Automatically detected

### Customization Options
- Modify AI difficulty by adjusting Minimax depth
- Customize UI colors and styling
- Configure performance monitoring intervals

## 🐛 Troubleshooting

### Common Issues

**Database Connection Problems**
- Ensure write permissions in application directory
- Check SQLite driver availability
- Verify database file integrity

**Performance Monitoring (Windows)**
- Some features require Windows-specific APIs
- Performance data may be limited on other platforms

**Build Issues**
- Verify Qt installation and version compatibility
- Ensure all required Qt modules are installed
- Check compiler C++17 support

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📝 License

This project is licensed under the MIT License - see the `LICENSE` file for details.

## 🙏 Acknowledgments

- Qt Framework for the excellent GUI toolkit
- SQLite for lightweight database functionality
- Minimax algorithm for AI game theory implementation

## 📞 Support

For support, feature requests, or bug reports, please open an issue on the project repository.

---

**Version**: 1.0.0  
**Last Updated**: 2025  
**Platform**: Cross-platform (Windows, macOS, Linux)  
**Language**: C++17 with Qt Framework
