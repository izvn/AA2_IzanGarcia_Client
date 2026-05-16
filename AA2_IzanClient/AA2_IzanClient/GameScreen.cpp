#include "GameScreen.h"
#include <iostream>
#include <cstdint>

GameScreen::GameScreen() : titleText(font), timerText(font), roomInfoText(font), markText(font), playerInfoText(font), exitText(font) {
    if (!font.openFromFile("assets/arial.ttf")) {}

    const unsigned int TITLE_SIZE = 26; const unsigned int TIMER_SIZE = 24; const unsigned int INFO_SIZE = 20;
    const float POS_MARGIN = 20.f; const float TIMER_X = 320.f;
    const float INFO_X = 580.f; const float INFO_Y = 100.f;
    const unsigned int MARK_SIZE = 40;
    const float BTN_W = 200.f; const float BTN_H = 50.f; const float BTN_X = 300.f; const float BTN_Y = 520.f;

    titleText.setCharacterSize(TITLE_SIZE); titleText.setPosition({ POS_MARGIN, POS_MARGIN }); titleText.setFillColor(sf::Color::White);
    timerText.setCharacterSize(TIMER_SIZE); timerText.setPosition({ TIMER_X, POS_MARGIN }); timerText.setFillColor(sf::Color::Yellow);
    markText.setCharacterSize(MARK_SIZE); markText.setStyle(sf::Text::Bold);

    playerInfoText.setCharacterSize(INFO_SIZE); playerInfoText.setPosition({ INFO_X, INFO_Y }); playerInfoText.setFillColor(sf::Color::White);

    exitButton.setSize({ BTN_W, BTN_H }); exitButton.setPosition({ BTN_X, BTN_Y }); exitButton.setFillColor(sf::Color(0, 200, 0));
    exitText.setString("VER RANKING"); exitText.setCharacterSize(INFO_SIZE); exitText.setPosition({ BTN_X + 35.f, BTN_Y + 12.f });

    const float START_X = 180.f; const float START_Y = 100.f; const float CELL_SIZE = 60.f; const float OUTLINE = 2.f;

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
    for (int i = 0; i < 5; i++) { hasWon[i] = false; isDisconnected[i] = false; }
    currentTurn = 1; winnersCount = 0; piecesPlaced = 0; gameStarted = false; gameFinished = false; resultReported = false; wantToExit = false; connectedToHost = false;
}

void GameScreen::setP2PData(const std::string& code, int id, const std::string& ip, const std::vector<std::string>& names, const std::vector<int>& points) {
    roomCode = code; myPlayerID = id; hostIP = ip; playerNames = names; playerPoints = points;
    titleText.setString("Sala: " + roomCode + " | Jugador: " + std::to_string(myPlayerID));

    std::string info = "JUGADORES:\n\n";
    std::string symbols[] = { "[ O ]", "[ X ]", "[ # ]", "[ S ]" };
    for (int i = 0; i < 4; i++) {
        info += symbols[i] + " " + playerNames[i] + "\n        " + std::to_string(playerPoints[i]) + " pts\n\n";
    }
    playerInfoText.setString(info);

    for (int i = 0; i < 5; i++) { hasWon[i] = false; isDisconnected[i] = false; }
    for (int i = 0; i < Config::GRID_SIZE; ++i) for (int j = 0; j < Config::GRID_SIZE; ++j) grid[i][j] = 0;
    currentTurn = 1; winnersCount = 0; piecesPlaced = 0; gameStarted = false; gameFinished = false; resultReported = false; wantToExit = false; standings.clear(); selector.clear();

    const int HOST_ID = 1;
    if (myPlayerID == HOST_ID) {
        for (auto* peer : peers) { peer->disconnect(); delete peer; }
        for (auto* p : pendingPeers) { p->disconnect(); delete p; }
        peers.clear(); pendingPeers.clear(); p2pListener.close();
        p2pListener.listen(Config::P2P_PORT_BASE); p2pListener.setBlocking(false);
    }
    else {
        hostSocket.disconnect(); connectedToHost = false; connectRetryClock.restart();
    }
}

bool GameScreen::checkDirection(int p, int x, int y, int dx, int dy) {
    int count = 0;
    const int WIN_TARGET = Config::WIN_CONDITION;
    for (int i = -2; i <= 2; i++) {
        int nx = x + i * dx, ny = y + i * dy;
        if (nx >= 0 && nx < Config::GRID_SIZE && ny >= 0 && ny < Config::GRID_SIZE && grid[nx][ny] == p) { count++; if (count == WIN_TARGET) return true; }
        else count = 0;
    } return false;
}

bool GameScreen::checkWinCondition(int player, int x, int y) {
    return checkDirection(player, x, y, 1, 0) || checkDirection(player, x, y, 0, 1) || checkDirection(player, x, y, 1, 1) || checkDirection(player, x, y, 1, -1);
}

void GameScreen::advanceTurn() {
    const int MAX_P = Config::MAX_PLAYERS;
    do { currentTurn++; if (currentTurn > MAX_P) currentTurn = 1; } while (hasWon[currentTurn] && winnersCount < 3);
    turnTimer.restart();
}

void GameScreen::broadcastState() {
    sf::Packet statePacket; statePacket << 1 << currentTurn << winnersCount << piecesPlaced;
    for (int i = 1; i <= Config::MAX_PLAYERS; i++) statePacket << hasWon[i] << isDisconnected[i];
    for (int i = 0; i < Config::GRID_SIZE; ++i) for (int j = 0; j < Config::GRID_SIZE; ++j) statePacket << grid[i][j];
    statePacket << static_cast<std::uint32_t>(standings.size());
    for (const auto& name : standings) statePacket << name;
    for (auto* peer : peers) peer->send(statePacket);
}

void GameScreen::applyMove(int player, int x, int y) {
    grid[x][y] = player;
    piecesPlaced++;

    std::string symbols[] = { "O", "X", "#", "S" };
    std::cout << "[JUEGO] " << playerNames[player - 1] << " (" << symbols[player - 1] << ") coloca en Fila " << x << ", Columna " << y << "\n";

    if (checkWinCondition(player, x, y)) {
        hasWon[player] = true; winnersCount++; standings.push_back(playerNames[player - 1]);
        std::cout << "[JUEGO] ¡" << playerNames[player - 1] << " gana la posicion " << winnersCount << "!\n";
    }
    const int MAX_WINNERS = 3;
    if (winnersCount < MAX_WINNERS) advanceTurn();
}

void GameScreen::sendMatchResult() {
    const int HOST_ID = 1;
    if (myPlayerID == HOST_ID) {
        for (auto* peer : peers) { peer->disconnect(); delete peer; }
        for (auto* p : pendingPeers) { p->disconnect(); delete p; }
        peers.clear(); pendingPeers.clear(); p2pListener.close();
    }
    else { hostSocket.disconnect(); }

    sf::TcpSocket socket; auto resolvedIPs = sf::Dns::resolve(Config::SERVER_IP);
    if (resolvedIPs.has_value() && !resolvedIPs->empty() && socket.connect((*resolvedIPs)[0], Config::BOOTSTRAP_PORT) == sf::Socket::Status::Done) {
        sf::Packet p; p << Config::NET_REPORT_RESULT << roomCode;
        for (size_t i = 0; i < 4; i++) { if (i < standings.size()) p << standings[i]; else p << "Desconectado"; }
        socket.send(p);
    }
}

void GameScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));
        const int MAX_WINNERS = 3;

        if (gameFinished && exitButton.getGlobalBounds().contains(mousePos)) { wantToExit = true; return; }
        if (!gameStarted || winnersCount >= MAX_WINNERS || hasWon[myPlayerID] || currentTurn != myPlayerID) return;

        for (int i = 0; i < Config::GRID_SIZE; ++i) {
            for (int j = 0; j < Config::GRID_SIZE; ++j) {
                if (board[i][j].getGlobalBounds().contains(mousePos) && grid[i][j] == 0) {
                    const int HOST_ID = 1;
                    if (myPlayerID == HOST_ID) { applyMove(myPlayerID, i, j); broadcastState(); }
                    else { sf::Packet movePacket; movePacket << 2 << myPlayerID << i << j; hostSocket.send(movePacket); }
                }
            }
        }
    }
}

void GameScreen::update(sf::RenderWindow& window) {
    const int HOST_ID = 1;
    const int REQUIRED_CLIENTS = Config::MAX_PLAYERS - 1;

    if (myPlayerID == HOST_ID && !gameStarted) {
        sf::TcpSocket* newPeer = new sf::TcpSocket();
        if (p2pListener.accept(*newPeer) == sf::Socket::Status::Done) { newPeer->setBlocking(false); pendingPeers.push_back(newPeer); }
        else { delete newPeer; }
        for (auto it = pendingPeers.begin(); it != pendingPeers.end(); ) {
            sf::Packet p;
            if ((*it)->receive(p) == sf::Socket::Status::Done) {
                int type, pID;
                if (p >> type >> pID && type == 3) {
                    selector.add(**it); peers.push_back(*it); it = pendingPeers.erase(it);
                    if (peers.size() == REQUIRED_CLIENTS) { gameStarted = true; turnTimer.restart(); broadcastState(); } continue;
                }
            } ++it;
        }
    }

    if (myPlayerID != HOST_ID && !connectedToHost) {
        if (connectRetryClock.getElapsedTime().asSeconds() > 1.0f) {
            connectRetryClock.restart(); auto resolvedIPs = sf::Dns::resolve(hostIP);
            if (resolvedIPs.has_value() && !resolvedIPs->empty()) {
                hostSocket.setBlocking(true);
                if (hostSocket.connect((*resolvedIPs)[0], Config::P2P_PORT_BASE, sf::milliseconds(1000)) == sf::Socket::Status::Done) {
                    hostSocket.setBlocking(false); connectedToHost = true;
                    sf::Packet hello; hello << 3 << myPlayerID; hostSocket.send(hello);
                }
            }
        }
    }

    if (!gameStarted) {
        if (myPlayerID == HOST_ID) timerText.setString("HOST: Esperando confirmacion... (" + std::to_string(peers.size()) + "/3)");
        else {
            timerText.setString(connectedToHost ? "Conectado. Esperando partida..." : "Llamando al Host...");
            if (connectedToHost) {
                sf::Packet p;
                if (hostSocket.receive(p) == sf::Socket::Status::Done) {
                    int type;
                    if (p >> type && type == 1) {
                        p >> currentTurn >> winnersCount >> piecesPlaced;
                        for (int i = 1; i <= Config::MAX_PLAYERS; i++) p >> hasWon[i] >> isDisconnected[i];
                        for (int i = 0; i < Config::GRID_SIZE; ++i) for (int j = 0; j < Config::GRID_SIZE; ++j) p >> grid[i][j];
                        std::uint32_t stSize; if (p >> stSize) { standings.clear(); for (std::uint32_t i = 0; i < stSize; i++) { std::string name; p >> name; standings.push_back(name); } }
                        turnTimer.restart(); gameStarted = true;
                    }
                }
            }
        } return;
    }

    const int MAX_WINNERS = 3;
    if (myPlayerID == HOST_ID && winnersCount < MAX_WINNERS && turnTimer.getElapsedTime().asSeconds() >= Config::TURN_TIME_LIMIT_SEC) {
        advanceTurn(); broadcastState();
    }

    if (myPlayerID == HOST_ID && selector.wait(sf::milliseconds(10))) {
        for (size_t i = 0; i < peers.size(); i++) {
            if (selector.isReady(*peers[i])) {
                sf::Packet p; auto status = peers[i]->receive(p);
                if (status == sf::Socket::Status::Disconnected) {
                    int pID = i + 2;
                    if (!isDisconnected[pID]) {
                        isDisconnected[pID] = true; hasWon[pID] = true;
                        std::cout << "[ALERTA] Jugador " << playerNames[pID - 1] << " desconectado. Penalizacion aplicada.\n";
                        if (currentTurn == pID) { advanceTurn(); broadcastState(); }
                    }
                }
                else if (status == sf::Socket::Status::Done) {
                    int type, pID, x, y;
                    if (p >> type && type == 2) {
                        p >> pID >> x >> y;
                        if (pID == currentTurn && grid[x][y] == 0 && !hasWon[pID]) { applyMove(pID, x, y); broadcastState(); }
                    }
                }
            }
        }
    }

    if (myPlayerID != HOST_ID && !gameFinished) {
        sf::Packet p;
        if (hostSocket.receive(p) == sf::Socket::Status::Done) {
            int type;
            if (p >> type && type == 1) {
                p >> currentTurn >> winnersCount >> piecesPlaced;
                for (int i = 1; i <= Config::MAX_PLAYERS; i++) p >> hasWon[i] >> isDisconnected[i];
                for (int i = 0; i < Config::GRID_SIZE; ++i) for (int j = 0; j < Config::GRID_SIZE; ++j) p >> grid[i][j];
                std::uint32_t stSize; if (p >> stSize) { standings.clear(); for (std::uint32_t i = 0; i < stSize; i++) { std::string name; p >> name; standings.push_back(name); } }
                turnTimer.restart();
            }
        }
    }

    int discCount = 0; for (int i = 1; i <= 4; i++) if (isDisconnected[i]) discCount++;
    const int MAX_PIECES = Config::GRID_SIZE * Config::GRID_SIZE;

    if ((winnersCount >= MAX_WINNERS || (winnersCount + discCount >= MAX_WINNERS) || piecesPlaced >= MAX_PIECES) && !gameFinished) {
        if (piecesPlaced >= MAX_PIECES && winnersCount < MAX_WINNERS) std::cout << "[JUEGO] Empate detectado: Tablero lleno.\n";
        for (int i = 1; i <= 4; i++) {
            bool yaEsta = false; for (auto& s : standings) if (s == playerNames[i - 1]) yaEsta = true;
            if (!yaEsta && !isDisconnected[i]) standings.push_back(playerNames[i - 1]);
        }
        for (int i = 1; i <= 4; i++) if (isDisconnected[i]) standings.push_back(playerNames[i - 1]);
        gameFinished = true;
    }

    if (gameFinished && !resultReported) {
        sendMatchResult(); resultReported = true;
        timerText.setString("PARTIDA FINALIZADA"); timerText.setFillColor(sf::Color::Green);
    }
    else if (!gameFinished) {
        if (hasWon[myPlayerID]) { timerText.setString(isDisconnected[myPlayerID] ? "DESCONECTADO" : "HAS GANADO! Modo espectador."); timerText.setFillColor(sf::Color::Cyan); }
        else {
            int timeRemaining = Config::TURN_TIME_LIMIT_SEC - static_cast<int>(turnTimer.getElapsedTime().asSeconds());
            if (timeRemaining < 0) timeRemaining = 0;
            if (currentTurn == myPlayerID) { timerText.setString("TU TURNO! Quedan: " + std::to_string(timeRemaining) + "s"); timerText.setFillColor(sf::Color::Green); }
            else { timerText.setString("Turno del Jugador " + std::to_string(currentTurn)); timerText.setFillColor(sf::Color::Yellow); }
        }
    }
}

void GameScreen::draw(sf::RenderWindow& window) {
    window.draw(titleText); window.draw(timerText); window.draw(playerInfoText);
    const float START_X = 180.f; const float START_Y = 100.f; const float CELL_SIZE = 60.f;
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