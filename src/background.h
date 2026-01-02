#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "raylib.h"

#define MAP_WIDTH 50
#define MAP_HEIGHT 50
#define TILE_SIZE 64

typedef struct {
    Texture2D tileSet[60];
    int map[MAP_HEIGHT][MAP_WIDTH];
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
void LoadTileset(Background* background, const char* tileFolder);
void GenerateRandomMap(Background* background);
void SetTile(Background* background, int x, int y, int tileIndex);
void SetCollision(Background* background, int x, int y, bool isBlocked);
bool GetCollision(Background* background, int x, int y);
void SaveMap(Background* background, const char* filename);
void LoadMap(Background* background, const char* filename);
Vector2 ScreenToWorldCoords(Vector2 screenPos, Vector2 bgPos);
void GetTileCoords(Vector2 worldPos, int* tileX, int* tileY);
bool IsTileWalkable(Background* background, Vector2 worldPos);

#endif
