#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>

class GameScreen {
private:
    sf::Font font;
    sf::Text titleText;
    sf::Text markText;
    sf::Text roomInfoText;
    sf::RectangleShape board[3][3];
    int grid[3][3];

    std::string roomCode;
    int myPlayerID;
    int currentTurn;
    int winner;
    sf::Clock pollClock;

public:
    GameScreen();
    void setRoomData(const std::string& code, int id);
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
};