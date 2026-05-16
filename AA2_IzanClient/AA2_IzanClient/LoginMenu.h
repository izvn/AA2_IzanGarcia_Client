#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>

class LoginMenu {
private:
    sf::Font font;
    sf::Text titleText, userLabel, passLabel, loginText, regText, statusText, displayUser, displayPass;
    sf::RectangleShape userBox, passBox, loginButton, regButton;

    std::string inputUser, inputPass;
    bool userBoxActive, passBoxActive;
    bool loginSuccessful;

public:
    LoginMenu();
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    bool isLoginSuccessful() const { return loginSuccessful; }
    std::string getInputUser() const { return inputUser; }
};