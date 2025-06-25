# Smart Traffic Light Simulation with Reinforcement Learning

## Overview
Smart Traffic Light Simulation is a real-time traffic simulation featuring adaptive traffic lights controlled by a reinforcement learning (RL) algorithm. It models an intelligent 4-way intersection where vehicles dynamically interact with the traffic signals, and queue-based optimization is applied to improve traffic flow.

## Simulation Video
▶️ [Traffic Simulation](https://drive.google.com/file/d/1M1ShuI9tzs_RBVAZBA6lCJpm3eJRKxNf/view?usp=sharing)

## Features
- **Realistic Traffic Simulation:** Supports various vehicle types (cars, trucks, ambulances, taxis, etc.).
- **Adaptive Traffic Light System:** Optimizes green light duration based on real-time congestion levels.
- **SFML-based Graphics:** Provides smooth real-time rendering of the intersection.
- **Reinforcement Learning Integration:** Utilizes Q-learning to adjust light timings dynamically, reducing vehicle wait times and congestion.
- **Multi-Phase Traffic Light Management:** Manages North-South and East-West green/yellow transitions.
- **Vehicle Spawn Control:** Includes an on-screen button to cycle through preset vehicle spawn intervals.
- **Customizable Q-Table Storage:** Saves and loads the RL policy for persistent learning across simulations.

## Technologies Used
- **C++ & SFML:** Core simulation logic and graphical interface.
- **Python:** For training the RL model.
- **JSON:** For storing and loading the Q-table.
- **MinGW/MSYS2 UCRT64:** Compiler toolchain used for building the project.

## Dependencies
- **SFML 2.6.1** – Graphics, window, and system modules.
- **MinGW/MSYS2 UCRT64** – GCC toolchain (GCC 14.2.0 or later recommended).
- **nlohmann/json** – JSON library (via `json.hpp`) for Q-table handling.
- **Python 3.x** – (with NumPy) for training the RL model.

## Build and Run Instructions

### Using VS Code
1. **Configure Build Tasks:**  
   In your `tasks.json`, set the compiler path to:
   ```json
   "command": "C:/msys64/ucrt64/bin/g++.exe",
   ```
   Ensure your include paths point to `C:/msys64/ucrt64/include` and your project’s `include/` folder, and that library paths point to `C:/msys64/ucrt64/lib`.

2. **Build the Project:**  
   Press `Ctrl+Shift+B` to run the build task.  
   The executable (e.g., `SFMLTest.exe`) will be generated in your `bin/` folder.

3. **Run the Simulation:**  
   In the integrated terminal, execute:
   ```sh
   .\bin\SFMLTest.exe
   ```
   (On Windows, use `.\` to run executables from the current directory.)

### Using Command Line (MSYS2 UCRT64 Shell)
1. **Navigate to Your Project Directory:**  
   ```sh
   cd /c/TrafficLightSimulation
   ```
2. **Clean Previous Builds (if needed):**  
   ```sh
   rm -rf bin/*
   rm -rf build/
   rm -rf CMakeCache.txt CMakeFiles/
   ```
3. **Compile the Project:**  
   ```sh
   C:/msys64/ucrt64/bin/g++.exe -std=c++17 -g main.cpp TrafficLight.cpp Vehicle.cpp TrafficManager.cpp QTableLoader.cpp -I include -I C:/msys64/ucrt64/include -L C:/msys64/ucrt64/lib -lsfml-graphics -lsfml-window -lsfml-system -o bin/SFMLTest.exe
   ```
4. **Run the Executable:**  
   ```sh
   ./bin/SFMLTest.exe
   ```

## Training the RL Agent
1. **Navigate to the RL Directory:**  
   ```sh
   cd RL
   ```
2. **Run the Training Script:**  
   ```sh
   pip install numpy
   python traffic_rl.py
   ```
   This will train the RL agent and update `q_table.json`, which is loaded by the simulation.

## How It Works
### Traffic Lights
The `TrafficLight` class handles the cycling of red, yellow, and green phases based on timers and pre-loaded textures.

### Vehicles
The `Vehicle` class supports various vehicle types and manages their movement and stop-line logic.

### Traffic Management & RL Integration
The `TrafficManager` class spawns vehicles, measures queues, and uses a Q-table (loaded from `q_table.json`) to adapt the green light duration dynamically based on real-time traffic conditions.

### User Interaction
An on-screen button allows users to cycle through preset vehicle spawn intervals (1.0f, 3.0f, 0.5f) to simulate different traffic densities.

### Reinforcement Learning
The RL component is trained in Python using Q-learning to optimize traffic light timings based on a simulated environment, and the resulting Q-table is saved as `q_table.json`. The C++ simulation loads this Q-table at runtime and uses it to dynamically adjust green light durations in response to real-time traffic conditions.




