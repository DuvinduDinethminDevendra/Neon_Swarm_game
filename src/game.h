#ifndef GAME_H
#define GAME_H

#include "raylib.h"

#define MIN_SHOOT_DISTANCE 80.0f  // Minimum distance from player to shoot
#define RELOAD_FRAMES 6  // Number of reload animation frames
#define IDLE_FRAMES 4  // Number of idle animation frames
#define WALK_FRAMES 20
#define SHOOT_FRAMES 2

typedef enum {
    PLAYER_IDLE,
    PLAYER_WALKING,
    PLAYER_SHOOTING,
    PLAYER_RELOADING
} PlayerState;

typedef struct {
    Vector2 position;
    float speed;
    float radius;
    float rotation;
    Texture2D texture;
    Texture2D idleTextures[IDLE_FRAMES];      // Idle animation frames
    Texture2D walkTextures[WALK_FRAMES];
    Texture2D shootTextures[SHOOT_FRAMES];
    Texture2D reloadTextures[RELOAD_FRAMES];  // Reload animation frames
    Vector2 barrelOffset;  // Offset from center to barrel tip
    float scale;  // Texture scale factor
    bool isMoving;  // Track if player is currently moving
    PlayerState state;
} Player;

typedef enum { GAME_PLAYING, GAME_PAUSED } GamePlayState;

typedef struct {
    Rectangle resumeRect;
    Rectangle settingsRect;
    Rectangle menuRect;
} PauseMenuRects;

Player InitPlayer(int screenWidth, int screenHeight);
void UpdatePlayer(Player* player, int screenWidth, int screenHeight);
void DrawPlayer(Player* player);
Vector2 GetBarrelPosition(Player* player);
void InitializeTextures(void);
void DrawPauseMenu(int screenWidth, int screenHeight, int selectedItem);
PauseMenuRects GetPauseMenuRects(int screenWidth);
int HandlePauseMenuClick(PauseMenuRects rects, int currentSelection);
bool IsPauseMenuItemClicked(PauseMenuRects rects, int itemIndex);
bool ShouldReturnToMenu(void);
bool ShouldOpenPause(void);

#endif
