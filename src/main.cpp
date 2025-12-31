#include "raylib.h"
#include "raymath.h"
#include "menu.h"
#include "settings.h"
#include "game.h"
#include "bullets.h"

// Game States
typedef enum { MENU, PLAYING, PAUSED, SETTINGS, QUIT } GameState;

int main(void) {
    // 1. Initialization
    const int screenWidth = 800;
    const int screenHeight = 600;
    const float reloadDuration = 120.0f;  // Reload duration in frames (2 seconds at 60 FPS)
    InitWindow(screenWidth, screenHeight, "Neon Swarm");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);  // Disable ESC closing the window
    InitializeTextures();

    // Game State
    GameState gameState = MENU;

    // Initialize modules
    Menu menu = InitMenu(screenWidth, screenHeight);
    Settings settings = InitSettings(screenWidth, screenHeight);
    Player player = InitPlayer(screenWidth, screenHeight);
    Bullet bullets[100] = {0};
    InitBulletPool(bullets, 100);
    int bulletCount = 100;
    int bulletCapacity = 100;
    int reloadTimer = 0;
    int pauseMenuSelection = 0;
    float idleFrameCounter = 0.0f;  // Counter for idle animation
    const float idleFrameRate = 0.15f;  // Time between frames (0.15s per frame)

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
            UpdateBullets(bullets, bulletCount);

            // Decrement reload timer
            if (reloadTimer > 0) reloadTimer--;

            // Update player state
            if (reloadTimer > 0) {
                player.state = PLAYER_RELOADING;
            } else if (player.isMoving) {
                player.state = PLAYER_WALKING;
            } else {
                player.state = PLAYER_IDLE;
            }

            //shoot bullet from barrel
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && reloadTimer == 0) {
                Vector2 mousePos = GetMousePosition();
                float distToMouse = Vector2Distance(player.position, mousePos);
                
                if (distToMouse > MIN_SHOOT_DISTANCE && bulletCapacity > 0) {
                    Vector2 barrelPos = GetBarrelPosition(&player);
                    ShootBullet(bullets, bulletCount, barrelPos, mousePos, 7.0f, LIME);
                    bulletCapacity--;
                    player.state = PLAYER_SHOOTING;
                }
            }

            // Reset bullet capacity with R key
            if (IsKeyPressed(KEY_R)) {
                bulletCapacity = 100;
                reloadTimer = reloadDuration;
                InitBulletPool(bullets, 100);
            }

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
            DrawBullets(bullets, bulletCount);
        }
        else if (gameState == PLAYING) {
            // Draw player based on state
            switch (player.state) {
                case PLAYER_IDLE: {
                    idleFrameCounter += GetFrameTime();
                    int frameIndex = (int)(idleFrameCounter / idleFrameRate) % IDLE_FRAMES;
                    
                    if (player.idleTextures[frameIndex].id != 0) {
                        Texture2D currentTex = player.idleTextures[frameIndex];
                        Rectangle source = { 0, 0, (float)currentTex.width, (float)currentTex.height };
                        Rectangle dest = { player.position.x, player.position.y, (float)currentTex.width * player.scale, (float)currentTex.height * player.scale };
                        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };
                        DrawTexturePro(currentTex, source, dest, origin, player.rotation, WHITE);
                    }
                    break;
                }
                case PLAYER_WALKING: {
                    idleFrameCounter += GetFrameTime();
                    int frameIndex = (int)(idleFrameCounter / 0.05f) % WALK_FRAMES;  // 0.05s per frame for smoother animation
                    
                    // Use walk texture, fallback to idle if not available
                    if (player.walkTextures[frameIndex].id != 0) {
                        Texture2D currentTex = player.walkTextures[frameIndex];
                        Rectangle source = { 0, 0, (float)currentTex.width, (float)currentTex.height };
                        Rectangle dest = { player.position.x, player.position.y, (float)currentTex.width * player.scale, (float)currentTex.height * player.scale };
                        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };
                        DrawTexturePro(currentTex, source, dest, origin, player.rotation, WHITE);
                    } else {
                        // Fallback: Draw green triangle if walk texture failed
                        DrawPoly(player.position, 3, player.radius, player.rotation, GREEN);
                    }
                    
                    // Draw reticle and HUD
                    Vector2 mousePos = GetMousePosition();
                    DrawCircleLines(mousePos.x, mousePos.y, 10, Fade(LIME, 0.5f));
                    DrawText("WASD to Move | Mouse to Aim | ESC for Menu | R to Reload", 10, 10, 14, DARKGRAY);
                    break;
                }
                case PLAYER_RELOADING: {
                    float progress = 1.0f - (reloadTimer / reloadDuration);
                    int frameIndex = (int)(progress * RELOAD_FRAMES);
                    if (frameIndex >= RELOAD_FRAMES) frameIndex = RELOAD_FRAMES - 1;
                    if (frameIndex < 0) frameIndex = 0;

                    if (player.reloadTextures[frameIndex].id != 0) {
                        Texture2D currentTex = player.reloadTextures[frameIndex];
                        Rectangle source = { 0, 0, (float)currentTex.width, (float)currentTex.height };
                        Rectangle dest = { player.position.x, player.position.y, (float)currentTex.width * player.scale, (float)currentTex.height * player.scale };
                        Vector2 origin = { dest.width / 2.0f, dest.height / 2.0f };
                        DrawTexturePro(currentTex, source, dest, origin, player.rotation, WHITE);
                    }
                    break;
                }
                case PLAYER_SHOOTING: {
                    DrawPlayer(&player);
                    break;
                }
            }
            
            // Decrement reload timer with delta time
            if (reloadTimer > 0) reloadTimer -= GetFrameTime();
            if (reloadTimer < 0) reloadTimer = 0;
            
            DrawBullets(bullets, bulletCount);
            DrawCircleLines(player.position.x, player.position.y, MIN_SHOOT_DISTANCE, Fade(RED, 0.3f));
            
            // Draw bullet counter and reload status
            DrawText(TextFormat("Bullets: %d/100", bulletCapacity), 10, screenHeight - 30, 20, LIME);
            if (reloadTimer > 0) {
                float reloadPercent = (reloadTimer / reloadDuration) * 100.0f;
                DrawText(TextFormat("Reloading: %.0f%%", reloadPercent), 10, screenHeight - 60, 20, RED);
            }
        }
        else if (gameState == PAUSED) {
            // Draw game in background
            DrawPlayer(&player);
            DrawPauseMenu(screenWidth, screenHeight, pauseMenuSelection);
            DrawBullets(bullets, bulletCount);
        }

        EndDrawing();
    }

    // 3. De-Initialization
    CloseWindow();
    return 0;
}
