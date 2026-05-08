#include "GameScreen.h"

GameScreen::GameScreen() : titleText(font), markText(font), roomInfoText(font), myPlayerID(0), currentTurn(1), winner(0) {
    if (!font.openFromFile("assets/arial.ttf")) {}

    titleText.setCharacterSize(35);
    titleText.setPosition({ 230.f, 30.f });
    titleText.setFillColor(sf::Color::White);

    markText.setCharacterSize(60);
    markText.setFillColor(sf::Color::White);

    roomInfoText.setCharacterSize(20);
    roomInfoText.setPosition({ 20.f, 20.f });
    roomInfoText.setFillColor(sf::Color::Cyan);

    float startX = 250.f; float startY = 150.f; float cellSize = 100.f;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            grid[i][j] = 0;
            board[i][j].setSize({ cellSize - 5.f, cellSize - 5.f });
            board[i][j].setPosition({ startX + j * cellSize, startY + i * cellSize });
            board[i][j].setFillColor(sf::Color(50, 50, 50));
            board[i][j].setOutlineThickness(2.f);
            board[i][j].setOutlineColor(sf::Color::White);
        }
    }
}

void GameScreen::setRoomData(const std::string& code, int id) {
    roomCode = code;
    myPlayerID = id;
    roomInfoText.setString("Sala: " + roomCode + " | Jugador " + std::to_string(myPlayerID));
}

void GameScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (winner != 0 || currentTurn != myPlayerID) return;

        sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (board[i][j].getGlobalBounds().contains(mousePos) && grid[i][j] == 0) {
                    sf::TcpSocket socket;
                    auto ip = sf::Dns::resolve("127.0.0.1");
                    if (ip && !ip->empty() && socket.connect((*ip)[0], 50000) == sf::Socket::Status::Done) {
                        sf::Packet p;
                        p << 5 << roomCode << myPlayerID << i << j;
                        socket.send(p);
                    }
                }
            }
        }
    }
}

void GameScreen::update(sf::RenderWindow& window) {
    if (pollClock.getElapsedTime().asSeconds() > 0.5f) {
        pollClock.restart();
        sf::TcpSocket socket;
        auto ip = sf::Dns::resolve("127.0.0.1");
        if (ip && !ip->empty() && socket.connect((*ip)[0], 50000) == sf::Socket::Status::Done) {
            sf::Packet p, r;
            p << 6 << roomCode;
            socket.send(p);
            if (socket.receive(r) == sf::Socket::Status::Done) {
                r >> currentTurn >> winner;
                for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) r >> grid[i][j];
            }
        }
    }

    if (winner == 1) titleText.setString("GANA EL JUGADOR 1");
    else if (winner == 2) titleText.setString("GANA EL JUGADOR 2");
    else if (winner == 3) titleText.setString("EMPATE");
    else if (myPlayerID == currentTurn) titleText.setString("TU TURNO");
    else titleText.setString("ESPERANDO RIVAL...");
}

void GameScreen::draw(sf::RenderWindow& window) {
    window.draw(titleText);
    window.draw(roomInfoText);

    float startX = 250.f; float startY = 150.f; float cellSize = 100.f;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            window.draw(board[i][j]);
            if (grid[i][j] == 1) {
                markText.setString("X");
                markText.setPosition({ startX + j * cellSize + 25.f, startY + i * cellSize + 10.f });
                window.draw(markText);
            }
            else if (grid[i][j] == 2) {
                markText.setString("O");
                markText.setPosition({ startX + j * cellSize + 25.f, startY + i * cellSize + 10.f });
                window.draw(markText);
            }
        }
    }
}