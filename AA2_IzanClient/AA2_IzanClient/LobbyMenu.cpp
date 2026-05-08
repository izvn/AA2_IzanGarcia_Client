#include "LobbyMenu.h"

LobbyMenu::LobbyMenu()
    : titleText(font), createText(font), joinText(font), inputLabel(font), displayRoomCode(font), boxActive(false), inRoom(false)
{
    if (!font.openFromFile("assets/arial.ttf")) {
        std::cerr << "Error\n";
    }

    titleText.setString("LOBBY MULTIJUGADOR");
    titleText.setCharacterSize(35);
    titleText.setPosition({ 190.f, 50.f });
    titleText.setFillColor(sf::Color::White);

    createButton.setSize({ 200.f, 60.f });
    createButton.setPosition({ 150.f, 200.f });
    createButton.setFillColor(sf::Color(0, 150, 255));

    createText.setString("CREAR SALA");
    createText.setCharacterSize(20);
    createText.setPosition({ 180.f, 215.f });
    createText.setFillColor(sf::Color::White);

    joinButton.setSize({ 200.f, 60.f });
    joinButton.setPosition({ 450.f, 200.f });
    joinButton.setFillColor(sf::Color(255, 120, 0));

    joinText.setString("UNIRSE");
    joinText.setCharacterSize(20);
    joinText.setPosition({ 510.f, 215.f });
    joinText.setFillColor(sf::Color::White);

    inputLabel.setString("Codigo de Sala:");
    inputLabel.setCharacterSize(18);
    inputLabel.setPosition({ 450.f, 280.f });
    inputLabel.setFillColor(sf::Color::White);

    inputBox.setSize({ 200.f, 40.f });
    inputBox.setPosition({ 450.f, 310.f });
    inputBox.setFillColor(sf::Color::White);

    displayRoomCode.setCharacterSize(22);
    displayRoomCode.setPosition({ 460.f, 315.f });
    displayRoomCode.setFillColor(sf::Color::Black);
}

void LobbyMenu::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));

        if (inputBox.getGlobalBounds().contains(mousePos)) boxActive = true;
        else boxActive = false;

        bool isCreate = createButton.getGlobalBounds().contains(mousePos);
        bool isJoin = joinButton.getGlobalBounds().contains(mousePos);

        if (isCreate || (isJoin && !roomCode.empty())) {
            sf::TcpSocket socket;
            auto resolvedIPs = sf::Dns::resolve("127.0.0.1");

            if (resolvedIPs.has_value() && !resolvedIPs->empty()) {
                if (socket.connect((*resolvedIPs)[0], 50000) == sf::Socket::Status::Done) {
                    sf::Packet packet;
                    if (isCreate) packet << 3;
                    else packet << 4 << roomCode;

                    socket.send(packet);

                    sf::Packet respuesta;
                    if (socket.receive(respuesta) == sf::Socket::Status::Done) {
                        int estado;
                        respuesta >> estado;

                        if (estado == 1) {
                            if (isCreate) {
                                respuesta >> roomCode;
                                displayRoomCode.setString(roomCode);
                                std::cout << "Sala generada: " << roomCode << "\n";
                            }
                            else {
                                std::cout << "Acceso permitido a sala: " << roomCode << "\n";
                            }
                            inRoom = true;
                        }
                        else {
                            std::cout << "Error: Codigo incorrecto o sala llena.\n";
                        }
                    }
                }
            }
        }
    }

    if (const auto* textEvent = event.getIf<sf::Event::TextEntered>()) {
        if (boxActive) {
            if (textEvent->unicode == 8 && !roomCode.empty()) {
                roomCode.pop_back();
                inRoom = false;
            }
            else if (textEvent->unicode >= 48 && textEvent->unicode <= 57 && roomCode.length() < 4) {
                roomCode += static_cast<char>(textEvent->unicode);
                inRoom = false;
            }
            displayRoomCode.setString(roomCode);
        }
    }
}

void LobbyMenu::update(sf::RenderWindow& window) {
    inputBox.setOutlineThickness(boxActive ? 3.f : 0.f);
    inputBox.setOutlineColor(sf::Color::Red);
}

void LobbyMenu::draw(sf::RenderWindow& window) {
    window.draw(titleText);
    window.draw(createButton);
    window.draw(createText);
    window.draw(joinButton);
    window.draw(joinText);
    window.draw(inputLabel);
    window.draw(inputBox);
    window.draw(displayRoomCode);
}

bool LobbyMenu::isRoomReady() const {
    return inRoom;
}