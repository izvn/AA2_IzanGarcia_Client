#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>
#include <vector>

// Used to cleanly map the data for the table layout
struct RankRow {
    int pos; std::string name; int pts; int w; int l;
};

// Displays the global Top 10 users and the player's personal placement
class RankingScreen {
private:
    sf::Font font;
    sf::Text titleText, backText;
    sf::RectangleShape backButton;
    bool returnToLobby;

    std::vector<RankRow> rankingData;
    std::string myName;

public:
    RankingScreen();

    // Requests the ranking data from the Bootstrap Server
    void fetchRanking(const std::string& name);

    // Handles returning to the lobby
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(sf::RenderWindow& window);

    // Draws the text columns aligned perfectly
    void draw(sf::RenderWindow& window);

    bool shouldReturnToLobby() const { return returnToLobby; }
    void reset() { returnToLobby = false; rankingData.clear(); }
};