#include "game.h"
#include "raymath.h"
#include <math.h>
#include <cstdio>

Texture2D PlayerTexture;
Shader slimeShader = {0};
int squashLoc = 0;

void InitializeTextures(void) {
    PlayerTexture = LoadTexture("assets/txtures/Top_Down_Survivor/rifle/shoot/survivor-shoot_rifle_0.png");
    if (PlayerTexture.id == 0) {
        TraceLog(LOG_WARNING, "Player texture not found. Using fallback.");
    } else {
        TraceLog(LOG_INFO, "Player texture loaded successfully!");
    }
    
    // Load slime shader
    slimeShader = LoadShader(0, "assets/shaders/slime.fs");
    if (slimeShader.id != 0) {
        squashLoc = GetShaderLocation(slimeShader, "squashFactor");
        TraceLog(LOG_INFO, "Slime shader loaded successfully!");
    } else {
        TraceLog(LOG_WARNING, "Failed to load slime shader");
    }
}

Player InitPlayer(int screenWidth, int screenHeight) {
    Player player = {0};
    player.position.x = screenWidth / 2;
    player.position.y = screenHeight / 2;
    player.speed = 4.0f;
    player.radius = 15.0f;
    player.rotation = 0.0f;
    player.texture = PlayerTexture;
    player.scale = 0.3f;
    player.isMoving = false;
    player.state = PLAYER_IDLE;
    player.health = 100.0f;     // Starting health
    player.maxHealth = 100.0f;  // Max health
    
    float baseBarrelX = 120.0f;
    float baseBarrelY = 46.0f;
    player.barrelOffset = {baseBarrelX, baseBarrelY};
    
    // Load idle animation frames
    for (int i = 0; i < IDLE_FRAMES; i++) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "assets/txtures/Top_Down_Survivor/rifle/idle/survivor-idle_rifle_%d.png", i);
        player.idleTextures[i] = LoadTexture(filepath);
        if (player.idleTextures[i].id == 0) {
            TraceLog(LOG_WARNING, "Failed to load idle texture: %s", filepath);
        }
    }
    
    // Load walk animation frames
    for (int i = 0; i < WALK_FRAMES; i++) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "assets/txtures/Top_Down_Survivor/rifle/move/survivor-move_rifle_%d.png", i);
        player.walkTextures[i] = LoadTexture(filepath);
        if (player.walkTextures[i].id == 0) {
            TraceLog(LOG_WARNING, "Failed to load walk texture: %s", filepath);
        }
    }
    
    // Load shoot animation frames
    for (int i = 0; i < SHOOT_FRAMES; i++) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "assets/txtures/Top_Down_Survivor/rifle/shoot/survivor-shoot_rifle_%d.png", i);
        player.shootTextures[i] = LoadTexture(filepath);
        if (player.shootTextures[i].id == 0) {
            TraceLog(LOG_WARNING, "Failed to load shoot texture: %s", filepath);
        }
    }
    
    // Load reload animation frames
    for (int i = 0; i < RELOAD_FRAMES; i++) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "assets/txtures/Top_Down_Survivor/rifle/reload/survivor-reload_rifle_%d.png", i);
        player.reloadTextures[i] = LoadTexture(filepath);
        if (player.reloadTextures[i].id == 0) {
            TraceLog(LOG_WARNING, "Failed to load reload texture: %s", filepath);
        }
    }
    
    return player;
}

void UpdatePlayer(Player* player, int screenWidth, int screenHeight) {
    // Track if player is moving
    player->isMoving = false;
    
    // WASD Movement
    if (IsKeyDown(KEY_W)) { player->position.y -= player->speed; player->isMoving = true; }
    if (IsKeyDown(KEY_S)) { player->position.y += player->speed; player->isMoving = true; }
    if (IsKeyDown(KEY_A)) { player->position.x -= player->speed; player->isMoving = true; }
    if (IsKeyDown(KEY_D)) { player->position.x += player->speed; player->isMoving = true; }

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
    // Check if texture is valid
    if (player->texture.id == 0) {
        // Fallback: Draw as triangle if texture failed to load
        DrawPoly(player->position, 3, player->radius, player->rotation, LIME);
    } else {
        // Draw Player texture rotated and scaled
        Rectangle source = {0, 0, (float)player->texture.width, (float)player->texture.height};
        Rectangle dest = {player->position.x, player->position.y, (float)player->texture.width * player->scale, (float)player->texture.height * player->scale};
        Vector2 origin = {(float)player->texture.width * player->scale / 2, (float)player->texture.height * player->scale / 2};
        DrawTexturePro(player->texture, source, dest, origin, player->rotation, WHITE);
    }
    
    // Draw reticle at mouse position
    Vector2 mousePos = GetMousePosition();
    DrawCircleLines(mousePos.x, mousePos.y, 10, Fade(LIME, 0.5f));
    
    // HUD
    DrawText("WASD to Move | Mouse to Aim | ESC for Menu | R to Reload", 10, 10, 14, DARKGRAY);
    
    // --- PLAYER HEALTH BAR (BOTTOM MIDDLE) ---
    float barWidth = 200.0f;
    float barHeight = 20.0f;
    float barX = (GetScreenWidth() - barWidth) / 2.0f;
    float barY = GetScreenHeight() - 60.0f;
    
    // Background (Black outline)
    DrawRectangle(barX - 2, barY - 2, barWidth + 4, barHeight + 4, BLACK);
    // Health (Red to Green gradient effect)
    float healthPercent = player->health / player->maxHealth;
    Color healthColor = (healthPercent > 0.5f) ? GREEN : (healthPercent > 0.25f) ? YELLOW : RED;
    DrawRectangle(barX, barY, barWidth * healthPercent, barHeight, healthColor);
    
    // Health text
    DrawText(TextFormat("HP: %.0f/%.0f", player->health, player->maxHealth), 
            barX, barY + 25, 14, WHITE);
}

bool ShouldReturnToMenu(void) {
    return false;
}

bool ShouldOpenPause(void) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        TraceLog(LOG_DEBUG, "Pause menu triggered!");
        return true;
    }
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

Vector2 GetBarrelPosition(Player* player) {
    // Scale the barrel offset and then rotate it
    Vector2 scaledOffset = {
        player->barrelOffset.x * player->scale,
        player->barrelOffset.y * player->scale
    };
    
    // Rotate the scaled barrel offset by player rotation
    float rad = player->rotation * DEG2RAD;
    Vector2 rotatedOffset = {
        scaledOffset.x * cosf(rad) - scaledOffset.y * sinf(rad),
        scaledOffset.x * sinf(rad) + scaledOffset.y * cosf(rad)
    };
    return Vector2Add(player->position, rotatedOffset);
}
