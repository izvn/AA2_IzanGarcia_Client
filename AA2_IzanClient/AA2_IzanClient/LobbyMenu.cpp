#include "LobbyMenu.h"
#include "Config.h"
#include <iostream>
#include <cstdint>

LobbyMenu::LobbyMenu()
    : titleText(font), createText(font), joinText(font), inputLabel(font),
    displayRoomCode(font), statusText(font), boxActive(false),
    inRoom(false), waitingForPlayers(false), myPlayerID(0)
{
    if (!font.openFromFile("assets/arial.ttf")) {}

    const float TITLE_X = 190.f; const float TITLE_Y = 50.f;
    const unsigned int TITLE_SIZE = 35;
    const float BTN_W = 200.f; const float BTN_H = 60.f;
    const float BTN_CREATE_X = 150.f; const float BTN_JOIN_X = 450.f; const float BTN_Y = 200.f;
    const float LBL_X = 450.f; const float LBL_Y = 280.f;
    const float BOX_Y = 310.f; const float BOX_H = 40.f;
    const unsigned int BTN_TXT_SIZE = 20; const unsigned int LBL_SIZE = 18; const unsigned int CODE_SIZE = 22;
    const float STATUS_X = 200.f; const float STATUS_Y = 400.f; const unsigned int STATUS_SIZE = 20;

    titleText.setString("LOBBY MULTIJUGADOR"); titleText.setCharacterSize(TITLE_SIZE); titleText.setPosition({ TITLE_X, TITLE_Y }); titleText.setFillColor(sf::Color::White);

    createButton.setSize({ BTN_W, BTN_H }); createButton.setPosition({ BTN_CREATE_X, BTN_Y }); createButton.setFillColor(sf::Color(0, 150, 255));
    createText.setString("CREAR SALA"); createText.setCharacterSize(BTN_TXT_SIZE); createText.setPosition({ BTN_CREATE_X + 30.f, BTN_Y + 15.f }); createText.setFillColor(sf::Color::White);

    joinButton.setSize({ BTN_W, BTN_H }); joinButton.setPosition({ BTN_JOIN_X, BTN_Y }); joinButton.setFillColor(sf::Color(255, 120, 0));
    joinText.setString("UNIRSE"); joinText.setCharacterSize(BTN_TXT_SIZE); joinText.setPosition({ BTN_JOIN_X + 60.f, BTN_Y + 15.f }); joinText.setFillColor(sf::Color::White);

    inputLabel.setString("Codigo de Sala:"); inputLabel.setCharacterSize(LBL_SIZE); inputLabel.setPosition({ LBL_X, LBL_Y }); inputLabel.setFillColor(sf::Color::White);
    inputBox.setSize({ BTN_W, BOX_H }); inputBox.setPosition({ LBL_X, BOX_Y }); inputBox.setFillColor(sf::Color::White);
    displayRoomCode.setCharacterSize(CODE_SIZE); displayRoomCode.setPosition({ LBL_X + 10.f, BOX_Y + 5.f }); displayRoomCode.setFillColor(sf::Color::Black);

    statusText.setCharacterSize(STATUS_SIZE); statusText.setPosition({ STATUS_X, STATUS_Y }); statusText.setFillColor(sf::Color::Yellow); statusText.setString("");
}

void LobbyMenu::setPlayerName(const std::string& name) { myName = name; }

void LobbyMenu::reset() {
    roomCode = ""; boxActive = false; inRoom = false; waitingForPlayers = false; myPlayerID = 0;
    displayRoomCode.setString(""); statusText.setString("Listo para una nueva partida."); statusText.setFillColor(sf::Color::Yellow);
    bootstrapSocket.disconnect(); bootstrapSocket.setBlocking(true);
}

void LobbyMenu::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (waitingForPlayers) return;

    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));
        boxActive = inputBox.getGlobalBounds().contains(mousePos);
        bool isCreate = createButton.getGlobalBounds().contains(mousePos);
        bool isJoin = joinButton.getGlobalBounds().contains(mousePos);

        if (isCreate || isJoin) {
            if (roomCode.empty()) {
                statusText.setString("Error: Escribe un codigo primero.");
                statusText.setFillColor(sf::Color::Red);
            }
            else {
                auto resolvedIPs = sf::Dns::resolve(Config::SERVER_IP);
                if (resolvedIPs.has_value() && !resolvedIPs->empty()) {
                    if (bootstrapSocket.connect((*resolvedIPs)[0], Config::BOOTSTRAP_PORT) == sf::Socket::Status::Done) {
                        sf::Packet packet;
                        if (isCreate) packet << Config::NET_CREATE_ROOM << roomCode << myName;
                        else packet << Config::NET_JOIN_ROOM << roomCode << myName;

                        bootstrapSocket.send(packet);
                        sf::Packet respuesta;
                        if (bootstrapSocket.receive(respuesta) == sf::Socket::Status::Done) {
                            const int SERVER_SUCCESS = 1;
                            int estado; respuesta >> estado;
                            if (estado == SERVER_SUCCESS) {
                                if (isCreate) respuesta >> roomCode >> myPlayerID; else respuesta >> myPlayerID;
                                displayRoomCode.setString(roomCode);
                                statusText.setString("Conectado. Esperando a que se unan 4 jugadores...");
                                statusText.setFillColor(sf::Color::Yellow);
                                waitingForPlayers = true; bootstrapSocket.setBlocking(false);
                            }
                            else {
                                statusText.setString(isCreate ? "Error: Esa sala ya existe." : "Error: Sala no existe o esta llena.");
                                statusText.setFillColor(sf::Color::Red);
                            }
                        }
                    }
                }
            }
        }
    }

    if (const auto* textEvent = event.getIf<sf::Event::TextEntered>()) {
        const std::uint32_t KEY_BACKSPACE = 8;
        const std::uint32_t KEY_0 = 48;
        const std::uint32_t KEY_9 = 57;
        const size_t MAX_CODE_LEN = 4;

        if (boxActive) {
            if (textEvent->unicode == KEY_BACKSPACE && !roomCode.empty()) roomCode.pop_back();
            else if (textEvent->unicode >= KEY_0 && textEvent->unicode <= KEY_9 && roomCode.length() < MAX_CODE_LEN) roomCode += static_cast<char>(textEvent->unicode);
            displayRoomCode.setString(roomCode);
        }
    }
}

void LobbyMenu::update(sf::RenderWindow& window) {
    const float THICKNESS_ACTIVE = 3.f;
    const float THICKNESS_INACTIVE = 0.f;
    inputBox.setOutlineThickness(boxActive ? THICKNESS_ACTIVE : THICKNESS_INACTIVE); inputBox.setOutlineColor(sf::Color::Red);

    if (waitingForPlayers) {
        sf::Packet packet;
        if (bootstrapSocket.receive(packet) == sf::Socket::Status::Done) {
            int type;
            if (packet >> type && type == Config::NET_START_P2P) {
                std::uint16_t port;
                packet >> p2pHostIP >> port;

                playerNames.clear(); playerPoints.clear();

                for (int i = 0; i < Config::MAX_PLAYERS; i++) {
                    std::string pName = "Desconocido";
                    std::int32_t pPts = 0;

                    if (packet >> pName >> pPts) {
                        playerNames.push_back(pName);
                        playerPoints.push_back(pPts);
                    }
                }

                statusText.setString("¡Partida Encontrada! Iniciando P2P...");
                statusText.setFillColor(sf::Color::Green);
                waitingForPlayers = false; inRoom = true; bootstrapSocket.disconnect();
            }
        }
    }
}

void LobbyMenu::draw(sf::RenderWindow& window) {
    window.draw(titleText); window.draw(createButton); window.draw(createText);
    window.draw(joinButton); window.draw(joinText); window.draw(inputLabel); window.draw(inputBox); window.draw(displayRoomCode);
    window.draw(statusText);
}

// ==============================================================
// LOS GETTERS QUE ME COMÍ EN EL MENSAJE ANTERIOR ESTÁN AQUÍ
// ==============================================================
bool LobbyMenu::isRoomReady() const { return inRoom; }
std::string LobbyMenu::getRoomCode() const { return roomCode; }
int LobbyMenu::getPlayerID() const { return myPlayerID; }
std::string LobbyMenu::getHostIP() const { return p2pHostIP; }
std::vector<std::string> LobbyMenu::getPlayerNames() const { return playerNames; }