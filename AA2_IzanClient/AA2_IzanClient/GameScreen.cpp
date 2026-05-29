#include "GameScreen.h"
#include <iostream>
#include <cstdint>
#include <algorithm>

GameScreen::GameScreen()
    : titleText(font), timerText(font), roomInfoText(font), markText(font), playerInfoText(font), exitText(font) {

    if (!font.openFromFile("assets/arial.ttf")) {
        std::cout << "[ERROR] Could not load assets/arial.ttf\n";
    }

    const unsigned int TITLE_SIZE = 26; const unsigned int TIMER_SIZE = 24; const unsigned int INFO_SIZE = 20;
    const float POS_MARGIN = 20.f; const float TIMER_X = 320.f; const float INFO_X = 580.f; const float INFO_Y = 100.f;
    const unsigned int MARK_SIZE = 40;
    const float BTN_W = 200.f; const float BTN_H = 50.f; const float BTN_X = 300.f; const float BTN_Y = 520.f;

    titleText.setCharacterSize(TITLE_SIZE); titleText.setPosition({ POS_MARGIN, POS_MARGIN }); titleText.setFillColor(sf::Color::White);
    timerText.setCharacterSize(TIMER_SIZE); timerText.setPosition({ TIMER_X, POS_MARGIN }); timerText.setFillColor(sf::Color::Yellow);
    markText.setCharacterSize(MARK_SIZE); markText.setStyle(sf::Text::Bold);
    playerInfoText.setCharacterSize(INFO_SIZE); playerInfoText.setPosition({ INFO_X, INFO_Y }); playerInfoText.setFillColor(sf::Color::White);

    exitButton.setSize({ BTN_W, BTN_H }); exitButton.setPosition({ BTN_X, BTN_Y }); exitButton.setFillColor(sf::Color(0, 200, 0));
    exitText.setString("VER RANKING"); exitText.setCharacterSize(INFO_SIZE); exitText.setPosition({ BTN_X + 35.f, BTN_Y + 12.f });

    const float START_X = 180.f; const float START_Y = 100.f;
    const float CELL_SIZE = 60.f; const float OUTLINE = 2.f;

    // Configuro tanto la matriz visual (board) como la de datos (grid) a 0.
    for (int i = 0; i < Config::GRID_SIZE; ++i) {
        for (int j = 0; j < Config::GRID_SIZE; ++j) {
            grid[i][j] = 0;
            board[i][j].setSize({ CELL_SIZE - (OUTLINE * 2.f), CELL_SIZE - (OUTLINE * 2.f) });
            board[i][j].setPosition({ START_X + j * CELL_SIZE, START_Y + i * CELL_SIZE });
            board[i][j].setFillColor(sf::Color(45, 45, 45));
            board[i][j].setOutlineThickness(OUTLINE);
            board[i][j].setOutlineColor(sf::Color(80, 80, 80));
        }
    }

    for (int i = 0; i <= Config::MAX_PLAYERS; i++) {
        hasWon[i] = false;
        isDisconnected[i] = false;
    }

    currentTurn = Config::HOST_PLAYER_ID; winnersCount = 0; piecesPlaced = 0;
    gameStarted = false; gameFinished = false; resultReported = false;
    wantToExit = false; connectedToHost = false; hostDisconnected = false;
    myPlayerID = 0; p2pPort = Config::P2P_PORT_BASE;
}

// MÉTODO CRÍTICO: Recibo los datos del main.cpp y decido qué rol asume mi PC.
void GameScreen::setP2PData(const std::string& code, const std::string& newMatchId, int id, const std::string& ip,
    unsigned short port, const std::vector<std::string>& names, const std::vector<int>& points) {

    roomCode = code; matchId = newMatchId; myPlayerID = id;
    hostIP = ip; p2pPort = port; playerNames = names; playerPoints = points;

    titleText.setString("Sala: " + roomCode + " | Jugador: " + std::to_string(myPlayerID));

    std::string info = "JUGADORES:\n\n";
    std::string symbols[] = { "[ O ]", "[ X ]", "[ # ]", "[ S ]" };

    for (int i = 0; i < Config::MAX_PLAYERS; i++) {
        std::string name = i < static_cast<int>(playerNames.size()) ? playerNames[i] : "Desconocido";
        int pointsValue = i < static_cast<int>(playerPoints.size()) ? playerPoints[i] : 0;
        info += symbols[i] + " " + name + "\n        " + std::to_string(pointsValue) + " pts\n\n";
    }
    playerInfoText.setString(info);

    // Reseteo todo (por si estoy echando la segunda partida sin cerrar el programa)
    for (int i = 0; i <= Config::MAX_PLAYERS; i++) { hasWon[i] = false; isDisconnected[i] = false; }
    for (int i = 0; i < Config::GRID_SIZE; ++i) { for (int j = 0; j < Config::GRID_SIZE; ++j) { grid[i][j] = 0; } }

    currentTurn = Config::HOST_PLAYER_ID; winnersCount = 0; piecesPlaced = 0;
    gameStarted = false; gameFinished = false; resultReported = false;
    wantToExit = false; connectedToHost = false; hostDisconnected = false;

    standings.clear(); selector.clear(); closeP2PConnections();

    // ==========================================
    // SEPARACIÓN DE ROLES (HOST vs CLIENTE)
    // ==========================================
    // Si mi ID es 1, el servidor me ha designado como HOST.
    if (myPlayerID == Config::HOST_PLAYER_ID) {
        p2pListener.close();
        // Abro mi propio puerto local y me pongo a escuchar. ¡Me he convertido en un miniserver!
        if (p2pListener.listen(p2pPort) != sf::Socket::Status::Done) {
            std::cout << "[P2P ERROR] Host could not listen on port " << p2pPort << "\n";
        }
        // VITAL: setBlocking(false) para no colgar mi juego mientras los clientes se unen.
        p2pListener.setBlocking(false);
    }
    // Si mi ID es > 1, soy CLIENTE.
    else {
        hostSocket.disconnect();
        connectedToHost = false;
        connectRetryClock.restart(); // Preparo el cronómetro para "bombardear" a peticiones al Host
    }
}

// Destrucción limpia de los Sockets para que no queden hilos zombies en el PC
void GameScreen::closeP2PConnections() {
    for (auto* peer : peers) { if (peer != nullptr) { peer->disconnect(); delete peer; } }
    for (auto* pendingPeer : pendingPeers) { if (pendingPeer != nullptr) { pendingPeer->disconnect(); delete pendingPeer; } }
    peers.clear(); pendingPeers.clear();
    hostSocket.disconnect(); p2pListener.close();
}

// OJO PROFE: En vez de hacer 8 if-elses, uso un comprobador direccional (dx, dy).
// Así con un bucle for miro -2 y +2 casillas hacia cualquier dirección (1,0 horizontal; 0,1 vertical; etc).
bool GameScreen::checkDirection(int player, int x, int y, int dx, int dy) {
    int count = 0;
    for (int i = -2; i <= 2; i++) {
        int nx = x + i * dx;
        int ny = y + i * dy;

        // Compruebo que no me salgo del tablero (nx y ny) y si la ficha es del jugador
        if (nx >= 0 && nx < Config::GRID_SIZE && ny >= 0 && ny < Config::GRID_SIZE && grid[nx][ny] == player) {
            count++;
            if (count == Config::WIN_CONDITION) { return true; } // Ha hecho 3 en raya
        }
        else { count = 0; } // Rompo la racha si encuentro una ficha ajena o vacío
    }
    return false;
}

bool GameScreen::checkWinCondition(int player, int x, int y) {
    // Paso los vectores de dirección (Horizontal, Vertical, Diag Baja, Diag Sube)
    return checkDirection(player, x, y, 1, 0)
        || checkDirection(player, x, y, 0, 1)
        || checkDirection(player, x, y, 1, 1)
        || checkDirection(player, x, y, 1, -1);
}

// Salto de turno inteligente: Omite a los jugadores que ya han ganado o cerrado el juego
void GameScreen::advanceTurn() {
    int attempts = 0;
    do {
        currentTurn++;
        if (currentTurn > Config::MAX_PLAYERS) { currentTurn = Config::HOST_PLAYER_ID; }
        attempts++;
    } while (
        attempts <= Config::MAX_PLAYERS
        && (hasWon[currentTurn] || isDisconnected[currentTurn])
        && winnersCount < Config::MAX_WINNERS
        );
    turnTimer.restart(); // Reseteo el contador de 20s
}

// EL HOST INFORMA A TODOS. Mando el tablero completo por fuerza bruta (es pequeño, 6x6, no afecta al rendimiento de red)
void GameScreen::broadcastState() {
    sf::Packet statePacket;
    // Envío el tipo de paquete (1 = Estado de Partida) + variables críticas
    statePacket << 1 << currentTurn << winnersCount << piecesPlaced;

    for (int i = 1; i <= Config::MAX_PLAYERS; i++) { statePacket << hasWon[i] << isDisconnected[i]; }

    // Serializo la matriz del tablero
    for (int i = 0; i < Config::GRID_SIZE; ++i) { for (int j = 0; j < Config::GRID_SIZE; ++j) { statePacket << grid[i][j]; } }

    statePacket << static_cast<std::uint32_t>(standings.size());
    for (const auto& name : standings) { statePacket << name; }

    // Mando el mismo paquete a mis 3 peers (clientes)
    for (auto* peer : peers) {
        if (peer != nullptr) { peer->send(statePacket); }
    }
}

// LOS CLIENTES LEEN EL ESTADO DEL HOST. Copian todo y actualizan su copia local. 
// Evito desincronizaciones (Desyncs) porque el Host tiene siempre la última palabra.
void GameScreen::readStatePacket(sf::Packet& packet) {
    packet >> currentTurn >> winnersCount >> piecesPlaced;

    for (int i = 1; i <= Config::MAX_PLAYERS; i++) { packet >> hasWon[i] >> isDisconnected[i]; }
    for (int i = 0; i < Config::GRID_SIZE; ++i) { for (int j = 0; j < Config::GRID_SIZE; ++j) { packet >> grid[i][j]; } }

    std::uint32_t standingsSize = 0;
    if (packet >> standingsSize) {
        standings.clear();
        for (std::uint32_t i = 0; i < standingsSize; i++) {
            std::string name; packet >> name; standings.push_back(name);
        }
    }
    turnTimer.restart(); // Sincronizo mi reloj interno con el del Host
}

// Poner una ficha
void GameScreen::applyMove(int player, int x, int y) {
    // Paranoia check: que no ponga la ficha fuera o sobre otra ficha
    if (player < 1 || player > Config::MAX_PLAYERS || x < 0 || x >= Config::GRID_SIZE || y < 0 || y >= Config::GRID_SIZE || grid[x][y] != 0) {
        return;
    }

    grid[x][y] = player;
    piecesPlaced++;

    std::string symbols[] = { "O", "X", "#", "S" };
    std::cout << "[JUEGO] " << playerNames[player - 1] << " (" << symbols[player - 1] << ") coloca en Fila " << x << ", Columna " << y << "\n";

    if (checkWinCondition(player, x, y)) {
        hasWon[player] = true;
        winnersCount++;
        standings.push_back(playerNames[player - 1]); // Lo meto a la lista de ganadores (Rank 1, Rank 2...)
        std::cout << "[JUEGO] " << playerNames[player - 1] << " gana la posicion " << winnersCount << "\n";
    }

    if (winnersCount < Config::MAX_WINNERS) { advanceTurn(); }
}

// Mandar resultados finales al servidor central (Bootstrap)
void GameScreen::sendMatchResult() {
    closeP2PConnections(); // Ya no necesito el P2P
    sf::TcpSocket socket;
    auto resolvedIPs = sf::Dns::resolve(Config::SERVER_IP);

    if (resolvedIPs.has_value() && !resolvedIPs->empty()) {
        if (socket.connect((*resolvedIPs)[0], Config::BOOTSTRAP_PORT) == sf::Socket::Status::Done) {
            sf::Packet packet;
            packet << Config::NET_REPORT_RESULT << matchId;

            // Envío los 4 nombres en el orden en el que quedaron
            for (int i = 0; i < Config::MAX_PLAYERS; i++) {
                if (i < static_cast<int>(standings.size())) { packet << standings[i]; }
                else { packet << "Desconectado"; }
            }

            socket.send(packet);
            socket.disconnect();
            std::cout << "[RANKING] Sent result for match " << matchId << "\n";
        }
        else { std::cout << "[RANKING ERROR] Could not reconnect to bootstrap server.\n"; }
    }
}

void GameScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));

        if (gameFinished && exitButton.getGlobalBounds().contains(mousePos)) { wantToExit = true; return; }

        // Controles de seguridad: No puedes jugar si no es tu turno o si ya has ganado
        if (!gameStarted || gameFinished || hasWon[myPlayerID] || isDisconnected[myPlayerID] || currentTurn != myPlayerID) { return; }

        for (int i = 0; i < Config::GRID_SIZE; ++i) {
            for (int j = 0; j < Config::GRID_SIZE; ++j) {
                if (board[i][j].getGlobalBounds().contains(mousePos) && grid[i][j] == 0) {

                    // ==========================================
                    // DECISIÓN LÓGICA (HOST vs CLIENTE)
                    // ==========================================
                    // OJO PROFE: Si soy el HOST, como mi matriz es LA QUE MANDA, aplico la jugada directo y hago broadcast.
                    if (myPlayerID == Config::HOST_PLAYER_ID) {
                        applyMove(myPlayerID, i, j);
                        broadcastState();
                    }
                    // Si soy CLIENTE, NO LO APLICO en mi matriz local. Simplemente le envío un paquete con la petición al Host (Código 2)
                    // Ya veré la ficha pintada cuando el Host me envíe el Broadcast con el tablero actualizado. (Modelo Autoridad de Servidor).
                    else {
                        sf::Packet movePacket; movePacket << 2 << myPlayerID << i << j;
                        hostSocket.send(movePacket);
                    }
                    return;
                }
            }
        }
    }
}

// ACTUALIZACIÓN DE ESTADO CONSTANTE (Se ejecuta a 60 FPS)
void GameScreen::update(sf::RenderWindow& window) {
    const int REQUIRED_CLIENTS = Config::MAX_PLAYERS - 1; // 3 clientes (el Host ya cuenta como 1)

    // FASE 1: HANDSHAKE (Apretón de manos). HOST ACEPTANDO CLIENTES.
    if (myPlayerID == Config::HOST_PLAYER_ID && !gameStarted) {
        sf::TcpSocket* newPeer = new sf::TcpSocket();

        // Pilla peticiones en no bloqueante. Si no hay nadie, pasa de largo y no congela el update.
        if (p2pListener.accept(*newPeer) == sf::Socket::Status::Done) {
            newPeer->setBlocking(false); pendingPeers.push_back(newPeer);
        }
        else { delete newPeer; } // Lo borro para evitar fugas de memoria

        // Proceso los que acaban de conectarse para esperar el paquete 3 (Saludo con su ID)
        for (auto it = pendingPeers.begin(); it != pendingPeers.end(); ) {
            sf::Packet packet;
            if ((*it)->receive(packet) == sf::Socket::Status::Done) {
                int type = 0; int peerID = 0;
                if (packet >> type >> peerID && type == 3) {
                    selector.add(**it); // AÑADO EL CLIENTE AL MULTIPLEXOR (SocketSelector)
                    peers.push_back(*it);
                    it = pendingPeers.erase(it);
                    std::cout << "[P2P] Peer " << peerID << " connected.\n";

                    // Si ya tengo a los 3, ¡Arranco la partida!
                    if (peers.size() == REQUIRED_CLIENTS) {
                        gameStarted = true; turnTimer.restart(); broadcastState();
                        std::cout << "[P2P] Match started.\n";
                    }
                    continue;
                }
            }
            ++it;
        }
    }

    // FASE 1: HANDSHAKE. CLIENTE INTENTANDO CONECTARSE AL HOST.
    if (myPlayerID != Config::HOST_PLAYER_ID && !connectedToHost && !hostDisconnected) {
        // Uso un Timeout de 1 segundo. ¿Por qué? Porque el host y yo cambiamos de pantalla a la vez. 
        // Si mi PC es más rápido que el suyo, intentaré conectarme a un puerto que aún no ha abierto. 
        // Así me aseguro de reintentar conectarme de forma espaciada sin bloquear SFML.
        if (connectRetryClock.getElapsedTime().asSeconds() > 1.0f) {
            connectRetryClock.restart();
            auto resolvedIPs = sf::Dns::resolve(hostIP);

            if (resolvedIPs.has_value() && !resolvedIPs->empty()) {
                hostSocket.setBlocking(true); // Bloqueo momentáneamente para el Timeout de conexión
                if (hostSocket.connect((*resolvedIPs)[0], p2pPort, sf::milliseconds(1000)) == sf::Socket::Status::Done) {
                    hostSocket.setBlocking(false); // Vuelvo a estado fluido
                    connectedToHost = true;
                    // Mando el Tipo 3 (Saludo)
                    sf::Packet hello; hello << 3 << myPlayerID;
                    hostSocket.send(hello);
                    std::cout << "[P2P] Connected to host.\n";
                }
            }
        }
    }

    // Dibujo textos de espera si no hemos empezado...
    if (!gameStarted) {
        if (myPlayerID == Config::HOST_PLAYER_ID) {
            timerText.setString("HOST: Waiting peers... (" + std::to_string(peers.size()) + "/" + std::to_string(REQUIRED_CLIENTS) + ")");
        }
        else {
            timerText.setString(connectedToHost ? "Connected. Waiting start..." : "Calling Host...");
            if (connectedToHost) {
                sf::Packet packet;
                auto status = hostSocket.receive(packet);
                if (status == sf::Socket::Status::Done) {
                    int type = 0;
                    if (packet >> type && type == 1) { readStatePacket(packet); gameStarted = true; } // Llegó el inicio
                }
                else if (status == sf::Socket::Status::Disconnected) {
                    hostDisconnected = true; gameFinished = true;
                    timerText.setString("HOST DESCONECTADO"); timerText.setFillColor(sf::Color::Red);
                }
            }
        }
        return;
    }

    // FASE 2: EN PLENA PARTIDA
    // HOST: Timer (Control de tiempo de turno)
    if (myPlayerID == Config::HOST_PLAYER_ID && winnersCount < Config::MAX_WINNERS && !gameFinished) {
        if (turnTimer.getElapsedTime().asSeconds() >= Config::TURN_TIME_LIMIT_SEC) {
            std::cout << "[JUEGO] Turno omitido por tiempo del jugador " << currentTurn << "\n";
            advanceTurn(); broadcastState(); // Salto turno y se lo comunico a todos
        }
    }

    // HOST ESCUCHANDO JUGADAS (MULTIPLEXADO CON EL SELECTOR)
    // Ojo profe: El selector se queda esperando (wait) un máximo de 10ms. Así leo los paquetes de la red
    // pero no me quedo colgado de forma infinita, permitiendo que la ventana de juego se repinte.
    if (myPlayerID == Config::HOST_PLAYER_ID && selector.wait(sf::milliseconds(10))) {
        for (size_t i = 0; i < peers.size(); i++) {
            if (selector.isReady(*peers[i])) { // Si este peer específico ha enviado algo
                sf::Packet packet;
                auto status = peers[i]->receive(packet);

                // Si se ha desconectado o cerrado el juego...
                if (status == sf::Socket::Status::Disconnected) {
                    int disconnectedPlayerID = static_cast<int>(i) + 2; // (Índice + 2 porque J1 es Host)
                    if (disconnectedPlayerID >= 1 && disconnectedPlayerID <= Config::MAX_PLAYERS && !isDisconnected[disconnectedPlayerID]) {
                        isDisconnected[disconnectedPlayerID] = true;
                        hasWon[disconnectedPlayerID] = true; // Truco de diseño: Lo marco como ganador virtual para que el turno se salte
                        if (currentTurn == disconnectedPlayerID) { advanceTurn(); }
                        broadcastState(); // Informo de su "muerte"
                    }
                }
                // Si he recibido un movimiento del cliente (Código 2)
                else if (status == sf::Socket::Status::Done) {
                    int type = 0, playerID = 0, x = 0, y = 0;
                    if (packet >> type && type == 2) {
                        packet >> playerID >> x >> y;
                        // Valido si realmente le tocaba a él y aplico (Autoridad del Servidor)
                        if (playerID == currentTurn && !hasWon[playerID] && !isDisconnected[playerID]) {
                            applyMove(playerID, x, y);
                            broadcastState();
                        }
                    }
                }
            }
        }
    }

    // CLIENTE ESCUCHANDO AL HOST (Sencillo, solo leo el Broadcast, Código 1)
    if (myPlayerID != Config::HOST_PLAYER_ID && !gameFinished) {
        sf::Packet packet;
        auto status = hostSocket.receive(packet);

        if (status == sf::Socket::Status::Done) {
            int type = 0;
            if (packet >> type && type == 1) { readStatePacket(packet); }
        }
        else if (status == sf::Socket::Status::Disconnected) {
            hostDisconnected = true; gameFinished = true; // Error fatal si cae el host
            timerText.setString("HOST DESCONECTADO"); timerText.setFillColor(sf::Color::Red);
        }
    }

    // FASE 3: COMPROBAR FIN DE PARTIDA
    int disconnectedCount = 0;
    for (int i = 1; i <= Config::MAX_PLAYERS; i++) { if (isDisconnected[i]) { disconnectedCount++; } }

    const int MAX_PIECES = Config::GRID_SIZE * Config::GRID_SIZE;

    // Si ya tenemos 3 ganadores (+1 perdedor), tablero lleno o error de host... Se acabó la partida.
    if ((winnersCount >= Config::MAX_WINNERS || (winnersCount + disconnectedCount >= Config::MAX_WINNERS) || piecesPlaced >= MAX_PIECES || hostDisconnected) && !gameFinished) {

        if (piecesPlaced >= MAX_PIECES && winnersCount < Config::MAX_WINNERS) { std::cout << "[JUEGO] Empate: Tablero lleno.\n"; }

        // Meto en el array final "standings" a los perdedores o los del empate
        for (int i = 1; i <= Config::MAX_PLAYERS; i++) {
            bool alreadyInStandings = false;
            for (const auto& name : standings) {
                if (name == playerNames[i - 1]) { alreadyInStandings = true; break; }
            }
            if (!alreadyInStandings && !isDisconnected[i]) { standings.push_back(playerNames[i - 1]); }
        }

        // Meto a los desconectados los últimos para que coman el loss de puntos
        for (int i = 1; i <= Config::MAX_PLAYERS; i++) {
            if (isDisconnected[i]) { standings.push_back(playerNames[i - 1]); }
        }
        while (standings.size() < Config::MAX_PLAYERS) { standings.push_back("Desconectado"); }

        gameFinished = true;
    }

    // FASE 4: REPORTAR A BASE DE DATOS
    if (gameFinished && !resultReported) {
        if (!hostDisconnected) { sendMatchResult(); }
        resultReported = true;
        timerText.setString(hostDisconnected ? "HOST DESCONECTADO" : "PARTIDA FINALIZADA");
        timerText.setFillColor(hostDisconnected ? sf::Color::Red : sf::Color::Green);
    }
    // SI SIGUE ACTIVA: Actualizar UI de turnos
    else if (!gameFinished) {
        if (hasWon[myPlayerID]) {
            timerText.setString(isDisconnected[myPlayerID] ? "DESCONECTADO" : "HAS GANADO! Modo espectador.");
            timerText.setFillColor(sf::Color::Cyan);
        }
        else {
            int timeRemaining = Config::TURN_TIME_LIMIT_SEC - static_cast<int>(turnTimer.getElapsedTime().asSeconds());
            if (timeRemaining < 0) { timeRemaining = 0; }

            if (currentTurn == myPlayerID) {
                timerText.setString("TU TURNO! Quedan: " + std::to_string(timeRemaining) + "s");
                timerText.setFillColor(sf::Color::Green);
            }
            else {
                timerText.setString("Turno Jugador " + std::to_string(currentTurn));
                timerText.setFillColor(sf::Color::Yellow);
            }
        }
    }
}

void GameScreen::draw(sf::RenderWindow& window) {
    window.draw(titleText); window.draw(timerText); window.draw(playerInfoText);

    const float START_X = 180.f; const float START_Y = 100.f; const float CELL_SIZE = 60.f;

    // Dibujo el tablero y las fichas encima iterando la matriz
    for (int i = 0; i < Config::GRID_SIZE; ++i) {
        for (int j = 0; j < Config::GRID_SIZE; ++j) {
            window.draw(board[i][j]);

            if (grid[i][j] != 0) {
                markText.setPosition({ START_X + j * CELL_SIZE + 15.f, START_Y + i * CELL_SIZE + 5.f });

                if (grid[i][j] == 1) { markText.setString("O"); markText.setFillColor(sf::Color::Red); }
                else if (grid[i][j] == 2) { markText.setString("X"); markText.setFillColor(sf::Color::Green); }
                else if (grid[i][j] == 3) { markText.setString("#"); markText.setFillColor(sf::Color::Blue); }
                else if (grid[i][j] == 4) { markText.setString("S"); markText.setFillColor(sf::Color::Magenta); }

                window.draw(markText);
            }
        }
    }

    if (gameFinished) { window.draw(exitButton); window.draw(exitText); }
}

bool GameScreen::wantsToExit() const { return wantToExit; }
void GameScreen::resetExit() { wantToExit = false; }