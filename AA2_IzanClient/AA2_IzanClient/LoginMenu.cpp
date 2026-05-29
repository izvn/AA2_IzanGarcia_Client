#include "LoginMenu.h"
#include "Config.h"
#include <iostream>
#include <cstdint>

LoginMenu::LoginMenu() : titleText(font), userLabel(font), passLabel(font), loginText(font), regText(font), statusText(font), displayUser(font), displayPass(font) {
    // Si no carga la fuente, el texto de SFML peta. Siempre comprobar.
    if (!font.openFromFile("assets/arial.ttf")) {}

    // TRUCO: Variables de diseño en constantes locales. 
    // Así si el profe me dice "mueve este botón 10px", solo toco aquí y no busco números sueltos por el código.
    const float TITLE_X = 220.f; const float TITLE_Y = 50.f;
    const unsigned int TITLE_SIZE = 40;
    const float COL_X = 250.f;
    const float USER_LBL_Y = 150.f; const float USER_BOX_Y = 180.f; const float USER_TXT_Y = 185.f;
    const float PASS_LBL_Y = 250.f; const float PASS_BOX_Y = 280.f; const float PASS_TXT_Y = 285.f;
    const float BOX_W = 300.f; const float BOX_H = 40.f;
    const float BTN_H = 50.f;
    const float L_BTN_Y = 360.f; const float R_BTN_Y = 430.f;
    const unsigned int LABEL_SIZE = 20; const unsigned int TXT_SIZE = 22;
    const float STATUS_Y = 500.f;
    const unsigned int STATUS_SIZE = 18;

    // Configuración de los textos y cajas (posiciones, colores, tamaños)
    titleText.setString("3 EN RAYA ONLINE");
    titleText.setCharacterSize(TITLE_SIZE);
    titleText.setPosition({ TITLE_X, TITLE_Y });
    titleText.setFillColor(sf::Color::White);

    userLabel.setString("Nickname:"); userLabel.setCharacterSize(LABEL_SIZE); userLabel.setPosition({ COL_X, USER_LBL_Y });
    userBox.setSize({ BOX_W, BOX_H }); userBox.setPosition({ COL_X, USER_BOX_Y }); userBox.setFillColor(sf::Color::White);
    displayUser.setCharacterSize(TXT_SIZE); displayUser.setPosition({ COL_X + 10.f, USER_TXT_Y }); displayUser.setFillColor(sf::Color::Black);

    passLabel.setString("Contrasena:"); passLabel.setCharacterSize(LABEL_SIZE); passLabel.setPosition({ COL_X, PASS_LBL_Y });
    passBox.setSize({ BOX_W, BOX_H }); passBox.setPosition({ COL_X, PASS_BOX_Y }); passBox.setFillColor(sf::Color::White);
    displayPass.setCharacterSize(TXT_SIZE); displayPass.setPosition({ COL_X + 10.f, PASS_TXT_Y }); displayPass.setFillColor(sf::Color::Black);

    loginButton.setSize({ BOX_W, BTN_H }); loginButton.setPosition({ COL_X, L_BTN_Y }); loginButton.setFillColor(sf::Color(0, 150, 255));
    loginText.setString("LOGIN"); loginText.setCharacterSize(TXT_SIZE); loginText.setPosition({ COL_X + 115.f, L_BTN_Y + 10.f }); loginText.setFillColor(sf::Color::White);

    regButton.setSize({ BOX_W, BTN_H }); regButton.setPosition({ COL_X, R_BTN_Y }); regButton.setFillColor(sf::Color(0, 200, 0));
    regText.setString("REGISTRO"); regText.setCharacterSize(TXT_SIZE); regText.setPosition({ COL_X + 95.f, R_BTN_Y + 10.f }); regText.setFillColor(sf::Color::White);

    statusText.setCharacterSize(STATUS_SIZE); statusText.setPosition({ COL_X, STATUS_Y }); statusText.setFillColor(sf::Color::Yellow);

    userBoxActive = false; passBoxActive = false; loginSuccessful = false;
}

void LoginMenu::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    // 1. EVENTOS DE RATÓN (Clicks)
    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        // Pillo las coordenadas del click
        sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));

        // Reviso si el click ha chocado con el "bounding box" (los límites) de mis cajas de texto.
        // Si es así, las marco como activas para que el teclado escriba en ellas.
        userBoxActive = userBox.getGlobalBounds().contains(mousePos);
        passBoxActive = passBox.getGlobalBounds().contains(mousePos);

        bool isLogin = loginButton.getGlobalBounds().contains(mousePos);
        bool isReg = regButton.getGlobalBounds().contains(mousePos);

        // Si clico en un botón y los campos no están vacíos -> Intento conectar al Server Bootstrap (Puerto 50000)
        if ((isLogin || isReg) && !inputUser.empty() && !inputPass.empty()) {
            sf::TcpSocket socket; // Ojo: socket local que se destruirá al salir de este if
            auto resolvedIPs = sf::Dns::resolve(Config::SERVER_IP);

            // Si la IP es válida y logro conectarme...
            if (resolvedIPs.has_value() && !resolvedIPs->empty() && socket.connect((*resolvedIPs)[0], Config::BOOTSTRAP_PORT) == sf::Socket::Status::Done) {
                sf::Packet packet;
                // Empaqueto mi código de protocolo (Login o Registro) y los datos
                packet << (isLogin ? Config::NET_LOGIN : Config::NET_REGISTER) << inputUser << inputPass;
                socket.send(packet);

                sf::Packet response;
                // Como este socket es BLOQUEANTE por defecto, el juego se "congela" aquí un microsegundo
                // hasta que el server responde. Para un login está bien porque es instantáneo.
                if (socket.receive(response) == sf::Socket::Status::Done) {
                    const int SERVER_SUCCESS = 1;
                    int success;
                    response >> success; // Leo la respuesta (1 = OK, 0 = FAIL)

                    if (success == SERVER_SUCCESS) {
                        if (isLogin) {
                            loginSuccessful = true; // Gatillo para que el main.cpp cambie el estado a LOBBY
                            std::cout << "[LOGIN] Session started successfully.\n";
                        }
                        else {
                            statusText.setString("Registro completado. Ahora haz Login.");
                            statusText.setFillColor(sf::Color::Green);
                            std::cout << "[REGISTRO] User " << inputUser << " successfully registered.\n";
                        }
                    }
                    else {
                        std::cout << "[ERROR] Invalid login/register credentials.\n";
                        statusText.setString(isLogin ? "Error: Credenciales incorrectas." : "Error: El usuario ya existe.");
                        statusText.setFillColor(sf::Color::Red);
                    }
                }
            }
        }
    }

    // 2. EVENTOS DE TECLADO (Escribir)
    if (const auto* textEvent = event.getIf<sf::Event::TextEntered>()) {
        const std::uint32_t KEY_BACKSPACE = 8;
        const std::uint32_t KEY_SPACE = 32;
        const std::uint32_t KEY_TILDE = 126;
        const size_t MAX_LEN = 15; // Limito a 15 caracteres para que no se salga de la caja de SFML

        // Si pulso borrar y la cadena no está vacía, saco el último caracter
        if (textEvent->unicode == KEY_BACKSPACE) {
            if (userBoxActive && !inputUser.empty()) inputUser.pop_back();
            else if (passBoxActive && !inputPass.empty()) inputPass.pop_back();
        }
        // Si es una letra normal (ASCII imprimible entre el espacio y la tilde)
        else if (textEvent->unicode >= KEY_SPACE && textEvent->unicode <= KEY_TILDE) {
            if (userBoxActive && inputUser.length() < MAX_LEN) inputUser += static_cast<char>(textEvent->unicode);
            else if (passBoxActive && inputPass.length() < MAX_LEN) inputPass += static_cast<char>(textEvent->unicode);
        }

        displayUser.setString(inputUser);

        // DETALLE DE SEGURIDAD VISUAL: Enmascaro la contraseña.
        // Creo un string nuevo lleno de asteriscos de la misma longitud que la password real.
        std::string hiddenPass(inputPass.length(), '*');
        displayPass.setString(hiddenPass);
    }
}

void LoginMenu::update(sf::RenderWindow& window) {
    const float THICKNESS_ACTIVE = 3.f;
    const float THICKNESS_INACTIVE = 0.f;

    // Feedback visual: Le pongo un borde rojo a la caja de texto en la que estoy escribiendo.
    userBox.setOutlineThickness(userBoxActive ? THICKNESS_ACTIVE : THICKNESS_INACTIVE);
    userBox.setOutlineColor(sf::Color::Red);
    passBox.setOutlineThickness(passBoxActive ? THICKNESS_ACTIVE : THICKNESS_INACTIVE);
    passBox.setOutlineColor(sf::Color::Red);
}

void LoginMenu::draw(sf::RenderWindow& window) {
    // Dibujo todos los rectángulos y textos capa a capa (el orden importa, el último tapa al anterior)
    window.draw(titleText);
    window.draw(userLabel); window.draw(userBox); window.draw(displayUser);
    window.draw(passLabel); window.draw(passBox); window.draw(displayPass);
    window.draw(loginButton); window.draw(loginText);
    window.draw(regButton); window.draw(regText);
    window.draw(statusText);
}