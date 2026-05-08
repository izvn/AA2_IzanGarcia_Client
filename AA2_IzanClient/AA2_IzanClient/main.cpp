#include <SFML/Graphics.hpp>
#include "LoginMenu.h"
#include <optional>

int main() {
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "3 en Raya Online - Cliente");
    window.setFramerateLimit(60);

    LoginMenu menu;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            menu.handleEvent(*event, window);
        }

        menu.update(window);
        window.clear(sf::Color(30, 30, 30));
        menu.draw(window);
        window.display();
    }
    return 0;
}