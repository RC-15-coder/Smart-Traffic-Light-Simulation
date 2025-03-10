#ifndef TRAFFICLIGHT_HPP
#define TRAFFICLIGHT_HPP

#include <SFML/Graphics.hpp>

// Simple states for a single traffic light
// Red/Yellow/Green for this example
enum class LightState { Red, Yellow, Green };

class TrafficLight {
public:
    TrafficLight(const sf::Vector2f& position);

    void update(float dt);
    void render(sf::RenderWindow& window);

    LightState getState() const;
    sf::Vector2f getPosition() const;

    // We'll add these to let us manually switch states 
    // if you want to handle them externally (e.g., 2-phase logic).
    void setState(LightState newState);

private:
    LightState state;
    float timer;
    float redDuration;
    float yellowDuration;
    float greenDuration;

    sf::Texture redTexture;
    sf::Texture yellowTexture;
    sf::Texture greenTexture;
    sf::Sprite sprite;

    // New: a post texture and sprite
    sf::Texture postTexture;
    sf::Sprite postSprite;

    void switchToRed();
    void switchToYellow();
    void switchToGreen();
};

#endif // TRAFFICLIGHT_HPP
