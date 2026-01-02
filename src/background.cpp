// filepath: d:\myPortfolioProject\Neon_Swarm_game\src\background.cpp
#include "background.h"
#include "raymath.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

Background InitBackground(int screenWidth, int screenHeight) {
    Background bg = {0};
    bg.position = (Vector2){0, 0};
    bg.scrollSpeed = 0.3f;
    bg.screenWidth = screenWidth;
    bg.screenHeight = screenHeight;

    // Initialize tile categories from different medieval asset folders
    // Each category uses the medieval prefix and occupies a specific index range
    LoadCategory(&bg, "assets/txtures/mid_evil/PNG/Default size/Tile", "medievalTile", TILE_FLOOR_START, 60);
    LoadCategory(&bg, "assets/txtures/mid_evil/PNG/Default size/Environment", "medievalEnvironment", TILE_STRUCTURE_START, 60);
    LoadCategory(&bg, "assets/txtures/mid_evil/PNG/Default size/Structure", "medievalStructure", TILE_DECO_START, 60);
    LoadCategory(&bg, "assets/txtures/mid_evil/PNG/Default size/Unit", "medievalUnit", TILE_SPECIAL_START, 60);
    
    TraceLog(LOG_INFO, "All medieval tile categories loaded! Tiles: 0-59, Environment: 60-119, Structure: 120-179, Unit: 180-239");

    // Initialize base map with dirt floor (Tile 14) and no decorations
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            bg.map[y][x] = 14;  // medievalTile_14.png (Dirt/Floor)
            bg.decorationMap[y][x] = 0;  // No decoration initially
            bg.collisionMap[y][x] = false;  // No collision by default
        }
    }
    TraceLog(LOG_INFO, "Empty design canvas created with base map at Tile 14 and empty decoration layer!");

    // Load saved map if it exists
    if (FileExists("assets/maps/map.dat")) {
        LoadMap(&bg, "assets/maps/map.dat");
    }
    
    return bg;
}

void LoadCategory(Background* background, const char* tileFolder, const char* filePrefix, int startIndex, int maxTiles) {
    int successCount = 0;
    int fallbackCount = 0;
    
    for (int i = 0; i < maxTiles; i++) {
        int tileIndex = startIndex + i;
        if (tileIndex >= MAX_TILES) {
            TraceLog(LOG_WARNING, "Tile index %d exceeds MAX_TILES (%d). Stopping category load.", tileIndex, MAX_TILES);
            break;
        }

        // Build filepath using the provided prefix and zero-padded index
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s_%02d.png", tileFolder, filePrefix, i);
        background->tileSet[tileIndex] = LoadTexture(filepath);
        
        if (background->tileSet[tileIndex].id == 0) {
            // Fallback: Create a colored placeholder tile
            unsigned char colorValue = (unsigned char)(50 + i * 4);
            Image fallback = GenImageColor(TILE_SIZE, TILE_SIZE, (Color){colorValue, 100, 50, 255});
            background->tileSet[tileIndex] = LoadTextureFromImage(fallback);
            UnloadImage(fallback);
            fallbackCount++;
        } else {
            successCount++;
        }
    }
    
    TraceLog(LOG_INFO, "Category '%s' loaded: Folder: %s | Indices %d-%d | Success: %d | Fallback: %d", 
             filePrefix, tileFolder, startIndex, startIndex + maxTiles - 1, successCount, fallbackCount);
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
        if (tileIndex >= 0 && tileIndex < MAX_TILES) {
            background->map[y][x] = tileIndex;
        }
    }
}

void SetDecoration(Background* background, int x, int y, int tileIndex) {
    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
        if (tileIndex >= 0 && tileIndex < MAX_TILES) {
            background->decorationMap[y][x] = tileIndex;
        } else {
            background->decorationMap[y][x] = 0;  // Clear decoration with negative index
        }
    }
}

int GetDecoration(Background* background, int x, int y) {
    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
        return background->decorationMap[y][x];
    }
    return 0;
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
    // Using 64 as TILE_SIZE now ensures correct count
    int tilesNeededX = (background->screenWidth / TILE_SIZE) + 2; 
    int tilesNeededY = (background->screenHeight / TILE_SIZE) + 2;
    
    // 2. Determine the first tile index to draw using floor to prevent jumping/clipping
    int startTileX = (int)floor(background->position.x / TILE_SIZE);
    int startTileY = (int)floor(background->position.y / TILE_SIZE);
    
    // 3. Draw visible tiles - BASE LAYER
    for (int y = startTileY; y < startTileY + tilesNeededY; y++) {
        for (int x = startTileX; x < startTileX + tilesNeededX; x++) {
            // Safety bounds check for the 50x50 map array
            if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
                int tileIndex = background->map[y][x];
                
                if (tileIndex >= 0 && tileIndex < MAX_TILES && background->tileSet[tileIndex].id != 0) {
                    // 4. PRECISE FLOATING POINT POSITION
                    // Subtracting the background position from the world grid position
                    Vector2 screenPos = {
                        (float)(x * TILE_SIZE) - background->position.x,
                        (float)(y * TILE_SIZE) - background->position.y
                    };
                    
                    // Drawing at WHITE tint ensures your medieval colors show correctly
                    DrawTextureV(background->tileSet[tileIndex], screenPos, WHITE);
                }
            }
        }
    }
    
    // 4. Draw DECORATION LAYER on top
    for (int y = startTileY; y < startTileY + tilesNeededY; y++) {
        for (int x = startTileX; x < startTileX + tilesNeededX; x++) {
            if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT) {
                int decoIndex = background->decorationMap[y][x];
                
                // Only draw if there's a decoration (non-zero)
                if (decoIndex > 0 && decoIndex < MAX_TILES && background->tileSet[decoIndex].id != 0) {
                    Vector2 screenPos = {
                        (float)(x * TILE_SIZE) - background->position.x,
                        (float)(y * TILE_SIZE) - background->position.y
                    };
                    
                    DrawTextureV(background->tileSet[decoIndex], screenPos, WHITE);
                }
            }
        }
    }
}

Vector2 ScreenToWorldCoords(Vector2 screenPos, Vector2 bgPos) {
    return Vector2Add(screenPos, bgPos);
}


void GetTileCoords(Vector2 worldPos, int* tileX, int* tileY) {
    // Precise conversion from world pixels to grid indices
    *tileX = (int)floor(worldPos.x / TILE_SIZE);
    *tileY = (int)floor(worldPos.y / TILE_SIZE);
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
    
    // Write map data: base tile + decoration tile + collision for each position
    unsigned char mapData[MAP_HEIGHT * MAP_WIDTH * 4];  // 4 bytes per tile (base + deco + collision + padding)
    int dataIndex = 0;
    
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            mapData[dataIndex++] = (unsigned char)background->map[y][x];
            mapData[dataIndex++] = (unsigned char)background->decorationMap[y][x];
            mapData[dataIndex++] = (unsigned char)(background->collisionMap[y][x] ? 1 : 0);
            mapData[dataIndex++] = 0;  // Padding for alignment
        }
    }
    
    SaveFileData(filename, mapData, dataIndex);
    TraceLog(LOG_INFO, "Map saved to %s (base layer + decoration layer + collision data)", filename);
}

void LoadMap(Background* background, const char* filename) {
    if (!FileExists(filename)) {
        TraceLog(LOG_WARNING, "Map file not found: %s", filename);
        GenerateRandomMap(background);
        return;
    }
    
    int bytesRead = 0;
    unsigned char* mapData = LoadFileData(filename, &bytesRead);
    
    // Try new format first (4 bytes per tile: base + deco + collision + padding)
    int expectedSizeNew = MAP_HEIGHT * MAP_WIDTH * 4;
    int expectedSizeOld = MAP_HEIGHT * MAP_WIDTH * 2;  // Legacy format
    
    if (bytesRead == expectedSizeNew) {
        int dataIndex = 0;
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                background->map[y][x] = (int)mapData[dataIndex++];
                background->decorationMap[y][x] = (int)mapData[dataIndex++];
                background->collisionMap[y][x] = (mapData[dataIndex++] != 0);
                dataIndex++;  // Skip padding
            }
        }
        TraceLog(LOG_INFO, "Map loaded from %s (base + decoration + collision layers)", filename);
    } else if (bytesRead == expectedSizeOld) {
        // Backward compatibility: load legacy format (base + collision only)
        int dataIndex = 0;
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                background->map[y][x] = (int)mapData[dataIndex++];
                background->collisionMap[y][x] = (mapData[dataIndex++] != 0);
                background->decorationMap[y][x] = 0;  // No decorations in legacy maps
            }
        }
        TraceLog(LOG_INFO, "Legacy map loaded from %s (base + collision only)", filename);
    } else {
        TraceLog(LOG_WARNING, "Map file corrupted or invalid size. Expected %d or %d bytes, got %d. Generating random map.", 
                expectedSizeOld, expectedSizeNew, bytesRead);
        GenerateRandomMap(background);
    }
    
    UnloadFileData(mapData);
}

void UnloadBackground(Background* background) {
    for (int i = 0; i < MAX_TILES; i++) {
        if (background->tileSet[i].id != 0) {
            UnloadTexture(background->tileSet[i]);
        }
    }
    TraceLog(LOG_INFO, "Background tilesets unloaded! (%d tiles freed)", MAX_TILES);
}