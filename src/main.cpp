#include "raylib.h"
#include "menu.h"
#include "settings.h"
#include "game.h"

// Game States
typedef enum { MENU, PLAYING, PAUSED, SETTINGS, QUIT } GameState;

int main(void) {
    // 1. Initialization
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Neon Swarm");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);  // Disable ESC closing the window

    // Game State
    GameState gameState = MENU;

    // Initialize modules
    Menu menu = InitMenu(screenWidth, screenHeight);
    Settings settings = InitSettings(screenWidth, screenHeight);
    Player player = InitPlayer(screenWidth, screenHeight);
    int pauseMenuSelection = 0;

    // 2. Main Game Loop
    while (!WindowShouldClose() && gameState != QUIT) {
        // Check for quit key (Alt+F4 or close button only)
        if (IsKeyPressed(KEY_Q) && IsKeyDown(KEY_LEFT_ALT)) gameState = QUIT;
        
        // --- UPDATE ---

        if (gameState == MENU) {
            UpdateMenu(&menu);
            int selection = GetMenuSelection(&menu);
            if (selection == 0) {
                gameState = PLAYING;
                player = InitPlayer(screenWidth, screenHeight);
            }
            else if (selection == 1) gameState = SETTINGS;
            else if (selection == 2) gameState = QUIT;
        }
        else if (gameState == SETTINGS) {
            UpdateSettings(&settings);
            if (ShouldBackToMenu(&settings)) {
                ApplySettings(&settings);
                gameState = MENU;
            }
        }
        else if (gameState == PLAYING) {
            UpdatePlayer(&player, screenWidth, screenHeight);
            if (ShouldOpenPause()) gameState = PAUSED;
        }
        else if (gameState == PAUSED) {
            // Pause Menu Navigation
            if (IsKeyPressed(KEY_DOWN)) pauseMenuSelection = (pauseMenuSelection + 1) % 3;
            if (IsKeyPressed(KEY_UP)) pauseMenuSelection = (pauseMenuSelection - 1 + 3) % 3;
            
            // Handle mouse hover
            PauseMenuRects pauseRects = GetPauseMenuRects(screenWidth);
            pauseMenuSelection = HandlePauseMenuClick(pauseRects, pauseMenuSelection);
            
            // Handle keyboard or mouse click selection
            if (IsKeyPressed(KEY_ENTER) || IsPauseMenuItemClicked(pauseRects, pauseMenuSelection)) {
                if (pauseMenuSelection == 0) gameState = PLAYING;
                else if (pauseMenuSelection == 1) gameState = SETTINGS;
                else if (pauseMenuSelection == 2) gameState = MENU;
            }
        }

        // --- DRAW ---
        BeginDrawing();
        ClearBackground(BLACK);

        if (gameState == MENU) {
            DrawMenu(&menu, screenWidth, screenHeight);
        }
        else if (gameState == SETTINGS) {
            // Draw game in background (paused)
            DrawPlayer(&player);
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.7f));
            DrawSettings(&settings, screenWidth, screenHeight);
        }
        else if (gameState == PLAYING) {
            DrawPlayer(&player);
        }
        else if (gameState == PAUSED) {
            // Draw game in background
            DrawPlayer(&player);
            DrawPauseMenu(screenWidth, screenHeight, pauseMenuSelection);
        }

        EndDrawing();
    }

    // 3. De-Initialization
    CloseWindow();
    return 0;
}
