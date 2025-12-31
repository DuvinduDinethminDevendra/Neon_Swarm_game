#ifndef MENU_H
#define MENU_H

#include "raylib.h"

typedef struct {
    Rectangle bounds;
    const char* text;
    bool hovered;
} MenuItem;

typedef struct {
    MenuItem items[3];
    int selectedIndex;
} Menu;

Menu InitMenu(int screenWidth, int screenHeight);
void UpdateMenu(Menu* menu);
void DrawMenu(Menu* menu, int screenWidth, int screenHeight);
int GetMenuSelection(Menu* menu);
bool IsMenuItemClicked(Menu* menu, int itemIndex);

#endif
