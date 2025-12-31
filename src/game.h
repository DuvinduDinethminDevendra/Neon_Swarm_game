#ifndef GAME_H
#define GAME_H

#include "raylib.h"

typedef struct {
    Vector2 position;
    float speed;
    float radius;
    float rotation;
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
void DrawPauseMenu(int screenWidth, int screenHeight, int selectedItem);
PauseMenuRects GetPauseMenuRects(int screenWidth);
int HandlePauseMenuClick(PauseMenuRects rects, int currentSelection);
bool IsPauseMenuItemClicked(PauseMenuRects rects, int itemIndex);
bool ShouldReturnToMenu(void);
bool ShouldOpenPause(void);

#endif
