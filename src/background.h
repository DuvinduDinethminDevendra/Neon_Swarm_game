#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "raylib.h"

#define MAP_WIDTH 50
#define MAP_HEIGHT 50
#define TILE_SIZE 64
#define MAX_TILES 256

// Tile Category Ranges (Medieval Asset Organization)
#define TILE_FLOOR_START 0      // Tiles - Dirt, grass, floor (0-59)
#define TILE_FLOOR_END 59

#define TILE_STRUCTURE_START 60 // Environment - Trees, vegetation (60-119)
#define TILE_STRUCTURE_END 119

#define TILE_DECO_START 120     // Structure - Buildings, walls (120-179)
#define TILE_DECO_END 179

#define TILE_SPECIAL_START 180  // Unit - Characters, enemies (180-239)
#define TILE_SPECIAL_END 239

typedef struct {
    Texture2D tileSet[MAX_TILES];
    int map[MAP_HEIGHT][MAP_WIDTH];
    int decorationMap[MAP_HEIGHT][MAP_WIDTH];  // Second layer for structures and decorations
    bool collisionMap[MAP_HEIGHT][MAP_WIDTH];
    Vector2 position;
    float scrollSpeed;
    int screenWidth;
    int screenHeight;
} Background;

Background InitBackground(int screenWidth, int screenHeight);
void UpdateBackground(Background* background, Vector2 cameraTarget);
void DrawBackground(Background* background);
void UnloadBackground(Background* background);
void LoadCategory(Background* background, const char* tileFolder, const char* filePrefix, int startIndex, int maxTiles);
void GenerateRandomMap(Background* background);
void SetTile(Background* background, int x, int y, int tileIndex);
void SetDecoration(Background* background, int x, int y, int tileIndex);
int GetDecoration(Background* background, int x, int y);
void SetCollision(Background* background, int x, int y, bool isBlocked);
bool GetCollision(Background* background, int x, int y);
void SaveMap(Background* background, const char* filename);
void LoadMap(Background* background, const char* filename);
Vector2 ScreenToWorldCoords(Vector2 screenPos, Vector2 bgPos);
void GetTileCoords(Vector2 worldPos, int* tileX, int* tileY);
bool IsTileWalkable(Background* background, Vector2 worldPos);

#endif
