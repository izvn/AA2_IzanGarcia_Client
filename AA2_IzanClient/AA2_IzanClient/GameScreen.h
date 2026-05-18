#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>
#include <vector>
#include "Config.h"

// Core gameplay loop, renders the 6x6 board and handles P2P connections
class GameScreen {
private:
    sf::Font font;
    sf::Text titleText, timerText, roomInfoText, markText, playerInfoText;
    sf::RectangleShape board[Config::GRID_SIZE][Config::GRID_SIZE];
    sf::RectangleShape exitButton; sf::Text exitText;

    int grid[Config::GRID_SIZE][Config::GRID_SIZE];
    int currentTurn;
    bool hasWon[5];
    int winnersCount;
    bool gameStarted;
    int piecesPlaced;
    sf::Clock turnTimer;

    std::string roomCode; int myPlayerID; std::string hostIP;
    std::vector<std::string> playerNames;
    std::vector<int> playerPoints;
    bool isDisconnected[5];

    // Networking P2P variables
    sf::TcpListener p2pListener;
    std::vector<sf::TcpSocket*> peers;
    std::vector<sf::TcpSocket*> pendingPeers;
    sf::TcpSocket hostSocket;
    sf::SocketSelector selector;

    bool connectedToHost;
    sf::Clock connectRetryClock;
    std::vector<std::string> standings;
    bool gameFinished;
    bool resultReported;
    bool wantToExit;

    // Evaluates if the current player has won
    bool checkWinCondition(int player, int x, int y);

    // Checks consecutive marks in a given mathematical direction (dx, dy)
    bool checkDirection(int p, int x, int y, int dx, int dy);

    // Advances turn skipping spectators/disconnected players
    void advanceTurn();

    // Host function to synchronize board state to all peers
    void broadcastState();

    // Registers a move on the board array
    void applyMove(int player, int x, int y);

    // Submits final results back to Bootstrap Server
    void sendMatchResult();

public:
    GameScreen();

    // Initializes session data from the Lobby
    void setP2PData(const std::string& code, int id, const std::string& ip, const std::vector<std::string>& names, const std::vector<int>& points);

    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    bool wantsToExit() const { return wantToExit; }
    void resetExit() { wantToExit = false; }
};