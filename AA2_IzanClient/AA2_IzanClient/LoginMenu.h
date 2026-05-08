#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <string>

class LoginMenu {
private:
    sf::Font font;
    sf::RectangleShape userBox, passBox, loginButton, registerButton;
    sf::Text titleText, loginText, registerText, userLabel, passLabel;
    std::string inputUser, inputPass;
    sf::Text displayUser, displayPass;
    int activeBox;
    bool loginSuccess;

public:
    LoginMenu();
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
    bool isLoginSuccessful() const;
};