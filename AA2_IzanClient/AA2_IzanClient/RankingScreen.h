#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>
#include <vector>

// Struct visual para iterar más cómodamente a la hora de dibujar las filas
struct RankRow {
    int pos; std::string name; int pts; int w; int l;
};

class RankingScreen {
private:
    sf::Font font;
    sf::Text titleText, backText;
    sf::RectangleShape backButton;

    // Disparador del main.cpp
    bool returnToLobby;

    std::vector<RankRow> rankingData;
    std::string myName;

public:
    RankingScreen();

    // Fetch que llamo justo ANTES de mostrar la pantalla (en el main.cpp)
    void fetchRanking(const std::string& name);

    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    bool shouldReturnToLobby() const { return returnToLobby; }
    void reset() { returnToLobby = false; rankingData.clear(); }
};