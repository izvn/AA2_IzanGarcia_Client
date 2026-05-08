#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <string>

class LoginMenu {
private:
    sf::Font font;

    sf::RectangleShape userBox;
    sf::RectangleShape passBox;
    sf::RectangleShape loginButton;
    sf::RectangleShape registerButton;

    sf::Text titleText, loginText, registerText, userLabel, passLabel;

    std::string inputUser;
    std::string inputPass;
    sf::Text displayUser;
    sf::Text displayPass;

    int activeBox;

public:
    LoginMenu();
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
};