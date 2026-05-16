#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>
#include <vector>

class LobbyMenu {
private:
    sf::Font font;
    sf::Text titleText, createText, joinText, inputLabel, displayRoomCode, statusText;
    sf::RectangleShape createButton, joinButton, inputBox;

    std::string roomCode; bool boxActive; bool inRoom; bool waitingForPlayers; int myPlayerID;
    sf::TcpSocket bootstrapSocket;
    std::string p2pHostIP;
    std::vector<std::string> playerNames;
    std::vector<int> playerPoints;

public:
    LobbyMenu();
    void setPlayerName(const std::string& name);
    void reset();
    std::string myName;

    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    bool isRoomReady() const;
    std::string getRoomCode() const;
    int getPlayerID() const;
    std::string getHostIP() const;
    std::vector<std::string> getPlayerNames() const;
    std::vector<int> getPlayerPoints() const { return playerPoints; }
};