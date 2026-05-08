#include "GameScreen.h"
#include <iostream>

GameScreen::GameScreen() : titleText(font), markText(font) {
    if (!font.openFromFile("assets/arial.ttf")) {
        std::cerr << "Error\n";
    }

    titleText.setString("PARTIDA EN CURSO");
    titleText.setCharacterSize(35);
    titleText.setPosition({ 230.f, 30.f });
    titleText.setFillColor(sf::Color::White);

    markText.setCharacterSize(60);
    markText.setFillColor(sf::Color::White);

    float startX = 250.f;
    float startY = 150.f;
    float cellSize = 100.f;

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

void GameScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (board[i][j].getGlobalBounds().contains(mousePos)) {
                    if (grid[i][j] == 0) {
                        std::cout << "Casilla: Fila " << i << ", Columna " << j << "\n";
                        grid[i][j] = 1;
                    }
                }
            }
        }
    }
}

void GameScreen::update(sf::RenderWindow& window) {
}

void GameScreen::draw(sf::RenderWindow& window) {
    window.draw(titleText);
    float startX = 250.f;
    float startY = 150.f;
    float cellSize = 100.f;

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