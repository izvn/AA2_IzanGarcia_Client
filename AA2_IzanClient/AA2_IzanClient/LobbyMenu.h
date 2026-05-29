#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>
#include <vector>

// Clase para crear salas o unirse a ellas y esperar a que se llenen (Matchmaking)
class LobbyMenu {
private:
    sf::Font font;
    sf::Text titleText, createText, joinText, inputLabel, displayRoomCode, statusText;
    sf::RectangleShape createButton, joinButton, inputBox;

    std::string roomCode;
    std::string matchId;
    bool boxActive;

    // inRoom dispara el cambio a la pantalla de juego
    bool inRoom;

    // waitingForPlayers bloquea los clicks mientras esperamos que se una gente
    bool waitingForPlayers;
    int myPlayerID; // Mi ID asignado por el servidor (1, 2, 3 o 4)

    // Socket persistente: A diferencia del Login, aquí necesito mantener el socket 
    // abierto mientras espero a los demás en el lobby.
    sf::TcpSocket bootstrapSocket;

    // Datos que el servidor me mandará cuando la partida P2P vaya a empezar
    std::string p2pHostIP;
    unsigned short p2pPort;
    std::vector<std::string> playerNames;
    std::vector<int> playerPoints;

public:
    std::string myName;
    LobbyMenu();

    void setPlayerName(const std::string& name);
    void reset();

    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    // Getters masivos para inyectar toda esta info en el GameScreen (paso de variables)
    bool isRoomReady() const;
    std::string getRoomCode() const;
    std::string getMatchId() const;
    int getPlayerID() const;
    std::string getHostIP() const;
    unsigned short getP2PPort() const;
    std::vector<std::string> getPlayerNames() const;
    std::vector<int> getPlayerPoints() const;
};