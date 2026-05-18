#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>
#include <vector>

// Handles room creation and matchmaking via the Bootstrap Server
class LobbyMenu {
private:
    sf::Font font;
    sf::Text titleText, createText, joinText, inputLabel, displayRoomCode, statusText;
    sf::RectangleShape createButton, joinButton, inputBox;

    std::string roomCode;
    bool boxActive;
    bool inRoom;
    bool waitingForPlayers;
    int myPlayerID;

    sf::TcpSocket bootstrapSocket;
    std::string p2pHostIP;
    std::vector<std::string> playerNames;
    std::vector<int> playerPoints;

public:
    std::string myName;

    // Initializes Lobby UI
    LobbyMenu();

    // Sets the logged-in users name
    void setPlayerName(const std::string& name);

    // Resets lobby state for a new search
    void reset();

    // Processes interaction with room code field and buttons
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);

    // Listens for server readiness signal to start P2P
    void update(sf::RenderWindow& window);

    // Draws UI
    void draw(sf::RenderWindow& window);

    // Getters
    bool isRoomReady() const;
    std::string getRoomCode() const;
    int getPlayerID() const;
    std::string getHostIP() const;
    std::vector<std::string> getPlayerNames() const;
    std::vector<int> getPlayerPoints() const { return playerPoints; }
};