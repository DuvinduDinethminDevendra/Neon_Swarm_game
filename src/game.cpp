#include "game.h"
#include <math.h>

Player InitPlayer(int screenWidth, int screenHeight) {
    Player player = {0};
    player.position.x = screenWidth / 2;
    player.position.y = screenHeight / 2;
    player.speed = 4.0f;
    player.radius = 15.0f;
    player.rotation = 0.0f;
    return player;
}

void UpdatePlayer(Player* player, int screenWidth, int screenHeight) {
    // WASD Movement
    if (IsKeyDown(KEY_W)) player->position.y -= player->speed;
    if (IsKeyDown(KEY_S)) player->position.y += player->speed;
    if (IsKeyDown(KEY_A)) player->position.x -= player->speed;
    if (IsKeyDown(KEY_D)) player->position.x += player->speed;

    // Keep player in bounds
    if (player->position.x < player->radius) player->position.x = player->radius;
    if (player->position.x > screenWidth - player->radius) player->position.x = screenWidth - player->radius;
    if (player->position.y < player->radius) player->position.y = player->radius;
    if (player->position.y > screenHeight - player->radius) player->position.y = screenHeight - player->radius;

    // Rotation Math: Calculate angle between Player and Mouse
    Vector2 mousePos = GetMousePosition();
    player->rotation = atan2f(mousePos.y - player->position.y, mousePos.x - player->position.x) * RAD2DEG;
}

void DrawPlayer(Player* player) {
    // Draw Player (as a triangle to show direction)
    DrawPoly(player->position, 3, player->radius, player->rotation, LIME);
    
    // Draw reticle at mouse position
    Vector2 mousePos = GetMousePosition();
    DrawCircleLines(mousePos.x, mousePos.y, 10, Fade(LIME, 0.5f));
    
    // HUD
    DrawText("WASD to Move | Mouse to Aim | ESC for Menu", 10, 10, 14, DARKGRAY);
}

bool ShouldReturnToMenu(void) {
    return false;
}

bool ShouldOpenPause(void) {
    if (IsKeyPressed(KEY_ESCAPE)) return true;
    return false;
}

void DrawPauseMenu(int screenWidth, int screenHeight, int selectedItem) {
    // Semi-transparent overlay
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.5f));
    
    // Pause Menu
    DrawText("PAUSED", screenWidth / 2 - 80, 100, 40, YELLOW);
    
    // Resume option
    Rectangle resumeRect = {screenWidth / 2.0f - 100, 200, 200, 60};
    Color resumeColor = (selectedItem == 0) ? YELLOW : WHITE;
    DrawRectangleLines(resumeRect.x, resumeRect.y, resumeRect.width, resumeRect.height, resumeColor);
    DrawText("RESUME", screenWidth / 2 - 70, 225, 20, resumeColor);
    
    // Settings option
    Rectangle settingsRect = {screenWidth / 2.0f - 100, 300, 200, 60};
    Color settingsColor = (selectedItem == 1) ? YELLOW : WHITE;
    DrawRectangleLines(settingsRect.x, settingsRect.y, settingsRect.width, settingsRect.height, settingsColor);
    DrawText("SETTINGS", screenWidth / 2 - 70, 325, 20, settingsColor);
    
    // Menu option
    Rectangle menuRect = {screenWidth / 2.0f - 100, 400, 200, 60};
    Color menuColor = (selectedItem == 2) ? YELLOW : WHITE;
    DrawRectangleLines(menuRect.x, menuRect.y, menuRect.width, menuRect.height, menuColor);
    DrawText("MENU", screenWidth / 2 - 50, 425, 20, menuColor);
    
    DrawText("UP/DOWN - Navigate | ENTER/Click - Select", 10, screenHeight - 30, 14, DARKGRAY);
}

PauseMenuRects GetPauseMenuRects(int screenWidth) {
    PauseMenuRects rects;
    rects.resumeRect = {screenWidth / 2.0f - 100, 200, 200, 60};
    rects.settingsRect = {screenWidth / 2.0f - 100, 300, 200, 60};
    rects.menuRect = {screenWidth / 2.0f - 100, 400, 200, 60};
    return rects;
}

int HandlePauseMenuClick(PauseMenuRects rects, int currentSelection) {
    Vector2 mousePos = GetMousePosition();
    
    // Hover detection for visual feedback
    if (CheckCollisionPointRec(mousePos, rects.resumeRect)) return 0;
    if (CheckCollisionPointRec(mousePos, rects.settingsRect)) return 1;
    if (CheckCollisionPointRec(mousePos, rects.menuRect)) return 2;
    
    return currentSelection;
}

bool IsPauseMenuItemClicked(PauseMenuRects rects, int itemIndex) {
    Vector2 mousePos = GetMousePosition();
    Rectangle targetRect;
    
    if (itemIndex == 0) targetRect = rects.resumeRect;
    else if (itemIndex == 1) targetRect = rects.settingsRect;
    else targetRect = rects.menuRect;
    
    return (CheckCollisionPointRec(mousePos, targetRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
}
