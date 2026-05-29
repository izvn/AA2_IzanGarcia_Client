#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>

// Clase que gestiona la pantalla de autenticación y registro
class LoginMenu {
private:
    sf::Font font;
    sf::Text titleText, userLabel, passLabel, loginText, regText, statusText, displayUser, displayPass;
    sf::RectangleShape userBox, passBox, loginButton, regButton;

    // Strings para guardar lo que teclea el usuario
    std::string inputUser, inputPass;

    // Banderas para saber en qué caja estoy escribiendo
    bool userBoxActive, passBoxActive;

    // Bandera para decirle al main.cpp que ya me puede sacar de esta pantalla
    bool loginSuccessful;

public:
    LoginMenu();

    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    // Getters para que el main.cpp pueda leer mi estado
    bool isLoginSuccessful() const { return loginSuccessful; }
    std::string getInputUser() const { return inputUser; }
};