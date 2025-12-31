#ifndef SETTINGS_H
#define SETTINGS_H

#include "raylib.h"

typedef struct {
    Rectangle bounds;
    const char* text;
    bool hovered;
} SettingItem;

typedef struct {
    SettingItem items[4];
    int selectedIndex;
    float masterVolume;
    float musicVolume;
    bool fullscreen;
} Settings;

Settings InitSettings(int screenWidth, int screenHeight);
void UpdateSettings(Settings* settings);
void DrawSettings(Settings* settings, int screenWidth, int screenHeight);
bool ShouldBackToMenu(Settings* settings);
void ApplySettings(Settings* settings);

#endif
