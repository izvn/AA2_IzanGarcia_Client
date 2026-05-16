#include <SFML/Graphics.hpp>
#include "LoginMenu.h"
#include "LobbyMenu.h"
#include "GameScreen.h"
#include "RankingScreen.h"
#include "Config.h"
#include <optional>

enum class AppState { LOGIN, LOBBY, GAME, RANKING };

int main() {
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "3 en Raya Online - P2P Client");
    window.setFramerateLimit(60);

    LoginMenu loginMenu; LobbyMenu lobbyMenu; GameScreen gameScreen; RankingScreen rankingScreen;
    AppState currentState = AppState::LOGIN;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (currentState == AppState::LOGIN) loginMenu.handleEvent(*event, window);
            else if (currentState == AppState::LOBBY) lobbyMenu.handleEvent(*event, window);
            else if (currentState == AppState::GAME) gameScreen.handleEvent(*event, window);
            else if (currentState == AppState::RANKING) rankingScreen.handleEvent(*event, window);
        }

        if (currentState == AppState::LOGIN && loginMenu.isLoginSuccessful()) {
            lobbyMenu.setPlayerName(loginMenu.getInputUser()); currentState = AppState::LOBBY;
        }

        if (currentState == AppState::LOBBY && lobbyMenu.isRoomReady()) {
            gameScreen.setP2PData(
                lobbyMenu.getRoomCode(), lobbyMenu.getPlayerID(), lobbyMenu.getHostIP(),
                lobbyMenu.getPlayerNames(), lobbyMenu.getPlayerPoints()
            );
            currentState = AppState::GAME;
        }

        if (currentState == AppState::GAME && gameScreen.wantsToExit()) {
            gameScreen.resetExit(); rankingScreen.fetchRanking(lobbyMenu.myName); currentState = AppState::RANKING;
        }

        if (currentState == AppState::RANKING && rankingScreen.shouldReturnToLobby()) {
            rankingScreen.reset(); lobbyMenu.reset(); currentState = AppState::LOBBY;
        }

        window.clear(sf::Color(30, 30, 30));

        if (currentState == AppState::LOGIN) { loginMenu.update(window); loginMenu.draw(window); }
        else if (currentState == AppState::LOBBY) { lobbyMenu.update(window); lobbyMenu.draw(window); }
        else if (currentState == AppState::GAME) { gameScreen.update(window); gameScreen.draw(window); }
        else if (currentState == AppState::RANKING) { rankingScreen.update(window); rankingScreen.draw(window); }

        window.display();
    }
    return 0;
}