#include <SFML/Graphics.hpp>

#include "LoginMenu.h"
#include "LobbyMenu.h"
#include "GameScreen.h"
#include "RankingScreen.h"
#include "Config.h"

#include <optional>
#include <iostream>

// CLAVE DE LA ARQUITECTURA: Máquina de Estados (State Machine)
// Me permite controlar en qué pantalla está el usuario y solo procesar/dibujar lo que toca.
enum class AppState {
    LOGIN,
    LOBBY,
    GAME,
    RANKING
};

int main() {
    // Inicializo la ventana de SFML. Le pongo un límite de 60 FPS para no freír la CPU 
    // (si no lo pongo, el bucle va a miles de FPS y satura el procesador).
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "3 en Raya Online - P2P Client");
    window.setFramerateLimit(60);

    // Instancio todas mis pantallas
    LoginMenu loginMenu;
    LobbyMenu lobbyMenu;
    GameScreen gameScreen;
    RankingScreen rankingScreen;

    // Estado inicial de la app
    AppState currentState = AppState::LOGIN;

    // Bucle principal del juego (Game Loop)
    while (window.isOpen()) {

        // 1. GESTIÓN DE EVENTOS (Input del usuario)
        // Uso std::optional porque es la forma moderna en SFML 3 de capturar eventos.
        while (const std::optional event = window.pollEvent()) {
            // Si le doy a la 'X' de la ventana, cierro de forma limpia
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // Derivo el evento solo a la pantalla que está activa
            if (currentState == AppState::LOGIN) {
                loginMenu.handleEvent(*event, window);
            }
            else if (currentState == AppState::LOBBY) {
                lobbyMenu.handleEvent(*event, window);
            }
            else if (currentState == AppState::GAME) {
                gameScreen.handleEvent(*event, window);
            }
            else if (currentState == AppState::RANKING) {
                rankingScreen.handleEvent(*event, window);
            }
        }

        // 2. LÓGICA DE TRANSICIONES DE ESTADO
        // Reviso las "banderas" (flags) de cada pantalla para saber si tengo que cambiar de menú.

        // Si estoy en Login y el método isLoginSuccessful devuelve true -> Paso al Lobby
        if (currentState == AppState::LOGIN && loginMenu.isLoginSuccessful()) {
            lobbyMenu.setPlayerName(loginMenu.getInputUser()); // Paso mi nombre al lobby
            currentState = AppState::LOBBY;
        }

        // Si estoy en el Lobby y la sala está llena (el server me avisó) -> Arranco el P2P
        if (currentState == AppState::LOBBY && lobbyMenu.isRoomReady()) {
            // Le inyecto TODA la info de red a la pantalla de juego para que sepa a quién conectarse
            gameScreen.setP2PData(
                lobbyMenu.getRoomCode(),
                lobbyMenu.getMatchId(),
                lobbyMenu.getPlayerID(),
                lobbyMenu.getHostIP(),
                lobbyMenu.getP2PPort(),
                lobbyMenu.getPlayerNames(),
                lobbyMenu.getPlayerPoints()
            );
            currentState = AppState::GAME;
        }

        // Si estoy en la partida, ya terminó, y he clicado en "Ver Ranking"
        if (currentState == AppState::GAME && gameScreen.wantsToExit()) {
            gameScreen.resetExit();
            // Antes de mostrar la pantalla, hago la petición a la BD para tener los datos frescos
            rankingScreen.fetchRanking(lobbyMenu.myName);
            currentState = AppState::RANKING;
        }

        // Si estoy en el Ranking y le doy al botón de volver -> Reseteo el Lobby
        if (currentState == AppState::RANKING && rankingScreen.shouldReturnToLobby()) {
            rankingScreen.reset();
            lobbyMenu.reset();
            currentState = AppState::LOBBY;
        }

        // 3. RENDERIZADO (Pintar en pantalla)
        // Limpio el frame anterior con un gris oscuro de fondo
        window.clear(sf::Color(30, 30, 30));

        // Actualizo lógica interna (update) y dibujo los elementos (draw) de la pantalla activa
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
        else if (currentState == AppState::RANKING) {
            rankingScreen.update(window);
            rankingScreen.draw(window);
        }

        // Muestro el frame ya pintado
        window.display();
    }

    return 0;
}