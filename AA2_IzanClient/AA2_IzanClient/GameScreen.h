#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include <string>
#include <vector>

#include "Config.h"

class GameScreen {
private:
    sf::Font font;

    sf::Text titleText;
    sf::Text timerText;
    sf::Text roomInfoText;
    sf::Text markText;
    sf::Text playerInfoText;
    sf::Text exitText;

    sf::RectangleShape board[Config::GRID_SIZE][Config::GRID_SIZE];
    sf::RectangleShape exitButton;

    int grid[Config::GRID_SIZE][Config::GRID_SIZE];

    int currentTurn;
    bool hasWon[Config::MAX_PLAYERS + 1];
    bool isDisconnected[Config::MAX_PLAYERS + 1];

    int winnersCount;
    int piecesPlaced;

    bool gameStarted;
    bool gameFinished;
    bool resultReported;
    bool wantToExit;
    bool connectedToHost;
    bool hostDisconnected;

    sf::Clock turnTimer;
    sf::Clock connectRetryClock;

    std::string roomCode;
    std::string matchId;
    int myPlayerID;
    std::string hostIP;
    unsigned short p2pPort;

    std::vector<std::string> playerNames;
    std::vector<int> playerPoints;
    std::vector<std::string> standings;

    sf::TcpListener p2pListener;
    std::vector<sf::TcpSocket*> peers;
    std::vector<sf::TcpSocket*> pendingPeers;
    sf::TcpSocket hostSocket;
    sf::SocketSelector selector;

    bool checkWinCondition(int player, int x, int y);
    bool checkDirection(int player, int x, int y, int dx, int dy);

    void advanceTurn();
    void broadcastState();
    void applyMove(int player, int x, int y);
    void sendMatchResult();
    void closeP2PConnections();
    void readStatePacket(sf::Packet& packet);

public:
    GameScreen();

    void setP2PData(
        const std::string& code,
        const std::string& newMatchId,
        int id,
        const std::string& ip,
        unsigned short port,
        const std::vector<std::string>& names,
        const std::vector<int>& points
    );

    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    bool wantsToExit() const;
    void resetExit();
};
