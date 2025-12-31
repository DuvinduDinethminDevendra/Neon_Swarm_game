#include "settings.h"
#include <math.h>

Settings InitSettings(int screenWidth, int screenHeight) {
    Settings settings = {0};
    settings.items[0] = {{50.0f, 100.0f, 300.0f, 40.0f}, "Master Volume", false};
    settings.items[1] = {{50.0f, 160.0f, 300.0f, 40.0f}, "Music Volume", false};
    settings.items[2] = {{50.0f, 220.0f, 300.0f, 40.0f}, "Fullscreen", false};
    settings.items[3] = {{50.0f, 300.0f, 150.0f, 60.0f}, "BACK", false};
    settings.selectedIndex = 0;
    settings.masterVolume = GetMasterVolume();
    settings.musicVolume = 0.8f;
    settings.fullscreen = IsWindowState(FLAG_FULLSCREEN_MODE);
    return settings;
}

void UpdateSettings(Settings* settings) {
    // Navigation
    if (IsKeyPressed(KEY_DOWN)) settings->selectedIndex = (settings->selectedIndex + 1) % 4;
    if (IsKeyPressed(KEY_UP)) settings->selectedIndex = (settings->selectedIndex - 1 + 4) % 4;

    // Mouse hover
    Vector2 mousePos = GetMousePosition();
    for (int i = 0; i < 3; i++) {
        settings->items[i].hovered = CheckCollisionPointRec(mousePos, settings->items[i].bounds);
        if (settings->items[i].hovered) {
            settings->selectedIndex = i;
        }
    }

    // Adjust volumes with keyboard
    if (settings->selectedIndex == 0) {
        if (IsKeyDown(KEY_RIGHT)) settings->masterVolume = fminf(settings->masterVolume + 0.02f, 1.0f);
        if (IsKeyDown(KEY_LEFT)) settings->masterVolume = fmaxf(settings->masterVolume - 0.02f, 0.0f);
    }
    
    if (settings->selectedIndex == 1) {
        if (IsKeyDown(KEY_RIGHT)) settings->musicVolume = fminf(settings->musicVolume + 0.02f, 1.0f);
        if (IsKeyDown(KEY_LEFT)) settings->musicVolume = fmaxf(settings->musicVolume - 0.02f, 0.0f);
    }
    
    // Mouse slider dragging for Master Volume
    if (settings->selectedIndex == 0 && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePos, {350.0f, 100.0f, 200.0f, 30.0f})) {
            float relativeX = mousePos.x - 350.0f;
            settings->masterVolume = fmaxf(0.0f, fminf(1.0f, relativeX / 200.0f));
            SetMasterVolume(settings->masterVolume);
        }
    }
    
    // Mouse slider dragging for Music Volume
    if (settings->selectedIndex == 1 && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePos, {350.0f, 160.0f, 200.0f, 30.0f})) {
            float relativeX = mousePos.x - 350.0f;
            settings->musicVolume = fmaxf(0.0f, fminf(1.0f, relativeX / 200.0f));
        }
    }
    
    // Toggle fullscreen
    if (settings->selectedIndex == 2 && IsKeyPressed(KEY_ENTER)) {
        settings->fullscreen = !settings->fullscreen;
        ApplySettings(settings);
    }
    
    if (settings->items[2].hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        settings->fullscreen = !settings->fullscreen;
        ApplySettings(settings);
    }
}

void DrawSettings(Settings* settings, int screenWidth, int screenHeight) {
    DrawText("SETTINGS", screenWidth / 2 - 80, 30, 40, LIME);

    // Master Volume
    Color vol1Color = (settings->selectedIndex == 0) ? YELLOW : WHITE;
    DrawText("Master Volume:", 50, 100, 20, vol1Color);
    DrawRectangleLines(350, 100, 200, 30, vol1Color);
    DrawRectangle(350, 100, settings->masterVolume * 200, 30, LIME);
    DrawText(TextFormat("%.0f%%", settings->masterVolume * 100), 560, 105, 18, WHITE);

    // Music Volume
    Color vol2Color = (settings->selectedIndex == 1) ? YELLOW : WHITE;
    DrawText("Music Volume:", 50, 160, 20, vol2Color);
    DrawRectangleLines(350, 160, 200, 30, vol2Color);
    DrawRectangle(350, 160, settings->musicVolume * 200, 30, LIME);
    DrawText(TextFormat("%.0f%%", settings->musicVolume * 100), 560, 165, 18, WHITE);

    // Fullscreen Toggle
    Color fsColor = (settings->selectedIndex == 2) ? YELLOW : WHITE;
    DrawText("Fullscreen:", 50, 220, 20, fsColor);
    DrawRectangleLines(350, 210, 100, 40, fsColor);
    DrawText(settings->fullscreen ? "ON" : "OFF", 360, 218, 20, settings->fullscreen ? GREEN : RED);

    // Back button
    Color backColor = (settings->selectedIndex == 3) ? YELLOW : WHITE;
    DrawRectangleLines(50, 300, 150, 60, backColor);
    DrawText("BACK", 80, 325, 20, backColor);

    DrawText("LEFT/RIGHT - Volume | ENTER/Click - Toggle | ESC - Back", 10, screenHeight - 30, 14, DARKGRAY);
}

bool ShouldBackToMenu(Settings* settings) {
    if (settings->selectedIndex == 3 && IsKeyPressed(KEY_ENTER)) return true;
    
    // Click back button
    Vector2 mousePos = GetMousePosition();
    if (CheckCollisionPointRec(mousePos, settings->items[3].bounds) && 
        IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return true;
    
    if (IsKeyPressed(KEY_ESCAPE)) return true;
    return false;
}

void ApplySettings(Settings* settings) {
    // Apply fullscreen
    if (settings->fullscreen && !IsWindowState(FLAG_FULLSCREEN_MODE)) {
        ToggleFullscreen();
    } else if (!settings->fullscreen && IsWindowState(FLAG_FULLSCREEN_MODE)) {
        ToggleFullscreen();
    }
    
    // Apply volumes
    SetMasterVolume(settings->masterVolume);
}
