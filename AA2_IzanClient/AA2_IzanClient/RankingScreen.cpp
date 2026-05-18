#include "RankingScreen.h"
#include "Config.h"
#include <cstdint>

RankingScreen::RankingScreen() : titleText(font), backText(font), returnToLobby(false) {
    if (!font.openFromFile("assets/arial.ttf")) {}

    // Table layout constants
    const float TITLE_X = 190.f; const float TITLE_Y = 30.f;
    const unsigned int TITLE_SIZE = 35;
    const float BTN_X = 300.f; const float BTN_Y = 520.f;
    const float BTN_W = 200.f; const float BTN_H = 50.f;
    const unsigned int BTN_TXT_SIZE = 20;

    titleText.setString("TOP 10 RANKING GLOBAL");
    titleText.setCharacterSize(TITLE_SIZE);
    titleText.setPosition({ TITLE_X, TITLE_Y });
    titleText.setFillColor(sf::Color::Yellow);

    backButton.setSize({ BTN_W, BTN_H });
    backButton.setPosition({ BTN_X, BTN_Y });
    backButton.setFillColor(sf::Color(0, 150, 255));

    backText.setString("VOLVER AL LOBBY");
    backText.setCharacterSize(BTN_TXT_SIZE);
    backText.setPosition({ BTN_X + 10.f, BTN_Y + 12.f });
}

void RankingScreen::fetchRanking(const std::string& name) {
    myName = name;
    sf::TcpSocket socket;
    auto resolvedIPs = sf::Dns::resolve(Config::SERVER_IP);

    // Query Bootstrap server for Top 10 + self
    if (resolvedIPs.has_value() && !resolvedIPs->empty()) {
        if (socket.connect((*resolvedIPs)[0], Config::BOOTSTRAP_PORT) == sf::Socket::Status::Done) {
            sf::Packet p;
            p << Config::NET_GET_RANKING << myName;
            socket.send(p);

            sf::Packet r;
            if (socket.receive(r) == sf::Socket::Status::Done) {
                std::uint32_t count;
                if (r >> count) {
                    rankingData.clear();
                    for (std::uint32_t i = 0; i < count; i++) {
                        std::uint32_t pos; std::string pName; std::int32_t pts, w, l;
                        r >> pos >> pName >> pts >> w >> l;
                        rankingData.push_back({ static_cast<int>(pos), pName, static_cast<int>(pts), static_cast<int>(w), static_cast<int>(l) });
                    }
                }
            }
        }
    }
}

void RankingScreen::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));
        if (backButton.getGlobalBounds().contains(mousePos)) { returnToLobby = true; }
    }
}

void RankingScreen::update(sf::RenderWindow& window) {}

void RankingScreen::draw(sf::RenderWindow& window) {
    window.draw(titleText);

    // Draw columns at fixed X positions avoiding layout breaking
    const float X_POS = 120.f; const float X_NAME = 200.f; const float X_PTS = 400.f; const float X_W = 520.f; const float X_L = 620.f;
    const float START_Y = 100.f; const float GAP_Y = 30.f;
    const float HEADER_Y = 85.f;
    const unsigned int FONT_SIZE = 18;
    const float HIGHLIGHT_OFFSET = 20.f;

    // Table Headers
    sf::Text tPos(font); tPos.setString("POS"); tPos.setCharacterSize(FONT_SIZE); tPos.setPosition({ X_POS, HEADER_Y }); tPos.setFillColor(sf::Color::Cyan); window.draw(tPos);
    sf::Text tName(font); tName.setString("NICKNAME"); tName.setCharacterSize(FONT_SIZE); tName.setPosition({ X_NAME, HEADER_Y }); tName.setFillColor(sf::Color::Cyan); window.draw(tName);
    sf::Text tPts(font); tPts.setString("PTS"); tPts.setCharacterSize(FONT_SIZE); tPts.setPosition({ X_PTS, HEADER_Y }); tPts.setFillColor(sf::Color::Cyan); window.draw(tPts);
    sf::Text tW(font); tW.setString("W"); tW.setCharacterSize(FONT_SIZE); tW.setPosition({ X_W, HEADER_Y }); tW.setFillColor(sf::Color::Cyan); window.draw(tW);
    sf::Text tL(font); tL.setString("L"); tL.setCharacterSize(FONT_SIZE); tL.setPosition({ X_L, HEADER_Y }); tL.setFillColor(sf::Color::Cyan); window.draw(tL);

    // Loop through retrieved data and print each row
    for (size_t i = 0; i < rankingData.size(); ++i) {
        float y = START_Y + GAP_Y + (i * GAP_Y);

        // Give a bit of margin if drawing the player outside the top 10
        const size_t TOP_LIMIT = 10;
        if (i >= TOP_LIMIT) y += HIGHLIGHT_OFFSET;

        sf::Color rowColor = (rankingData[i].name == myName) ? sf::Color::Yellow : sf::Color::White;

        sf::Text rowPos(font);
        rowPos.setString(std::to_string(rankingData[i].pos) + ".");
        rowPos.setCharacterSize(FONT_SIZE);
        rowPos.setPosition({ X_POS, y });
        rowPos.setFillColor(rowColor);
        window.draw(rowPos);

        sf::Text rowName(font);
        rowName.setString(rankingData[i].name);
        rowName.setCharacterSize(FONT_SIZE);
        rowName.setPosition({ X_NAME, y });
        rowName.setFillColor(rowColor);
        window.draw(rowName);

        sf::Text rowPts(font);
        rowPts.setString(std::to_string(rankingData[i].pts));
        rowPts.setCharacterSize(FONT_SIZE);
        rowPts.setPosition({ X_PTS, y });
        rowPts.setFillColor(rowColor);
        window.draw(rowPts);

        sf::Text rowW(font);
        rowW.setString(std::to_string(rankingData[i].w));
        rowW.setCharacterSize(FONT_SIZE);
        rowW.setPosition({ X_W, y });
        rowW.setFillColor(rowColor);
        window.draw(rowW);

        sf::Text rowL(font);
        rowL.setString(std::to_string(rankingData[i].l));
        rowL.setCharacterSize(FONT_SIZE);
        rowL.setPosition({ X_L, y });
        rowL.setFillColor(rowColor);
        window.draw(rowL);
    }

    window.draw(backButton);
    window.draw(backText);
}