#include "LobbyMenu.h"
#include "Config.h"

#include <iostream>
#include <cstdint>

LobbyMenu::LobbyMenu()
    : titleText(font), createText(font), joinText(font), inputLabel(font), displayRoomCode(font), statusText(font),
    boxActive(false), inRoom(false), waitingForPlayers(false), myPlayerID(0), p2pPort(Config::P2P_PORT_BASE) {

    if (!font.openFromFile("assets/arial.ttf")) {
        std::cout << "[ERROR] No se pudo cargar assets/arial.ttf\n";
    }

    // Variables visuales
    const float TITLE_X = 190.f; const float TITLE_Y = 50.f;
    const unsigned int TITLE_SIZE = 35;
    const float BTN_W = 200.f; const float BTN_H = 60.f;
    const float BTN_CREATE_X = 150.f; const float BTN_JOIN_X = 450.f; const float BTN_Y = 200.f;
    const float LBL_X = 450.f; const float LBL_Y = 280.f;
    const float BOX_Y = 310.f; const float BOX_H = 40.f;
    const unsigned int BTN_TXT_SIZE = 20; const unsigned int LBL_SIZE = 18; const unsigned int CODE_SIZE = 22;
    const float STATUS_X = 200.f; const float STATUS_Y = 400.f; const unsigned int STATUS_SIZE = 20;

    titleText.setString("LOBBY MULTIJUGADOR"); titleText.setCharacterSize(TITLE_SIZE);
    titleText.setPosition({ TITLE_X, TITLE_Y }); titleText.setFillColor(sf::Color::White);

    createButton.setSize({ BTN_W, BTN_H }); createButton.setPosition({ BTN_CREATE_X, BTN_Y }); createButton.setFillColor(sf::Color(0, 150, 255));
    createText.setString("CREAR SALA"); createText.setCharacterSize(BTN_TXT_SIZE);
    createText.setPosition({ BTN_CREATE_X + 30.f, BTN_Y + 15.f }); createText.setFillColor(sf::Color::White);

    joinButton.setSize({ BTN_W, BTN_H }); joinButton.setPosition({ BTN_JOIN_X, BTN_Y }); joinButton.setFillColor(sf::Color(255, 120, 0));
    joinText.setString("UNIRSE"); joinText.setCharacterSize(BTN_TXT_SIZE);
    joinText.setPosition({ BTN_JOIN_X + 60.f, BTN_Y + 15.f }); joinText.setFillColor(sf::Color::White);

    inputLabel.setString("Codigo de Sala:"); inputLabel.setCharacterSize(LBL_SIZE);
    inputLabel.setPosition({ LBL_X, LBL_Y }); inputLabel.setFillColor(sf::Color::White);

    inputBox.setSize({ BTN_W, BOX_H }); inputBox.setPosition({ LBL_X, BOX_Y }); inputBox.setFillColor(sf::Color::White);

    displayRoomCode.setCharacterSize(CODE_SIZE); displayRoomCode.setPosition({ LBL_X + 10.f, BOX_Y + 5.f });
    displayRoomCode.setFillColor(sf::Color::Black);

    statusText.setCharacterSize(STATUS_SIZE); statusText.setPosition({ STATUS_X, STATUS_Y });
    statusText.setFillColor(sf::Color::Yellow); statusText.setString("");
}

void LobbyMenu::setPlayerName(const std::string& name) { myName = name; }

// Función vital por si termino una partida y vuelvo al lobby, para no llevarme datos "sucios" de la anterior.
void LobbyMenu::reset() {
    roomCode = ""; matchId = ""; p2pPort = Config::P2P_PORT_BASE;
    boxActive = false; inRoom = false; waitingForPlayers = false; myPlayerID = 0;
    displayRoomCode.setString("");
    statusText.setString("Listo para una nueva partida."); statusText.setFillColor(sf::Color::Yellow);

    bootstrapSocket.disconnect();
    bootstrapSocket.setBlocking(true); // Me aseguro de que vuelve a su estado bloqueante inicial
}

void LobbyMenu::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    // Si ya le he dado a "Crear" o "Unirse" y estoy esperando al server, ignoro todo input (antispam)
    if (waitingForPlayers) { return; }

    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));

        boxActive = inputBox.getGlobalBounds().contains(mousePos);
        bool isCreate = createButton.getGlobalBounds().contains(mousePos);
        bool isJoin = joinButton.getGlobalBounds().contains(mousePos);

        // Intento de conexión al servidor Bootstrap (para crear/unirse)
        if (isCreate || isJoin) {
            if (roomCode.empty()) {
                statusText.setString("Error: Escribe un codigo primero.");
                statusText.setFillColor(sf::Color::Red);
            }
            else {
                auto resolvedIPs = sf::Dns::resolve(Config::SERVER_IP);

                if (resolvedIPs.has_value() && !resolvedIPs->empty()) {
                    // Si conecta con el Bootstrap (Puerto 50000)...
                    if (bootstrapSocket.connect((*resolvedIPs)[0], Config::BOOTSTRAP_PORT) == sf::Socket::Status::Done) {
                        sf::Packet packet;

                        // Empaqueto la acción + el código que he escrito + mi nick
                        if (isCreate) { packet << Config::NET_CREATE_ROOM << roomCode << myName; }
                        else { packet << Config::NET_JOIN_ROOM << roomCode << myName; }

                        bootstrapSocket.send(packet);
                        sf::Packet response;

                        // Recibo la respuesta del servidor (Bloqueante, pero es instantáneo)
                        if (bootstrapSocket.receive(response) == sf::Socket::Status::Done) {
                            int status = Config::SERVER_FAIL;
                            response >> status;

                            if (status == Config::SERVER_SUCCESS) {
                                // Si he creado la sala, el servidor me devuelve el código oficial y mi ID (que será 1, porque soy el Host)
                                if (isCreate) { response >> roomCode >> myPlayerID; }
                                // Si solo me uno, el server solo me asigna mi ID (2, 3 o 4)
                                else { response >> myPlayerID; }

                                displayRoomCode.setString(roomCode);
                                statusText.setString("Conectado. Esperando a que se unan 4 jugadores...");
                                statusText.setFillColor(sf::Color::Yellow);

                                waitingForPlayers = true;

                                // OJO PROFE AQUÍ ESTÁ EL TRUCO: 
                                // Pongo el socket en "No Bloqueante". Esto significa que a partir de ahora, 
                                // si intento leer el socket (receive) y no hay nada, el código no se queda parado esperando, 
                                // sino que pasa de largo. Esto es VITAL para que la pantalla no se quede congelada y el bucle siga pintando a 60 FPS.
                                bootstrapSocket.setBlocking(false);
                            }
                            else {
                                statusText.setString(isCreate ? "Error: Esa sala ya existe." : "Error: Sala no existe o esta llena.");
                                statusText.setFillColor(sf::Color::Red);
                                bootstrapSocket.disconnect();
                            }
                        }
                    }
                    else {
                        statusText.setString("Error: No se pudo conectar al servidor.");
                        statusText.setFillColor(sf::Color::Red);
                    }
                }
            }
        }
    }

    // Teclado para escribir el código de sala (máx 4 números)
    if (const auto* textEvent = event.getIf<sf::Event::TextEntered>()) {
        const std::uint32_t KEY_BACKSPACE = 8;
        const std::uint32_t KEY_0 = 48;
        const std::uint32_t KEY_9 = 57;
        const size_t MAX_CODE_LEN = 4;

        if (boxActive) {
            if (textEvent->unicode == KEY_BACKSPACE && !roomCode.empty()) { roomCode.pop_back(); }
            // Filtro estricto: Solo dejo que se metan caracteres del '0' al '9'
            else if (textEvent->unicode >= KEY_0 && textEvent->unicode <= KEY_9 && roomCode.length() < MAX_CODE_LEN) {
                roomCode += static_cast<char>(textEvent->unicode);
            }
            displayRoomCode.setString(roomCode);
        }
    }
}

// Este update se ejecuta 60 veces por segundo en el main.cpp
void LobbyMenu::update(sf::RenderWindow& window) {
    const float THICKNESS_ACTIVE = 3.f; const float THICKNESS_INACTIVE = 0.f;
    inputBox.setOutlineThickness(boxActive ? THICKNESS_ACTIVE : THICKNESS_INACTIVE);
    inputBox.setOutlineColor(sf::Color::Red);

    // Si estoy esperando a los demás jugadores...
    if (waitingForPlayers) {
        sf::Packet packet;

        // Intento recibir un paquete. Como el socket es "No Bloqueante", esto el 99% de las veces devolverá NotReady y no hará nada.
        // Pero cuando el Servidor Bootstrap nos envíe el paquete mágico (NET_START_P2P)... ¡entrará al if!
        if (bootstrapSocket.receive(packet) == sf::Socket::Status::Done) {
            int type = 0;

            if (packet >> type && type == Config::NET_START_P2P) {
                std::uint16_t receivedPort = Config::P2P_PORT_BASE;

                // Desempaqueto toda la configuración de la partida P2P
                packet >> p2pHostIP >> receivedPort >> matchId;
                p2pPort = static_cast<unsigned short>(receivedPort);

                playerNames.clear();
                playerPoints.clear();

                // Extraigo la info de los 4 jugadores
                for (int i = 0; i < Config::MAX_PLAYERS; i++) {
                    std::string playerName = "Desconocido";
                    std::int32_t playerPointValue = 0;

                    if (packet >> playerName >> playerPointValue) {
                        playerNames.push_back(playerName);
                        playerPoints.push_back(static_cast<int>(playerPointValue));
                    }
                }

                statusText.setString("Partida encontrada. Iniciando P2P...");
                statusText.setFillColor(sf::Color::Green);

                waitingForPlayers = false;
                inRoom = true; // Activo la bandera para que main.cpp haga la transición a GameScreen

                // Me desconecto del Servidor Bootstrap. A partir de ahora es TODO P2P.
                bootstrapSocket.disconnect();

                std::cout << "[LOBBY] Starting match " << matchId << " on port " << p2pPort << "\n";
            }
        }
    }
}

void LobbyMenu::draw(sf::RenderWindow& window) {
    window.draw(titleText);
    window.draw(createButton); window.draw(createText);
    window.draw(joinButton); window.draw(joinText);
    window.draw(inputLabel); window.draw(inputBox); window.draw(displayRoomCode);
    window.draw(statusText);
}

// GETTERS (Paso de variables al main.cpp)

bool LobbyMenu::isRoomReady() const { return inRoom; }
std::string LobbyMenu::getRoomCode() const { return roomCode; }
std::string LobbyMenu::getMatchId() const { return matchId; }
int LobbyMenu::getPlayerID() const { return myPlayerID; }
std::string LobbyMenu::getHostIP() const { return p2pHostIP; }
unsigned short LobbyMenu::getP2PPort() const { return p2pPort; }
std::vector<std::string> LobbyMenu::getPlayerNames() const { return playerNames; }
std::vector<int> LobbyMenu::getPlayerPoints() const { return playerPoints; }