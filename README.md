# Neon Swarm Game

A top-down "Survivor" style arcade shooter built with C++ and raylib. Battle waves of enemies, upgrade your abilities, and survive as long as possible in a neon-themed world.

## Features

- **Top-Down Gameplay**: Classic survivor-style mechanics with pixel-perfect movement and combat
- **Enemy Variety**: Multiple enemy types including slimes with distinct behaviors
- **Dynamic Tilemap System**: 50x50 tile-based maps with collision detection and interactive tiles
- **Bullet System**: Projectile-based combat with collision handling
- **Medieval & Fantasy Aesthetics**: Multiple tileset themes including medieval and fungi environments
- **Audio Support**: Background music and sound effects for combat and events
- **Custom Shaders**: Visual effects including shader-based slime rendering
- **Game Settings**: Configurable graphics and gameplay options
- **Map Editor Friendly**: Support for saving and loading custom map designs

## Project Structure

```
src/
├── main.cpp           # Game entry point and main loop
├── game.cpp/.h        # Core game logic and state management
├── background.cpp/.h  # Tilemap rendering and collision system
├── enemy.cpp/.h       # Enemy AI and behaviors
├── bullets.cpp/.h     # Projectile system
├── menu.cpp/.h        # Menu interface
├── settings.cpp/.h    # Game settings and configuration
└── (headers)

assets/
├── textures/          # Sprite and tileset assets
│   ├── mid_evil/      # Medieval tileset
│   ├── fungy_map/     # Fungi/mushroom tileset
│   ├── milli_env/     # Millennial environment
│   ├── enemy/         # Enemy sprites
│   └── slime/         # Slime-specific assets
├── shaders/           # GLSL shader files
├── fonts/             # Game fonts
├── maps/              # Saved map files
├── music/             # Background music
└── sounds/            # SFX

raylib-5.5/           # raylib graphics library (5.5 version)
build/                # Compiled executable and build artifacts
```

## Requirements

- **C++ Compiler**: GCC/MinGW or compatible (C++11 or later)
- **raylib 5.5**: Included in the project
- **CMake** or **Make**: For building the project
- **Windows**: Currently configured for Windows builds

## Building the Project

### Quick Build (Windows)

Run the provided batch file:

```bash
build_mingw.bat
```

This will compile the project using MinGW and create an executable in the `build/` directory.

### Build Manually

```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
make
```

Or with Visual Studio:

```bash
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## Running the Game

After building, run the executable:

```bash
./build/NeonSwarmGame.exe
```

Or from the project root:

```bash
build\NeonSwarmGame.exe
```

## Controls

- **WASD** or **Arrow Keys**: Move the player
- **Mouse**: Aim and shoot
- **ESC**: Pause / Return to menu
- **Space**: Confirm menu selections
- **Settings**: Adjust graphics and gameplay options from the menu

## Game Mechanics

### Combat System
- Point and click to shoot projectiles at enemies
- Enemies have health and deal damage on contact
- Multiple enemy types with different behaviors

### Tilemap System
- 50x50 tile grid with 64-pixel tiles
- Collision detection prevents clipping through obstacles
- Support for multiple tileset themes
- Customizable tile attributes (walkable/blocked)

### Persistence
- Game maps can be saved to disk (`assets/maps/map.dat`)
- Settings are preserved between sessions
- Map loading with validation and fallback generation

## Game Systems

### Background & Tilemap
- Dynamic tileset loading (60 tiles per theme)
- Smooth scrolling with parallax-style positioning
- Collision map system for pathfinding and collision
- Tile coordinate conversion utilities

### Enemy System
- Multiple enemy types (slimes, etc.)
- AI-driven movement and attack patterns
- Health and damage system
- Spawn and despawn management

### Bullet System
- Projectile pooling for performance
- Collision detection with enemies and environment
- Damage application on hit

### Graphics
- Multiple texture themes (medieval, fungi, survivor)
- Custom GLSL shaders for visual effects
- Sprite-based rendering with proper depth sorting

## Asset Credits

The project uses free and open-source assets:
- **Tilesets**: Itch.io community assets (see individual LICENSE.txt files in texture directories)
- **raylib**: https://www.raylib.com/ - Open-source graphics library

## Configuration

Game settings can be adjusted in `settings.cpp` or through the in-game menu:
- Screen resolution
- Vsync settings
- Graphics quality options

## Debugging

The project includes extensive logging via raylib's `TraceLog()`:
- Map loading/generation status
- Tileset initialization
- Asset loading confirmations
- Performance warnings

## Known Limitations

- Currently Windows-only (can be cross-compiled to Linux/Mac with raylib)
- Map size fixed at 50x50 tiles
- Tileset limited to 60 tiles per theme

## Future Enhancements

- [ ] Additional enemy types and behaviors
- [ ] Power-up system
- [ ] Wave progression system
- [ ] High score tracking
- [ ] Procedural map generation
- [ ] Cross-platform builds
- [ ] Difficulty settings

## License

See [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please ensure code follows the existing style and includes appropriate comments.

## Contact

For issues, suggestions, or inquiries about this project, please open an issue in the repository.
