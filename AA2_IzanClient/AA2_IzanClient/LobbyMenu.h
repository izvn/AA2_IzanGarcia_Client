#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <string>

class LobbyMenu {
private:
    sf::Font font;
    sf::Text titleText, createText, joinText, inputLabel, displayRoomCode;
    sf::RectangleShape createButton, joinButton, inputBox;
    std::string roomCode;
    bool boxActive;
    bool inRoom;

public:
    LobbyMenu();
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
    bool isRoomReady() const;
};