#include <SFML/Graphics.hpp>
#include "TrafficManager.hpp"
#include <sstream>
#include <iostream>

// Helper function: checks if the mouse is over a given rectangle
bool isMouseOverButton(const sf::RectangleShape& button, const sf::Vector2f& mousePos)
{
    return button.getGlobalBounds().contains(mousePos);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(900, 600), "4-Way Intersection");
    window.setFramerateLimit(60);

    // Create roads and intersection objects as before
    sf::RectangleShape horizontalRoad(sf::Vector2f(900.f, 200.f));
    horizontalRoad.setFillColor(sf::Color(60, 60, 60));
    horizontalRoad.setPosition(0.f, 200.f);

    sf::RectangleShape verticalRoad(sf::Vector2f(200.f, 600.f));
    verticalRoad.setFillColor(sf::Color(60, 60, 60));
    verticalRoad.setPosition(350.f, 0.f);

    sf::RectangleShape horizontalLaneLine(sf::Vector2f(900.f, 2.f));
    horizontalLaneLine.setFillColor(sf::Color::White);
    horizontalLaneLine.setPosition(0.f, 300.f);

    sf::RectangleShape verticalLaneLine(sf::Vector2f(2.f, 600.f));
    verticalLaneLine.setFillColor(sf::Color::White);
    verticalLaneLine.setPosition(450.f, 0.f);

    // Create the TrafficManager instance (make sure it's declared before using in the button callback)
    TrafficManager manager;

    // --- Button Setup ---
    // Create a rectangle shape for the button
    sf::RectangleShape button(sf::Vector2f(200, 58));
    button.setFillColor(sf::Color::Black); 
    button.setPosition(650, 20);

    // Load a font for the button label (adjust the path to your font file)
    sf::Font font;
    if (!font.loadFromFile("C:/TrafficLightSimulation/assets/OpenSans-Regular.ttf")) {
        std::cerr << "Failed to load font for button" << std::endl;
        // Handle error as needed
    }

    // Create a text label to display the spawn interval on the button
    sf::Text buttonText("Spawn Interval: 1.0f", font, 18);
    buttonText.setFillColor(sf::Color::White);
    // Position the text relative to the button
    buttonText.setPosition(button.getPosition().x + 15, button.getPosition().y + 15);

    // the button click to cycle through spawn interval options.
    auto updateSpawnInterval = [&manager, &buttonText]() {
        // Cycle through: if current is 1.0f, set to 3.0f; if 3.0f, set to 0.5f; otherwise, reset to 1.0f.
        if (manager.getSpawnInterval() == 1.0f) {
            manager.setSpawnInterval(3.0f);
            buttonText.setString("Spawn Interval: 3.0f");
        } else if (manager.getSpawnInterval() == 3.0f) {
            manager.setSpawnInterval(0.5f);
            buttonText.setString("Spawn Interval: 0.5f");
        } else {
            manager.setSpawnInterval(1.0f);
            buttonText.setString("Spawn Interval: 1.0f");
        }
        std::cout << "[DEBUG] Updated spawn interval to: " << manager.getSpawnInterval() << "\n";
    };

    // --- End Button Setup ---

    sf::Clock clock;

    // Load a font for HUD overlay (if needed)
    sf::Font hudFont;
    if (!hudFont.loadFromFile("C:/TrafficLightSimulation/assets/OpenSans-Regular.ttf")) {
        std::cerr << "Failed to load font for HUD\n";
    }

    // Main Loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            // Check if window is closed
            if (event.type == sf::Event::Closed)
                window.close();

            // Check for mouse button press event
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                if (isMouseOverButton(button, mousePos)) {
                    updateSpawnInterval();
                }
            }
        }

        float dt = clock.restart().asSeconds();
        manager.update(dt);

        window.clear(sf::Color(100, 200, 200)); // Clear with background color

        // Draw the roads and lane lines
        window.draw(horizontalRoad);
        window.draw(verticalRoad);
        window.draw(horizontalLaneLine);
        window.draw(verticalLaneLine);

        // Render the simulation (lights, vehicles, etc.)
        manager.render(window);

        // Draw HUD overlay (vehicle count, etc.)
        std::stringstream ss;
        ss << "Vehicles on road: " << manager.getVehicleCount();
        sf::Text hudText;
        hudText.setFont(hudFont);
        hudText.setString(ss.str());
        hudText.setCharacterSize(20);
        hudText.setFillColor(sf::Color::Black);
        hudText.setPosition(10.f, 10.f);
        window.draw(hudText);

        // Draw the button and its label on top
        window.draw(button);
        window.draw(buttonText);

        window.display();
    }

    return 0;
}
