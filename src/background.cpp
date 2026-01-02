// filepath: d:\myPortfolioProject\Neon_Swarm_game\src\background.cpp
#include "background.h"
#include "raymath.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

Background InitBackground(int screenWidth, int screenHeight) {
    Background bg = {0};
    bg.position = (Vector2){0, 0};
    bg.scrollSpeed = 0.3f; //
    bg.screenWidth = screenWidth;
    bg.screenHeight = screenHeight;

    LoadTileset(&bg, "assets/txtures/mid_evil/PNG/Default size/Tile"); //

    // Use a blank floor (Tile 14) as the default design base
    if (!FileExists("assets/maps/map.dat")) {
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                bg.map[y][x] = 14;  // medievalTile_14.png (Dirt/Floor)
                bg.collisionMap[y][x] = false;
            }
        }
        TraceLog(LOG_INFO, "Empty design canvas created!");
    } else {
        LoadMap(&bg, "assets/maps/map.dat"); //
    }
    return bg;
}

void LoadTileset(Background* background, const char* tileFolder) {
    for (int i = 0; i < 60; i++) {
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/medievalTile_%02d.png", tileFolder, i);
        background->tileSet[i] = LoadTexture(filepath);
        
        if (background->tileSet[i].id == 0) {
            TraceLog(LOG_WARNING, "Failed to load tile %d from %s", i, filepath);
            // Create a fallback colored tile
            unsigned char colorValue = (unsigned char)(50 + i * 4);
            Image fallback = GenImageColor(TILE_SIZE, TILE_SIZE, (Color){colorValue, 100, 50, 255});
            background->tileSet[i] = LoadTextureFromImage(fallback);
            UnloadImage(fallback);
        }
    }
    TraceLog(LOG_INFO, "Tileset loading complete! Loaded 60 medieval tiles.");
}

void GenerateRandomMap(Background* background) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            background->map[y][x] = rand() % 6;
            background->collisionMap[y][x] = false;  // No collision by default
        }
    }
    TraceLog(LOG_INFO, "Random map generated!");
}

void SetTile(Background* background, int x, int y, int tileIndex) {
    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
        if (tileIndex >= 0 && tileIndex < 60) {
            background->map[y][x] = tileIndex;
        }
    }
}

void SetCollision(Background* background, int x, int y, bool isBlocked) {
    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
        background->collisionMap[y][x] = isBlocked;
    }
}

bool GetCollision(Background* background, int x, int y) {
    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
        return background->collisionMap[y][x];
    }
    return false;
}

void UpdateBackground(Background* background, Vector2 cameraTarget) {
    background->position.x = cameraTarget.x * background->scrollSpeed;
    background->position.y = cameraTarget.y * background->scrollSpeed;
}

void DrawBackground(Background* background) {
    // 1. Calculate how many tiles we need to cover the screen plus a safety buffer
    int tilesNeededX = (background->screenWidth / TILE_SIZE) + 2; 
    int tilesNeededY = (background->screenHeight / TILE_SIZE) + 2;
    
    // 2. Determine the first tile index to draw (floor the value to prevent clipping)
    int startTileX = (int)floor(background->position.x / TILE_SIZE);
    int startTileY = (int)floor(background->position.y / TILE_SIZE);
    
    // 3. Draw visible tiles with floating-point precision for positions
    for (int y = startTileY; y < startTileY + tilesNeededY; y++) {
        for (int x = startTileX; x < startTileX + tilesNeededX; x++) {
            // Safety bounds check for the 50x50 map array
            if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
                int tileIndex = background->map[y][x];
                
                if (tileIndex >= 0 && tileIndex < 60 && background->tileSet[tileIndex].id != 0) {
                    // Calculate precise screen position by subtracting camera offset
                    Vector2 screenPos = {
                        (float)(x * TILE_SIZE) - background->position.x,
                        (float)(y * TILE_SIZE) - background->position.y
                    };
                    DrawTextureV(background->tileSet[tileIndex], screenPos, WHITE);
                }
            }
        }
    }
}

Vector2 ScreenToWorldCoords(Vector2 screenPos, Vector2 bgPos) {
    return Vector2Add(screenPos, bgPos);
}

void GetTileCoords(Vector2 worldPos, int* tileX, int* tileY) {
    *tileX = (int)(worldPos.x / TILE_SIZE);
    *tileY = (int)(worldPos.y / TILE_SIZE);
}

bool IsTileWalkable(Background* background, Vector2 worldPos) {
    int tileX, tileY;
    GetTileCoords(worldPos, &tileX, &tileY);
    return !GetCollision(background, tileX, tileY);
}

void SaveMap(Background* background, const char* filename) {
    // Create the maps directory if it doesn't exist
    if (!DirectoryExists("assets/maps")) {
        char cmdBuffer[256];
        snprintf(cmdBuffer, sizeof(cmdBuffer), "mkdir assets\\maps");
        system(cmdBuffer);
    }
    
    // Write the map data AND collision data to file
    unsigned char mapData[MAP_HEIGHT * MAP_WIDTH * 2];  // 2 bytes per tile (tile index + collision)
    int dataIndex = 0;
    
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            mapData[dataIndex++] = (unsigned char)background->map[y][x];
            mapData[dataIndex++] = (unsigned char)(background->collisionMap[y][x] ? 1 : 0);
        }
    }
    
    SaveFileData(filename, mapData, dataIndex);
    TraceLog(LOG_INFO, "Map saved to %s (including collision data)", filename);
}

void LoadMap(Background* background, const char* filename) {
    if (!FileExists(filename)) {
        TraceLog(LOG_WARNING, "Map file not found: %s", filename);
        GenerateRandomMap(background);
        return;
    }
    
    int bytesRead = 0;
    unsigned char* mapData = LoadFileData(filename, &bytesRead);
    
    // Expected size: MAP_HEIGHT * MAP_WIDTH * 2 (tile index + collision)
    int expectedSize = MAP_HEIGHT * MAP_WIDTH * 2;
    
    if (bytesRead == expectedSize) {
        int dataIndex = 0;
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                background->map[y][x] = (int)mapData[dataIndex++];
                background->collisionMap[y][x] = (mapData[dataIndex++] != 0);
            }
        }
        TraceLog(LOG_INFO, "Map loaded from %s (with collision data)", filename);
    } else {
        TraceLog(LOG_WARNING, "Map file corrupted or invalid size. Expected %d bytes, got %d. Generating random map.", 
                expectedSize, bytesRead);
        GenerateRandomMap(background);
    }
    
    UnloadFileData(mapData);
}

void UnloadBackground(Background* background) {
    for (int i = 0; i < 60; i++) {
        if (background->tileSet[i].id != 0) {
            UnloadTexture(background->tileSet[i]);
        }
    }
    TraceLog(LOG_INFO, "Background tileset unloaded!");
}