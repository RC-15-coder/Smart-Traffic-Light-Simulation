#include "TrafficLight.hpp"
#include <iostream>

TrafficLight::TrafficLight(const sf::Vector2f& position)
    : state(LightState::Red),
      timer(0.f),
      redDuration(5.f),
      yellowDuration(2.f),
      greenDuration(5.f)
{
    // 1) Load traffic light textures
    if (!redTexture.loadFromFile("C:/TrafficLightSimulation/assets/red.png")) {
        std::cerr << "Failed to load red.png\n";
    }
    if (!yellowTexture.loadFromFile("C:/TrafficLightSimulation/assets/yellow.png")) {
        std::cerr << "Failed to load yellow.png\n";
    }
    if (!greenTexture.loadFromFile("C:/TrafficLightSimulation/assets/green.png")) {
        std::cerr << "Failed to load green.png\n";
    }

    // 2) Load the post texture
    if (!postTexture.loadFromFile("C:/TrafficLightSimulation/assets/post.png")) {
        std::cerr << "Failed to load post.png\n";
    }

    // ------------------------------------------------------
    // 3) Configure the traffic light sprite (initially red)
    // ------------------------------------------------------
    sprite.setTexture(redTexture);
    // Unified scale: let's use 0.04 for the light
    sprite.setScale(0.04f, 0.04f);

    sf::Vector2u lightSize = redTexture.getSize();
    // Anchor the light so its bottom-center is at (0, 0)
    sprite.setOrigin(lightSize.x * 0.5f, lightSize.y);

    // ------------------------------------------------------
    // 4) Configure the post sprite
    // ------------------------------------------------------
    postSprite.setTexture(postTexture);
    // A bit smaller scale for the post
    postSprite.setScale(0.02f, 0.02f);

    sf::Vector2u postSize = postTexture.getSize();
    // Anchor the post so its top-center is at (0, 0)
    postSprite.setOrigin(postSize.x * 0.5f, 0.f);

    // place the meeting point (where the light’s bottom meets the post’s top)
    // at some offset below 'position' so that the bottom of the light is further down.
    // A positive offsetY moves it downward on the screen.
    float offsetY = +2.f; // Move entire assembly 10 pixels lower on screen
    float finalX = position.x;
    float finalY = position.y + offsetY;

    sprite.setPosition(finalX, finalY);
    postSprite.setPosition(finalX, finalY);

    // The traffic light is initially red
    state = LightState::Red;
}

void TrafficLight::switchToRed() {
    sprite.setTexture(redTexture);
    sprite.setScale(0.04f, 0.04f);

    sf::Vector2u texSize = redTexture.getSize();
    // bottom-center origin
    sprite.setOrigin(texSize.x * 0.5f, texSize.y);

    state = LightState::Red;
}

void TrafficLight::switchToYellow() {
    sprite.setTexture(yellowTexture);
    sprite.setScale(0.04f, 0.04f);

    sf::Vector2u texSize = yellowTexture.getSize();
    sprite.setOrigin(texSize.x * 0.5f, texSize.y);

    state = LightState::Yellow;
}

void TrafficLight::switchToGreen() {
    sprite.setTexture(greenTexture);
    sprite.setScale(0.04f, 0.04f);

    sf::Vector2u texSize = greenTexture.getSize();
    sprite.setOrigin(texSize.x * 0.5f, texSize.y);

    state = LightState::Green;
}

void TrafficLight::update(float dt) {
    // If you want each light to cycle independently
    timer += dt;
    switch (state) {
        case LightState::Red:
            if (timer >= redDuration) {
                switchToGreen();
                timer = 0.f;
            }
            break;
        case LightState::Green:
            if (timer >= greenDuration) {
                switchToYellow();
                timer = 0.f;
            }
            break;
        case LightState::Yellow:
            if (timer >= yellowDuration) {
                switchToRed();
                timer = 0.f;
            }
            break;
    }
}

void TrafficLight::render(sf::RenderWindow& window) {
    // Draw the post first
    window.draw(postSprite);
    // Then draw the traffic light on top
    window.draw(sprite);
}

LightState TrafficLight::getState() const {
    return state;
}

sf::Vector2f TrafficLight::getPosition() const {
    return sprite.getPosition();
}

void TrafficLight::setState(LightState newState) {
    timer = 0.f;
    if (newState == LightState::Red) switchToRed();
    else if (newState == LightState::Yellow) switchToYellow();
    else switchToGreen();
}


