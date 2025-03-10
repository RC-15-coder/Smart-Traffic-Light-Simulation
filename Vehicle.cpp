#include "Vehicle.hpp"
#include <iostream>
#include <cstdlib>
#include <cmath>

using namespace std;

Vehicle::Vehicle(const sf::Vector2f& startPos, VehicleType type, Direction dir)
    : type(type), direction(dir), textureLoaded(false), passedStopLine(false), stoppedTime(0.f) // initialize false

{
    lastPosition = startPos;
    // Choose a texture file
    std::string fileName = "C:/TrafficLightSimulation/assets/car.png";
    switch (type) {
        case VehicleType::Normal:    fileName = "C:/TrafficLightSimulation/assets/car.png"; break;
        case VehicleType::Taxi:      fileName = "C:/TrafficLightSimulation/assets/taxi.png"; break;
        case VehicleType::Ambulance: fileName = "C:/TrafficLightSimulation/assets/ambulance.png"; break;
        case VehicleType::Audi:      fileName = "C:/TrafficLightSimulation/assets/Audi.png"; break;
        case VehicleType::Truck:     fileName = "C:/TrafficLightSimulation/assets/mini_truck.png"; break;
        case VehicleType::Bus:       fileName = "C:/TrafficLightSimulation/assets/police.png"; break;
        case VehicleType::BlackViper:fileName = "C:/TrafficLightSimulation/assets/black_viper.png"; break;
        case VehicleType::BigTruck:  fileName = "C:/TrafficLightSimulation/assets/truck.png"; break;
    }

    if (texture.loadFromFile(fileName)) {
        textureLoaded = true;
        sprite.setTexture(texture);
        // Scale down if needed
        sprite.setScale(0.3f, 0.3f);

        // Center origin
        sf::Vector2u texSize = texture.getSize();
        sprite.setOrigin(texSize.x * 0.5f, texSize.y * 0.5f);

        // Position
        sprite.setPosition(startPos);

        // Rotate based on direction
        switch (dir) {
            case Direction::LeftToRight:
                // No rotation needed if your car.png faces right by default
                sprite.setRotation(0.f);
                break;
            case Direction::RightToLeft:
                // 180 degrees if your car.png is oriented to the right by default
                sprite.setRotation(180.f);
                break;
            case Direction::TopToBottom:
                // 90 degrees clockwise
                sprite.setRotation(90.f);
                break;
            case Direction::BottomToTop:
                // 270 degrees clockwise (or -90)
                sprite.setRotation(270.f);
                break;
        }
    } else {
        std::cerr << "Failed to load vehicle image from " << fileName << "\n";
        fallback.setSize(sf::Vector2f(40.f, 20.f));
        fallback.setOrigin(20.f, 10.f);
        fallback.setPosition(startPos);
        fallback.setFillColor(sf::Color::Blue);
    }
}

void Vehicle::update(float dt, float speed) {
    float dx = 0.f;
    float dy = 0.f;

    // Decide direction of movement
    switch (direction) {
        case Direction::LeftToRight:  dx = speed; break;
        case Direction::RightToLeft:  dx = -speed; break;
        case Direction::TopToBottom:  dy = speed; break;
        case Direction::BottomToTop:  dy = -speed; break;
    }

    dx *= dt;
    dy *= dt;

    if (textureLoaded) {
        sprite.move(dx, dy);
    } else {
        fallback.move(dx, dy);
    }
    
    // Compute actual displacement from last frame.
    sf::Vector2f currentPos = textureLoaded ? sprite.getPosition() : fallback.getPosition();
    float displacement = std::sqrt(std::pow(currentPos.x - lastPosition.x, 2) +
                                   std::pow(currentPos.y - lastPosition.y, 2));
    const float epsilon = 0.1f; 

    // Use displacement to update stoppedTime
    if (displacement < epsilon) {
        stoppedTime += dt;
    } else {
        stoppedTime = 0.f;
    }

    // Update lastPosition.
    lastPosition = currentPos;
}

void Vehicle::render(sf::RenderWindow& window) {
    if (textureLoaded) {
        window.draw(sprite);
    } else {
        window.draw(fallback);
    }
}

VehicleType Vehicle::getType() const {
    return type;
}

float Vehicle::getX() const {
    // For sorting horizontally
    if (textureLoaded) {
        return sprite.getPosition().x;
    } else {
        return fallback.getPosition().x;
    }
}

float Vehicle::getY() const {
    // For sorting vertically
    if (textureLoaded) {
        return sprite.getPosition().y;
    } else {
        return fallback.getPosition().y;
    }
}

Direction Vehicle::getDirection() const {
    return direction;
}

bool Vehicle::hasPassedStopLine() const {
    return passedStopLine;
}

void Vehicle::setPassedStopLine(bool val) {
    passedStopLine = val;
}




