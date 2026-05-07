#include "LoginMenu.h"

// FÍJATE AQUÍ: Añadimos los dos puntos (:) y pasamos la 'font' a todos los textos
LoginMenu::LoginMenu()
    : titleText(font), loginText(font), registerText(font), userLabel(font), passLabel(font)
{
    // 1. Cargar la fuente (Si falla, avisamos por consola)
    if (!font.openFromFile("assets/arial.ttf")) {
        std::cerr << "Error: No se pudo cargar la fuente arial.ttf\n";
    }

    // 2. Configurar el Título (Ya no hace falta usar setFont aquí)
    titleText.setString("3 EN RAYA ONLINE");
    titleText.setCharacterSize(40);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(sf::Vector2f(220.f, 50.f));

    // 3. Configurar Cajas de Texto (Fondos blancos)
    userBox.setSize(sf::Vector2f(300.f, 40.f));
    userBox.setPosition(sf::Vector2f(250.f, 150.f));
    userBox.setFillColor(sf::Color::White);

    passBox.setSize(sf::Vector2f(300.f, 40.f));
    passBox.setPosition(sf::Vector2f(250.f, 250.f));
    passBox.setFillColor(sf::Color::White);

    // 4. Etiquetas de las cajas (Quitamos setFont)
    userLabel.setString("Nickname:");
    userLabel.setCharacterSize(20);
    userLabel.setFillColor(sf::Color::White);
    userLabel.setPosition(sf::Vector2f(250.f, 120.f));

    passLabel.setString("Contrasena:"); // Evitamos la 'ñ'
    passLabel.setCharacterSize(20);
    passLabel.setFillColor(sf::Color::White);
    passLabel.setPosition(sf::Vector2f(250.f, 220.f));

    // 5. Configurar Botones
    loginButton.setSize(sf::Vector2f(140.f, 50.f));
    loginButton.setPosition(sf::Vector2f(250.f, 350.f));
    loginButton.setFillColor(sf::Color(0, 150, 255)); // Azul claro

    registerButton.setSize(sf::Vector2f(140.f, 50.f));
    registerButton.setPosition(sf::Vector2f(410.f, 350.f));
    registerButton.setFillColor(sf::Color(0, 200, 0)); // Verde

    // Textos de los botones (Quitamos setFont)
    loginText.setString("LOGIN");
    loginText.setCharacterSize(20);
    loginText.setFillColor(sf::Color::White);
    loginText.setPosition(sf::Vector2f(285.f, 360.f));

    registerText.setString("REGISTRO");
    registerText.setCharacterSize(20);
    registerText.setFillColor(sf::Color::White);
    registerText.setPosition(sf::Vector2f(425.f, 360.f));
}

// Deja los métodos update y draw tal cual los tenías
void LoginMenu::update(sf::RenderWindow& window) {
    // Aquí irá la lógica de detectar el ratón y el teclado
}

void LoginMenu::draw(sf::RenderWindow& window) {
    window.draw(titleText);

    window.draw(userLabel);
    window.draw(userBox);

    window.draw(passLabel);
    window.draw(passBox);

    window.draw(loginButton);
    window.draw(loginText);

    window.draw(registerButton);
    window.draw(registerText);
}