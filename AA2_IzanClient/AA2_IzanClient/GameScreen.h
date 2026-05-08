#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

class GameScreen {
private:
    sf::Font font;
    sf::Text titleText;
    sf::Text markText;
    sf::RectangleShape board[3][3];
    int grid[3][3];

public:
    GameScreen();
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
};