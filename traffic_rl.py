import numpy as np
import random
import json
import os

class TrafficEnv:
    """
    A simple traffic environment.
    State: (queueNS, queueEW) - number of vehicles waiting on North-South and East-West.
    Action:
      0: Decrease green time by 1 second.
      1: No change.
      2: Increase green time by 1 second.
    Reward:
      Negative total queue length (we want to minimize congestion).
    """
    def __init__(self):
        self.base_green = 5  # Base green time (seconds)
        self.reset()

    def reset(self):
        self.queueNS = np.random.randint(0, 20)  # Capped at 20 for more realistic values
        self.queueEW = np.random.randint(0, 20)
        self.green_time = self.base_green
        return (self.queueNS, self.queueEW)

    def step(self, action):
        # Action: 0 -> decrease, 1 -> no change, 2 -> increase green time
        adjustment = action - 1  # Maps (0,1,2) -> (-1,0,+1)
        self.green_time = max(3, min(10, self.base_green + adjustment))  # Ensures reasonable green times

        # Simulate effect: NS green reduces NS queue; EW queue increases slightly
        new_queueNS = max(0, self.queueNS - int(self.green_time))
        new_queueEW = min(20, self.queueEW + np.random.randint(0, 4))  # Capped at 20
        reward = - (new_queueNS + new_queueEW)  # Minimize total congestion
        self.queueNS, self.queueEW = new_queueNS, new_queueEW
        next_state = (self.queueNS, self.queueEW)
        done = False  # Continuous environment
        return next_state, reward, done, {}

class QLearningAgent:
    def __init__(self, action_space_size, alpha=0.015, gamma=0.97, epsilon=0.1):
        self.alpha = alpha      # Learning rate (Lowered for stability)
        self.gamma = gamma      # Discount factor (Increased for long-term optimization)
        self.epsilon = epsilon  # Exploration rate
        self.action_space_size = action_space_size
        self.q_table = {}       # Dictionary: state (tuple) -> list of Q-values

    def get_q_values(self, state):
        if state not in self.q_table:
            self.q_table[state] = [0.0 for _ in range(self.action_space_size)]
        return self.q_table[state]

    def choose_action(self, state):
        if random.random() < self.epsilon:
            return random.randrange(self.action_space_size)
        q_values = self.get_q_values(state)
        return np.argmax(q_values)

    def update(self, state, action, reward, next_state):
        current_q = self.get_q_values(state)[action]
        next_max = max(self.get_q_values(next_state))
        new_q = current_q + self.alpha * (reward + self.gamma * next_max - current_q)
        self.q_table[state][action] = new_q

def save_q_table(q_table, filename="q_table.json"):
    """
    Saves the Q-table to a JSON file.
    Converts tuple keys to strings for JSON compatibility.
    """
    serializable_table = {str(key): value for key, value in q_table.items()}
    with open(filename, "w") as f:
        json.dump(serializable_table, f)

def load_q_table(filename="q_table.json"):
    """
    Loads the Q-table from a JSON file.
    Converts string keys back to tuple keys.
    """
    if os.path.exists(filename):
        with open(filename, "r") as f:
            serializable_table = json.load(f)
        return {eval(key): value for key, value in serializable_table.items()}
    return {}

if __name__ == '__main__':
    env = TrafficEnv()
    agent = QLearningAgent(action_space_size=3, alpha=0.015, gamma=0.97, epsilon=0.1)
    
    # Load existing Q-table if available
    agent.q_table = load_q_table("q_table.json")
    
    episodes = 20000   # Increased to 20,000 episodes for deeper learning
    steps_per_episode = 75  # Increased to improve policy refinement

    for ep in range(episodes):
        state = env.reset()
        total_reward = 0
        
        # Gradual epsilon decay
        agent.epsilon = max(0.02, 0.1 * (0.9997 ** ep))
        
        for t in range(steps_per_episode):
            action = agent.choose_action(state)
            next_state, reward, done, _ = env.step(action)
            agent.update(state, action, reward, next_state)
            state = next_state
            total_reward += reward
            if done:
                break
        
        if ep % 100 == 0:
            print(f"Episode {ep} Total Reward: {total_reward}")

    print("Training complete.")
    print("Learned Q-Table:")
    for s, q in list(agent.q_table.items())[:10]:
        print(f"State {s}: {q}")
    
    # Save the Q-table for later use (e.g., integration with your C++ simulation)
    save_q_table(agent.q_table, "q_table.json")
