#ifndef TRAFFICMANAGER_HPP
#define TRAFFICMANAGER_HPP

#include "TrafficLight.hpp"
#include "Vehicle.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include <utility>
#include <vector>

// We'll store the Q‑table with keys as strings.
using QTable = std::unordered_map<std::string, std::vector<double>>;

class TrafficManager {
public:
    TrafficManager();
    ~TrafficManager();

    void update(float dt);
    void render(sf::RenderWindow& window);
    void setSpawnInterval(float newInterval);
    float getSpawnInterval() const { return spawnInterval; }
    size_t getVehicleCount() const;

private:
    // Four traffic lights.
    TrafficLight topLeftLight;
    TrafficLight topRightLight;
    TrafficLight bottomLeftLight;
    TrafficLight bottomRightLight;

    // Two-phase approach.
    enum class Phase { NS_Green, NS_Yellow, EW_Green, EW_Yellow };
    Phase phase;

    // Timers & durations.
    float phaseTimer;
    float greenTime;    // Base green time.
    float yellowTime;

    // Adaptive green time.
    float currentGreenTime;
    float minGreen;
    float maxGreen;

    // Queues for NS and EW.
    int queueNS;
    int queueEW;

    // Vehicles.
    std::vector<Vehicle*> vehicles;

    // Spawning logic.
    float spawnTimer;
    float spawnInterval;
    float vehicleSpeed;

    // --- NEW: Q-table loaded from JSON.
    QTable qTable;

    void spawnVehicle();
    void updateLights(float dt);
    bool shouldStopVehicle(Vehicle* v);
    void measureQueues();
    bool spawnIntervalChanged = false;  // Track if the spawn interval was changed

    // Logs the RL decision based on the current state.
    void applyRLDecision(const std::pair<int, int>& stateKey, const char* phaseLabel, int prevQueueNS, int prevQueueEW);
};

#endif
