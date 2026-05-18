#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>

// Handles the initial screen for user authentication and registration
class LoginMenu {
private:
    sf::Font font;
    sf::Text titleText, userLabel, passLabel, loginText, regText, statusText, displayUser, displayPass;
    sf::RectangleShape userBox, passBox, loginButton, regButton;

    std::string inputUser, inputPass;
    bool userBoxActive, passBoxActive;
    bool loginSuccessful;

public:
    // Initializes UI elements and variables
    LoginMenu();

    // Processes mouse clicks and keyboard input
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);

    // Updates UI logic
    void update(sf::RenderWindow& window);

    // Renders elements to the screen
    void draw(sf::RenderWindow& window);

    // Getters for the state machine
    bool isLoginSuccessful() const { return loginSuccessful; }
    std::string getInputUser() const { return inputUser; }
};