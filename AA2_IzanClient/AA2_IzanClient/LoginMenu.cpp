#include "LoginMenu.h"

LoginMenu::LoginMenu()
    : titleText(font), loginText(font), registerText(font), userLabel(font),
    passLabel(font), displayUser(font), displayPass(font)
{
    activeBox = 0;

    if (!font.openFromFile("assets/arial.ttf")) {
        std::cerr << "Error cargando fuente\n";
    }

    titleText.setString("3 EN RAYA ONLINE");
    titleText.setCharacterSize(40);
    titleText.setPosition({ 220.f, 50.f });

    userBox.setSize({ 300.f, 40.f });
    userBox.setPosition({ 250.f, 150.f });
    userBox.setFillColor(sf::Color::White);

    passBox.setSize({ 300.f, 40.f });
    passBox.setPosition({ 250.f, 250.f });
    passBox.setFillColor(sf::Color::White);

    loginButton.setSize({ 140.f, 50.f });
    loginButton.setPosition({ 250.f, 350.f });
    loginButton.setFillColor(sf::Color(0, 150, 255));

    registerButton.setSize({ 140.f, 50.f });
    registerButton.setPosition({ 410.f, 350.f });
    registerButton.setFillColor(sf::Color(0, 200, 0));

    userLabel.setString("Nickname:");
    userLabel.setPosition({ 250.f, 120.f });
    passLabel.setString("Contrasena:");
    passLabel.setPosition({ 250.f, 220.f });
    loginText.setString("LOGIN");
    loginText.setPosition({ 285.f, 360.f });
    registerText.setString("REGISTRO");
    registerText.setPosition({ 425.f, 360.f });

    displayUser.setFillColor(sf::Color::Black);
    displayUser.setPosition({ 255.f, 155.f });
    displayPass.setFillColor(sf::Color::Black);
    displayPass.setPosition({ 255.f, 255.f });
}

void LoginMenu::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));

        if (userBox.getGlobalBounds().contains(mousePos)) activeBox = 1;
        else if (passBox.getGlobalBounds().contains(mousePos)) activeBox = 2;
        else activeBox = 0;

        bool isLogin = loginButton.getGlobalBounds().contains(mousePos);
        bool isRegister = registerButton.getGlobalBounds().contains(mousePos);

        if (isLogin || isRegister) {
            sf::TcpSocket socket;
            auto resolvedIPs = sf::Dns::resolve("127.0.0.1");

            if (resolvedIPs.has_value() && !resolvedIPs->empty()) {

                if (socket.connect((*resolvedIPs)[0], 50000) == sf::Socket::Status::Done) {

                    sf::Packet packet;
                    int type = isLogin ? 1 : 2;
                    packet << type << inputUser << inputPass;
                    socket.send(packet);

                    sf::Packet respuesta;
                    if (socket.receive(respuesta) == sf::Socket::Status::Done) {
                        int estado;
                        respuesta >> estado;

                        if (estado == 1) {
                            if (isLogin) std::cout << "\n[OK] Login correcto. Entrando al Lobby...\n";
                            else std::cout << "\n[OK] Registro correcto. Ya puedes hacer Login.\n";
                        }
                        else {
                            if (isLogin) std::cout << "\n[ERROR] Contrasena incorrecta o usuario no existe.\n";
                            else std::cout << "\n[ERROR] El nickname ya esta en uso.\n";
                        }
                    }

                }
                else {
                    std::cerr << "Error: Servidor offline.\n";
                }
            }
         
        }
    }

    if (const auto* textEvent = event.getIf<sf::Event::TextEntered>()) {
        if (activeBox != 0) {
            if (textEvent->unicode == 8) { 
                if (activeBox == 1 && !inputUser.empty()) inputUser.pop_back();
                else if (activeBox == 2 && !inputPass.empty()) inputPass.pop_back();
            }
            else if (textEvent->unicode >= 32 && textEvent->unicode < 128) {
                if (activeBox == 1) inputUser += static_cast<char>(textEvent->unicode);
                else inputPass += static_cast<char>(textEvent->unicode);
            }
            displayUser.setString(inputUser);
            displayPass.setString(std::string(inputPass.length(), '*'));
        }
    }
}

void LoginMenu::update(sf::RenderWindow& window) {
    userBox.setOutlineThickness(activeBox == 1 ? 2.f : 0.f);
    userBox.setOutlineColor(sf::Color::Cyan);
    passBox.setOutlineThickness(activeBox == 2 ? 2.f : 0.f);
    passBox.setOutlineColor(sf::Color::Cyan);
}

void LoginMenu::draw(sf::RenderWindow& window) {
    window.draw(titleText);
    window.draw(userLabel); window.draw(userBox); window.draw(displayUser);
    window.draw(passLabel); window.draw(passBox); window.draw(displayPass);
    window.draw(loginButton); window.draw(loginText);
    window.draw(registerButton); window.draw(registerText);
}