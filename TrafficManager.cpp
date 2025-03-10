#include "TrafficManager.hpp"
#include "json.hpp"          // nlohmann::json header
#include "QTableLoader.hpp"  
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <climits>
#include <cmath>
#include <unordered_map>

// Utility: Convert a state (pair) to a string that matches the JSON Q‑table keys.
std::string stateToString(const std::pair<int, int>& state) {
    std::ostringstream oss;
    oss << "(" << state.first << ", " << state.second << ")";
    return oss.str();
}

TrafficManager::TrafficManager()
    : topLeftLight(sf::Vector2f(310.f, 160.f)),
      topRightLight(sf::Vector2f(600.f, 160.f)),
      bottomLeftLight(sf::Vector2f(310.f, 440.f)),
      bottomRightLight(sf::Vector2f(600.f, 440.f)),
      phase(Phase::NS_Green),
      phaseTimer(0.f),
      greenTime(5.f),
      yellowTime(2.f),
      currentGreenTime(5.f),
      minGreen(3.f),
      maxGreen(10.f),
      queueNS(0),
      queueEW(0),
      spawnTimer(0.f),
      spawnInterval(1.f),
      vehicleSpeed(120.f)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    // Load the Q‑table using our QTableLoader (returns a map with string keys)
    qTable = QTableLoader::loadQTable("q_table.json");

    // Logical grouping: NS group starts green, EW group red.
    topLeftLight.setState(LightState::Green);
    bottomLeftLight.setState(LightState::Green);
    topRightLight.setState(LightState::Red);
    bottomRightLight.setState(LightState::Red);
}

TrafficManager::~TrafficManager() {
    for (auto v : vehicles) {
        delete v;
    }
}

void TrafficManager::applyRLDecision(const std::pair<int, int>& stateKey, const char* phaseLabel, int prevQueueNS, int prevQueueEW) {
    std::string keyStr = stateToString(stateKey);
    int action = 0;  // Default action: 0 = no change

    // Try to look up the state in the Q-table.
    auto it = qTable.find(keyStr);
    if (it != qTable.end()) {
        const std::vector<double>& qValues = it->second;
        action = std::distance(qValues.begin(), std::max_element(qValues.begin(), qValues.end()));
        std::cout << "RL Decision (" << phaseLabel << ") for state " << keyStr 
                  << ": Action = " << action << std::endl;
    } else {
        std::cout << "No RL Q-values found for state " << keyStr << std::endl;
        action = (std::rand() % 2) + 1;  // Explore between action 1 and 2
        std::cout << "[DEBUG] Choosing random action: " << action << std::endl;
    }

    // --- Exponential Moving Average (EMA) for queue trends (Faster Adaptation)
    static float emaQueueNS = static_cast<float>(prevQueueNS);
    static float emaQueueEW = static_cast<float>(prevQueueEW);
    float emaAlpha = 0.8f;  // Increased to make smoothing more responsive
    emaQueueNS = emaAlpha * queueNS + (1 - emaAlpha) * emaQueueNS;
    emaQueueEW = emaAlpha * queueEW + (1 - emaAlpha) * emaQueueEW;
    std::pair<int, int> smoothedState = { 
        static_cast<int>(std::round(emaQueueNS)),
        static_cast<int>(std::round(emaQueueEW))
    };
    std::cout << "[DEBUG] Smoothed state: " << stateToString(smoothedState) << std::endl;

    // --- Adjust Reward Scaling.
    int queueReduction = (prevQueueNS + prevQueueEW) - (queueNS + queueEW);
    double reward = (queueReduction > 0) ? 5.0 * std::pow(queueReduction, 1.5) : -1.5;
    std::cout << "[DEBUG] Reward computed (Adaptive Scaling): " << reward << std::endl;

    // --- Set max green time based on congestion level
    float maxGreenTime = 5.f; // Default base max
    if (queueNS >= 9 || queueEW >= 9) {  
        std::cout << "[DEBUG] Extreme congestion detected; reinforcing max green time to 8 sec." << std::endl;
        maxGreenTime = 8.f;
    } else if (queueNS >= 6 || queueEW >= 6) {  
        std::cout << "[DEBUG] High congestion detected; capping max green time at 6 sec." << std::endl;
        maxGreenTime = 6.f;
    }

    // --- Now update currentGreenTime so it fully respects the new maxGreenTime.
    if (currentGreenTime < maxGreenTime) {
        std::cout << "[DEBUG] Increasing green time to match new max limit." << std::endl;
        currentGreenTime = maxGreenTime;
    } else {
        currentGreenTime = std::clamp(currentGreenTime, minGreen, maxGreenTime);
    }

    // --- Override Action 0 Only If Congestion is Increasing (AFTER setting maxGreenTime)
    if ((queueNS >= 6 || queueEW >= 6) && action == 0) {
        if (queueNS > prevQueueNS || queueEW > prevQueueEW) {  // Override only if congestion is rising
            std::cout << "[DEBUG] High congestion worsening; forcing non-zero action." << std::endl;
            action = (std::rand() % 2) + 1;
            std::cout << "[DEBUG] Overriding RL action to: " << action << std::endl;
        }
    }

    // --- Immediate adjustment if the spawn interval was changed by the user.
    if (spawnIntervalChanged) {
        std::cout << "[DEBUG] User changed congestion settings, forcing immediate green time update." << std::endl;
        if (queueNS >= 9 || queueEW >= 9) {
            maxGreenTime = 8.f;
        } else if (queueNS >= 6 || queueEW >= 6) {
            maxGreenTime = 6.f;
        }
        currentGreenTime = maxGreenTime; // Immediately apply new max limit
        spawnIntervalChanged = false;    // Reset the flag
    }

    // --- Mid-phase congestion adaptation for real-time response.
    if (currentGreenTime < maxGreenTime) {
        std::cout << "[DEBUG] Adjusting green time dynamically mid-phase!" << std::endl;
        currentGreenTime = std::min(currentGreenTime + 1, maxGreenTime);
    }

    // --- Dynamic Green Time Adjustments based on congestion.
    if (queueNS >= 6 || queueEW >= 6) { 
        std::cout << "[DEBUG] High congestion detected; increasing green time." << std::endl;
        currentGreenTime = std::min(currentGreenTime + 1, maxGreenTime);
    } else if (queueNS < 3 && queueEW < 3) { 
        std::cout << "[DEBUG] Low traffic detected; decreasing green time." << std::endl;
        currentGreenTime = std::max(currentGreenTime - 1, minGreen + 1); // Avoid reducing too fast
    }

    // --- Apply the RL-based green time adjustment.
    if (action == 1) {
        currentGreenTime += 1;
    } else if (action == 2) {
        currentGreenTime -= 1;
    }

    // --- Final clamp of currentGreenTime.
    currentGreenTime = std::clamp(currentGreenTime, minGreen, maxGreenTime);

    std::cout << "[DEBUG] " << phaseLabel << " phase - New green time set to: " 
              << currentGreenTime << std::endl;
}


void TrafficManager::setSpawnInterval(float newInterval) { 
    if (newInterval != spawnInterval) { // Only trigger if the value is actually different
        spawnInterval = newInterval;
        spawnIntervalChanged = true;  // Mark that it has changed
    }
}


void TrafficManager::updateLights(float dt) {
    phaseTimer += dt;
    switch (phase) {
        case Phase::NS_Green:
            topLeftLight.setState(LightState::Green);
            bottomLeftLight.setState(LightState::Green);
            topRightLight.setState(LightState::Red);
            bottomRightLight.setState(LightState::Red);
            if (phaseTimer >= currentGreenTime) {
                phase = Phase::NS_Yellow;
                phaseTimer = 0.f;
            }
            break;
        case Phase::NS_Yellow:
        {
            topLeftLight.setState(LightState::Yellow);
            bottomLeftLight.setState(LightState::Yellow);
            topRightLight.setState(LightState::Red);
            bottomRightLight.setState(LightState::Red);
            if (phaseTimer >= yellowTime) {
                int prevQueueNS = queueNS;
                int prevQueueEW = queueEW;
                measureQueues();
                std::pair<int,int> stateKey = {queueNS, queueEW};
                std::cout << "[DEBUG] NS_Yellow phase - QueueNS: " << queueNS
                          << ", QueueEW: " << queueEW << std::endl;
                applyRLDecision(stateKey, "NS_Yellow", prevQueueNS, prevQueueEW);
                phase = Phase::EW_Green;
                phaseTimer = 0.f;
            }
            break;
        }
        case Phase::EW_Green:
            topLeftLight.setState(LightState::Red);
            bottomLeftLight.setState(LightState::Red);
            topRightLight.setState(LightState::Green);
            bottomRightLight.setState(LightState::Green);
            if (phaseTimer >= currentGreenTime) {
                phase = Phase::EW_Yellow;
                phaseTimer = 0.f;
            }
            break;
        case Phase::EW_Yellow:
        {
            topLeftLight.setState(LightState::Red);
            bottomLeftLight.setState(LightState::Red);
            topRightLight.setState(LightState::Yellow);
            bottomRightLight.setState(LightState::Yellow);
            if (phaseTimer >= yellowTime) {
                int prevQueueNS = queueNS;
                int prevQueueEW = queueEW;
                measureQueues();
                std::pair<int,int> stateKey = {queueNS, queueEW};
                std::cout << "[DEBUG] EW_Yellow phase - QueueNS: " << queueNS
                          << ", QueueEW: " << queueEW << std::endl;
                applyRLDecision(stateKey, "EW_Yellow", prevQueueNS, prevQueueEW);
                phase = Phase::NS_Green;
                phaseTimer = 0.f;
            }
            break;
        }
    }
    topLeftLight.update(dt);
    topRightLight.update(dt);
    bottomLeftLight.update(dt);
    bottomRightLight.update(dt);
}

// Returns true if the vehicle's speed is below the threshold (i.e., it is effectively stopped)
bool isVehicleQueuedBySpeed(Vehicle* v) {
    const float SPEED_THRESHOLD = 5.f; // Adjust this value based on your simulation units
    return (v->speed < SPEED_THRESHOLD);
}

void TrafficManager::measureQueues() {
    queueNS = 0;
    queueEW = 0;

    // Separate vehicles by direction
    std::vector<Vehicle*> topVec, bottomVec, leftVec, rightVec;
    for (auto* v : vehicles) {
        switch (v->getDirection()) {
            case Direction::TopToBottom:   topVec.push_back(v);    break;
            case Direction::BottomToTop:   bottomVec.push_back(v); break;
            case Direction::LeftToRight:   leftVec.push_back(v);   break;
            case Direction::RightToLeft:   rightVec.push_back(v);  break;
        }
    }

    // Sort them by position so we can iterate from front to back
    std::sort(topVec.begin(), topVec.end(),   [](auto* a, auto* b){return a->getY() < b->getY();});
    std::sort(bottomVec.begin(), bottomVec.end(),[](auto* a, auto* b){return a->getY() > b->getY();});
    std::sort(leftVec.begin(), leftVec.end(), [](auto* a, auto* b){return a->getX() < b->getX();});
    std::sort(rightVec.begin(), rightVec.end(),[](auto* a, auto* b){return a->getX() > b->getX();});

    // Helper to process each lane
    auto processLane = [&](std::vector<Vehicle*>& lane, bool isNS) {
        bool frontIsStopped = false;
        for (size_t i = 0; i < lane.size(); ++i) {
            Vehicle* v = lane[i];
            if (shouldStopVehicle(v) || isVehicleQueuedBySpeed(v) || frontIsStopped) {
                if (isNS) queueNS++;
                else      queueEW++;
                frontIsStopped = true;
            } else {
                frontIsStopped = false;
            }
        }
    };

    processLane(topVec, true);
    processLane(bottomVec, true);
    processLane(leftVec, false);
    processLane(rightVec, false);
}

void TrafficManager::spawnVehicle() {
    int approach = std::rand() % 4;
    VehicleType t = VehicleType::Normal;
    int r = std::rand() % 8;
    switch (r) {
        case 0: t = VehicleType::Normal; break;
        case 1: t = VehicleType::Taxi; break;
        case 2: t = VehicleType::Ambulance; break;
        case 3: t = VehicleType::Audi; break;
        case 4: t = VehicleType::Truck; break;
        case 5: t = VehicleType::Bus; break;
        case 6: t = VehicleType::BlackViper; break;
        case 7: t = VehicleType::BigTruck; break;
    }
    Vehicle* v = nullptr;
    if (approach == 0) {
        v = new Vehicle(sf::Vector2f(390.f, -50.f), t, Direction::TopToBottom);
    } else if (approach == 1) {
        v = new Vehicle(sf::Vector2f(500.f, 650.f), t, Direction::BottomToTop);
    } else if (approach == 2) {
        v = new Vehicle(sf::Vector2f(-50.f, 250.f), t, Direction::LeftToRight);
    } else {
        v = new Vehicle(sf::Vector2f(950.f, 350.f), t, Direction::RightToLeft);
    }
    vehicles.push_back(v);
}

bool TrafficManager::shouldStopVehicle(Vehicle* v)
{
    // If the vehicle has been stopped for at least 2 seconds, count it as queued
    // (this helps catch vehicles that remain stopped even after crossing the line).
    if (v->stoppedTime >= 2.0f) {
        return true;
    }

    float x = v->getX();
    float y = v->getY();
    Direction d = v->getDirection();

    // If the vehicle was already marked as having passed the stop line
    // AND it's not forced to stop, we won't count it as queued anymore.
    // (Because typically it has cleared or is clearing the intersection.)
    if (v->hasPassedStopLine()) {
        return false;
    }

    // Otherwise, check each approach with your existing thresholds:
    if (d == Direction::TopToBottom) {
        // Once y > 150.f, the vehicle is at/near the intersection
        if (y > 150.f) {
            LightState s = topLeftLight.getState();
            // If red or yellow, the vehicle must stop => still queued
            if (s == LightState::Red || s == LightState::Yellow) {
                return true;
            } else {
                // Green => let it pass, mark as passedStopLine
                v->setPassedStopLine(true);
                return false;
            }
        }
    }
    else if (d == Direction::BottomToTop) {
        if (y < 450.f) {
            LightState s = bottomLeftLight.getState();
            if (s == LightState::Red || s == LightState::Yellow) {
                return true;
            } else {
                v->setPassedStopLine(true);
                return false;
            }
        }
    }
    else if (d == Direction::LeftToRight) {
        if (x > 300.f) {
            LightState s = topRightLight.getState();
            if (s == LightState::Red || s == LightState::Yellow) {
                return true;
            } else {
                v->setPassedStopLine(true);
                return false;
            }
        }
    }
    else { // RightToLeft
        if (x < 605.f) {
            LightState s = bottomRightLight.getState();
            if (s == LightState::Red || s == LightState::Yellow) {
                return true;
            } else {
                v->setPassedStopLine(true);
                return false;
            }
        }
    }

    // If none of the above conditions apply (vehicle not near intersection), it's not queued
    return false;
}

void TrafficManager::update(float dt) {
    updateLights(dt);
    spawnTimer += dt;
    if (spawnTimer >= spawnInterval) {
        spawnVehicle();
        spawnTimer = 0.f;
    }
    float minDistance = 80.f;
    std::vector<Vehicle*> topVec, bottomVec, leftVec, rightVec;
    for (auto* veh : vehicles) {
        switch (veh->getDirection()) {
            case Direction::TopToBottom:   topVec.push_back(veh); break;
            case Direction::BottomToTop:   bottomVec.push_back(veh); break;
            case Direction::LeftToRight:   leftVec.push_back(veh); break;
            case Direction::RightToLeft:   rightVec.push_back(veh); break;
        }
    }
    std::sort(topVec.begin(), topVec.end(), [](Vehicle* a, Vehicle* b) {
        return a->getY() < b->getY();
    });
    std::sort(bottomVec.begin(), bottomVec.end(), [](Vehicle* a, Vehicle* b) {
        return a->getY() > b->getY();
    });
    std::sort(leftVec.begin(), leftVec.end(), [](Vehicle* a, Vehicle* b) {
        return a->getX() < b->getX();
    });
    std::sort(rightVec.begin(), rightVec.end(), [](Vehicle* a, Vehicle* b) {
        return a->getX() > b->getX();
    });
    auto updateGroup = [&](std::vector<Vehicle*>& group, bool vertical) {
        for (size_t i = 0; i < group.size(); ++i) {
            Vehicle* current = group[i];
            bool canMove = true;
            if (shouldStopVehicle(current))
                canMove = false;
            if (i + 1 < group.size()) {
                Vehicle* front = group[i + 1];
                float dist = vertical ? (front->getY() - current->getY())
                                      : (front->getX() - current->getX());
                if (dist < 0) dist = -dist;
                if (dist < minDistance)
                    canMove = false;
            }
            if (canMove)
                current->update(dt, vehicleSpeed);
        }
    };
    updateGroup(topVec, true);
    updateGroup(bottomVec, true);
    updateGroup(leftVec, false);
    updateGroup(rightVec, false);
    vehicles.erase(
        std::remove_if(vehicles.begin(), vehicles.end(), [&](Vehicle* v) {
            float xx = v->getX();
            float yy = v->getY();
            if (xx < -50.f || xx > 950.f || yy < -50.f || yy > 650.f) {
                delete v;
                return true;
            }
            return false;
        }),
        vehicles.end()
    );
}

void TrafficManager::render(sf::RenderWindow& window) {
    topLeftLight.render(window);
    topRightLight.render(window);
    bottomLeftLight.render(window);
    bottomRightLight.render(window);
    for (auto* v : vehicles)
        v->render(window);
}

size_t TrafficManager::getVehicleCount() const {
    return vehicles.size();
}

