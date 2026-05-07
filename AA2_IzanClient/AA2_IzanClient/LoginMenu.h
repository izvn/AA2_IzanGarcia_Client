#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>

class LoginMenu {
private:
    sf::Font font;

    sf::RectangleShape userBox;
    sf::RectangleShape passBox;

    sf::RectangleShape loginButton;
    sf::RectangleShape registerButton;

    sf::Text titleText;
    sf::Text loginText;
    sf::Text registerText;
    sf::Text userLabel;
    sf::Text passLabel;

public:
    LoginMenu();
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
};