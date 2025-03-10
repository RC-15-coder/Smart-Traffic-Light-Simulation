#ifndef VEHICLE_HPP
#define VEHICLE_HPP

#include <SFML/Graphics.hpp>

enum class VehicleType {
    Normal,
    Taxi,
    Ambulance,
    Audi,
    Truck,
    Bus,
    BlackViper,
    BigTruck
};

enum class Direction {
    TopToBottom,
    BottomToTop,
    LeftToRight,
    RightToLeft
};

class Vehicle {
public:
    Vehicle(const sf::Vector2f& startPos, VehicleType type, Direction dir);

    void update(float dt, float speed);
    void render(sf::RenderWindow& window);

    VehicleType getType() const;
    Direction getDirection() const;

    float getX() const;
    float getY() const;

    // New methods for stop-line logic
    bool hasPassedStopLine() const;
    void setPassedStopLine(bool val);

    // New: Store the last position for displacement calculation.
    sf::Vector2f lastPosition;

    float stoppedTime;  // in seconds

    float speed; // public or add a getter function
    float getSpeed() const { return speed; }

private:
    VehicleType type;
    Direction direction;

    sf::Sprite sprite;
    sf::Texture texture;
    bool textureLoaded;
    sf::RectangleShape fallback;

    // Flag indicating the vehicle has crossed the intersection stop line
    bool passedStopLine;
};

#endif
