#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>
#include <vector>

struct RankRow {
    int pos; std::string name; int pts; int w; int l;
};

class RankingScreen {
private:
    sf::Font font;
    sf::Text titleText, backText; // ELIMINADO rankingText
    sf::RectangleShape backButton;
    bool returnToLobby;

    std::vector<RankRow> rankingData;
    std::string myName;

public:
    RankingScreen();
    void fetchRanking(const std::string& name);
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);
    bool shouldReturnToLobby() const { return returnToLobby; }
    void reset() { returnToLobby = false; rankingData.clear(); }
};