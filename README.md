# DuckFishing

blog links:  
1. [Client-Server Game Communication](https://rubaduckduck.github.io/2025/01/20/networking.html)
2. [Discrete Parallel Transport](https://rubaduckduck.github.io/2025/01/10/discrete_non_euc.html)

## Project Overview

DuckFishing is a multiplayer game project exploring collaborative gameplay through a unique networking and rendering architecture. The project demonstrates advanced game development techniques, focusing on flexible game state management and real-time multiplayer interactions.

## Project Structure

### Key Components

- **Core System**: Fundamental game mechanics and infrastructure
- **Networking**: Robust client-server communication
- **Game Modes**: Different gameplay states and contexts
- **Rendering**: Graphics and visualization system

### Main Directories

```
DuckFishing/
│
├── include/           # Header files
│   ├── Core/          # Game core definitions
│   ├── GameModes/     # Different game state modes
│   ├── Network/       # Networking architecture
│   └── Rendering/     # Graphics system
│
├── src/               # Implementation files
│   ├── Core/
│   ├── GameModes/
│   ├── Network/
│   └── Rendering/
│
└── shaders/           # OpenGL shader files
```

## Technical Highlights

- Dual-protocol networking (TCP/UDP)
- Flexible game state management
- Dynamic game mode switching
- OpenGL-based rendering
- Modular system design

## Development Environment

- Language: C++
- IDE: Visual Studio
- Libraries:
  - SDL2
  - GLAD (OpenGL loading)
  - Assimp (Asset importing)

## Getting Started

### Prerequisites

- Visual Studio
- SDL2
- OpenGL development libraries

### Build Instructions

1. Clone the repository
2. Open `duckfishing.sln` in Visual Studio
3. Restore NuGet packages
4. Build the solution

## Current Status

Ongoing development of a multiplayer game framework with focus on:
- Networking architecture
- Game state synchronization

## Future Development

- Recursive interaction of Gameobjects riding other Gameobjects
- Refine the overall game mechanics
