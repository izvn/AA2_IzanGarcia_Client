#include <SFML/Graphics.hpp>
#include "LoginMenu.h"
#include "LobbyMenu.h"
#include "GameScreen.h"
#include <optional>

enum class AppState { LOGIN, LOBBY, GAME };

int main() {
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "3 en Raya Online - Cliente");
    window.setFramerateLimit(60);

    LoginMenu loginMenu;
    LobbyMenu lobbyMenu;
    GameScreen gameScreen;
    AppState currentState = AppState::LOGIN;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (currentState == AppState::LOGIN) {
                loginMenu.handleEvent(*event, window);
                if (loginMenu.isLoginSuccessful()) {
                    currentState = AppState::LOBBY;
                }
            }
            else if (currentState == AppState::LOBBY) {
                lobbyMenu.handleEvent(*event, window);
                if (lobbyMenu.isRoomReady()) {
                    gameScreen.setRoomData(lobbyMenu.getRoomCode(), lobbyMenu.getPlayerID());
                    currentState = AppState::GAME;
                }
            }
            else if (currentState == AppState::GAME) {
                gameScreen.handleEvent(*event, window);
            }
        }

        window.clear(sf::Color(30, 30, 30));

        if (currentState == AppState::LOGIN) {
            loginMenu.update(window);
            loginMenu.draw(window);
        }
        else if (currentState == AppState::LOBBY) {
            lobbyMenu.update(window);
            lobbyMenu.draw(window);
        }
        else if (currentState == AppState::GAME) {
            gameScreen.update(window);
            gameScreen.draw(window);
        }

        window.display();
    }
    return 0;
}