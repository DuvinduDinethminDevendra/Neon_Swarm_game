#include "menu.h"

Menu InitMenu(int screenWidth, int screenHeight) {
    Menu menu = {0};
    menu.items[0] = {{(float)(screenWidth / 2 - 100), 150.0f, 200.0f, 60.0f}, "PLAY", false};
    menu.items[1] = {{(float)(screenWidth / 2 - 100), 250.0f, 200.0f, 60.0f}, "SETTINGS", false};
    menu.items[2] = {{(float)(screenWidth / 2 - 100), 350.0f, 200.0f, 60.0f}, "QUIT", false};
    menu.selectedIndex = 0;
    return menu;
}

void UpdateMenu(Menu* menu) {
    // Keyboard Navigation
    if (IsKeyPressed(KEY_DOWN)) menu->selectedIndex = (menu->selectedIndex + 1) % 3;
    if (IsKeyPressed(KEY_UP)) menu->selectedIndex = (menu->selectedIndex - 1 + 3) % 3;

    // Mouse Hover and Click
    Vector2 mousePos = GetMousePosition();
    for (int i = 0; i < 3; i++) {
        menu->items[i].hovered = CheckCollisionPointRec(mousePos, menu->items[i].bounds);
        
        if (menu->items[i].hovered) {
            menu->selectedIndex = i;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // Item was clicked - selection will be returned by GetMenuSelection
            }
        }
    }
}

int GetMenuSelection(Menu* menu) {
    if (IsKeyPressed(KEY_ENTER)) {
        return menu->selectedIndex;
    }
    
    // Check for mouse clicks
    Vector2 mousePos = GetMousePosition();
    for (int i = 0; i < 3; i++) {
        if (CheckCollisionPointRec(mousePos, menu->items[i].bounds) && 
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            return i;
        }
    }
    
    return -1;
}

bool IsMenuItemClicked(Menu* menu, int itemIndex) {
    Vector2 mousePos = GetMousePosition();
    return (CheckCollisionPointRec(mousePos, menu->items[itemIndex].bounds) && 
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
}

void DrawMenu(Menu* menu, int screenWidth, int screenHeight) {
    DrawText("NEON SWARM", screenWidth / 2 - 150, 50, 40, LIME);
    
    for (int i = 0; i < 3; i++) {
        Color color = (menu->items[i].hovered || menu->selectedIndex == i) ? YELLOW : WHITE;
        DrawRectangleLines(menu->items[i].bounds.x, menu->items[i].bounds.y, 
                          menu->items[i].bounds.width, menu->items[i].bounds.height, color);
        DrawText(menu->items[i].text, menu->items[i].bounds.x + 40, 
                menu->items[i].bounds.y + 15, 20, color);
    }
    DrawText("UP/DOWN - Navigate | ENTER/Click - Select", 10, screenHeight - 30, 14, DARKGRAY);
}
