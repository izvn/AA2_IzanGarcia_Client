#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>
#include <vector>
#include "Config.h"

class GameScreen {
private:
    sf::Font font;
    sf::Text titleText, timerText, roomInfoText, markText, playerInfoText;
    sf::RectangleShape board[Config::GRID_SIZE][Config::GRID_SIZE];
    sf::RectangleShape exitButton; sf::Text exitText;

    int grid[Config::GRID_SIZE][Config::GRID_SIZE];
    int currentTurn; bool hasWon[5]; int winnersCount; bool gameStarted;
    int piecesPlaced; // NUEVO: Contador para el empate
    sf::Clock turnTimer;

    std::string roomCode; int myPlayerID; std::string hostIP;
    std::vector<std::string> playerNames;
    std::vector<int> playerPoints;
    bool isDisconnected[5];

    sf::TcpListener p2pListener;
    std::vector<sf::TcpSocket*> peers;
    std::vector<sf::TcpSocket*> pendingPeers;
    sf::TcpSocket hostSocket;
    sf::SocketSelector selector;

    bool connectedToHost; sf::Clock connectRetryClock;
    std::vector<std::string> standings;
    bool gameFinished; bool resultReported; bool wantToExit;

    bool checkWinCondition(int player, int x, int y);
    bool checkDirection(int p, int x, int y, int dx, int dy);
    void advanceTurn(); void broadcastState(); void applyMove(int player, int x, int y);
    void sendMatchResult();

public:
    GameScreen();
    void setP2PData(const std::string& code, int id, const std::string& ip, const std::vector<std::string>& names, const std::vector<int>& points);
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    bool wantsToExit() const { return wantToExit; }
    void resetExit() { wantToExit = false; }
};