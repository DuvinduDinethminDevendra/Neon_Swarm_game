#include "raylib.h"
#include "raymath.h"
#include "menu.h"
#include "settings.h"
#include "game.h"
#include "bullets.h"
#include "enemy.h"
#include "background.h"

// Game States
typedef enum { MENU, PLAYING, PAUSED, SETTINGS, QUIT } GameState;

int main(void) {
    // 1. Initialization
    const int screenWidth = 800;
    const int screenHeight = 600;
    const float reloadDuration = 120.0f;  // Reload duration in frames (2 seconds at 60 FPS)
    InitWindow(screenWidth, screenHeight, "Neon Swarm");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    
    // Initialize audio BEFORE loading sounds
    InitAudioDevice();
    
    // Now initialize textures and sounds
    InitializeTextures();
    InitSounds();

    TraceLog(LOG_INFO, "Audio initialized. Master volume: %.2f", GetMasterVolume());

    // Game State
    GameState gameState = MENU;

    // Initialize modules
    Menu menu = InitMenu(screenWidth, screenHeight);
    Settings settings = InitSettings(screenWidth, screenHeight);
    Player player = InitPlayer(screenWidth, screenHeight);
    Background background = InitBackground(screenWidth, screenHeight);
    Bullet bullets[100] = {0};
    InitBulletPool(bullets, 100);
    Particle particles[200] = {0};
    InitParticlePool(particles, 200);
    int bulletCount = 100;
    int bulletCapacity = 100;
    int reloadTimer = 0;
    int pauseMenuSelection = 0;
    float idleFrameCounter = 0.0f;  // Counter for idle animation
    const float idleFrameRate = 0.15f;  // Time between frames (0.15s per frame for smoother animation)
    CameraShake cameraShake = {0, 0};
    
    // --- MAP EDITOR VARIABLES ---
    int currentSelectedTile = 0;
    bool mapEditorMode = false;
    bool collisionEditorMode = false;  // Toggle between tile and collision editing

    // Initialize enemy pool
    EnemyPool enemyPool = InitEnemyPool();
    SpawnSlime(&enemyPool, (Vector2){200, 150});
    SpawnSlime(&enemyPool, (Vector2){600, 150});
    SpawnSlime(&enemyPool, (Vector2){400, 450});

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
            // 1. Store the position BEFORE moving
            Vector2 previousPos = player.position;

            // 2. Perform regular movement update
            UpdatePlayer(&player, screenWidth, screenHeight);

            // 3. Check if the NEW position is blocked
            if (!IsTileWalkable(&background, player.position)) {
                // Revert only to the previous valid position
                player.position = previousPos; 
            }
            
            // --- COLLISION CHECK FOR PLAYER ---
            // Check if next position would be walkable
            if (!IsTileWalkable(&background, player.position)) {
                // Revert to previous position by moving back
                Vector2 revertMove = Vector2Scale(player.position, 0);
                player.position = revertMove;
            }
            
            UpdateBullets(bullets, bulletCount);
            UpdateParticles(particles, 200);
            UpdateScreenShake(&cameraShake);
            UpdateEnemies(&enemyPool, player.position, screenWidth, screenHeight);
            UpdateBackground(&background, player.position);

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
                    ShootBullet(bullets, bulletCount, barrelPos, mousePos, 7.0f, YELLOW);
                    bulletCapacity--;
                    TriggerScreenShake(&cameraShake, 1.5f);  // Muzzle shake
                }
            }

            // Reset bullet capacity with R key
            if (IsKeyPressed(KEY_R)) {
                bulletCapacity = 100;
                reloadTimer = reloadDuration;
                InitBulletPool(bullets, 100);
                
                // --- PLAY RELOAD SOUND ---
                if (gunReloadSound.frameCount > 0 && IsSoundValid(gunReloadSound)) {
                    PlaySound(gunReloadSound);
                    TraceLog(LOG_DEBUG, "Reload sound played!");
                }
            }

            // Spawn 3 random slimes with backtick key (for testing)
            if (IsKeyPressed(KEY_GRAVE)) {
                for (int i = 0; i < 3; i++) {
                    float randomX = 50.0f + (float)(rand() % (screenWidth - 100));
                    float randomY = 50.0f + (float)(rand() % (screenHeight - 100));
                    SpawnSlime(&enemyPool, (Vector2){randomX, randomY});
                }
                TraceLog(LOG_INFO, "Spawned 3 test slimes!");
            }

            // Check bullet-enemy collisions
            for (int b = 0; b < bulletCount; b++) {
                if (!bullets[b].active) continue;
                
                for (int e = 0; e < enemyPool.count; e++) {
                    if (!enemyPool.enemies[e].active) continue;
                    
                    // Check collision between bullet and enemy
                    float distance = Vector2Distance(bullets[b].position, enemyPool.enemies[e].position);
                    if (distance < bullets[b].radius + enemyPool.enemies[e].radius) {
                        // Collision detected!
                        Vector2 knockbackDir = Vector2Subtract(enemyPool.enemies[e].position, bullets[b].position);
                        TakeDamage(&enemyPool.enemies[e], bullets[b].damage, knockbackDir);
                        
                        // --- IMPACT EFFECTS ---
                        SpawnImpactParticles(particles, 200, bullets[b].position, bullets[b].velocity);
                        TriggerScreenShake(&cameraShake, 0.8f);
                        
                        bullets[b].active = false;
                    }
                }
            }
            
            // --- CHECK ENEMY-PLAYER COLLISION ---
            for (int e = 0; e < enemyPool.count; e++) {
                if (!enemyPool.enemies[e].active) continue;
                
                float distance = Vector2Distance(player.position, enemyPool.enemies[e].position);
                if (distance < player.radius + enemyPool.enemies[e].radius) {
                    // Check damage cooldown to prevent multiple hits per frame
                    if (enemyPool.enemies[e].lastDamageTime <= 0.0f) {
                        // Get random damage from enemy's range
                        float damageDealt = GetRandomDamage(&enemyPool.enemies[e]);
                        player.health -= damageDealt;
                        
                        // Set cooldown (0.5 seconds between hits)
                        enemyPool.enemies[e].lastDamageTime = 0.5f;
                        
                        // --- PLAY PLAYER DAMAGE SOUND ---
                        if (playerDamageSound.frameCount > 0 && IsSoundValid(playerDamageSound)) {
                            PlaySound(playerDamageSound);
                            TraceLog(LOG_DEBUG, "Player damage sound played!");
                        }
                        
                        TraceLog(LOG_INFO, "Player took %.0f damage!", damageDealt);
                    }
                    
                    // Knockback player away from enemy
                    Vector2 pushDir = Vector2Subtract(player.position, enemyPool.enemies[e].position);
                    pushDir = Vector2Normalize(pushDir);
                    player.position = Vector2Add(player.position, Vector2Scale(pushDir, 2.0f));
                }
            }
            
            // --- GAME OVER CONDITION ---
            if (player.health <= 0) {
                gameState = MENU;
                TraceLog(LOG_INFO, "Player defeated! Returning to menu.");
            }

            // --- MAP EDITOR CONTROLS ---
            if (IsKeyPressed(KEY_M)) {
                mapEditorMode = !mapEditorMode;
                collisionEditorMode = false;  // Reset collision mode when toggling editor
                TraceLog(LOG_INFO, "Map editor mode toggled: %s", mapEditorMode ? "ON" : "OFF");
            }
            
            if (mapEditorMode) {
                // Toggle between tile and collision editing with C key
                if (IsKeyPressed(KEY_C)) {
                    collisionEditorMode = !collisionEditorMode;
                    TraceLog(LOG_INFO, "Collision editing mode: %s", collisionEditorMode ? "ON" : "OFF");
                }
                
                // Cycle through tiles with mouse wheel (only in tile mode)
                if (!collisionEditorMode) {
                    float wheelMove = GetMouseWheelMove();
                    if (wheelMove != 0.0f) {
                        currentSelectedTile += (int)wheelMove;
                        if (currentSelectedTile < 0) currentSelectedTile = 59;
                        if (currentSelectedTile >= 60) currentSelectedTile = 0;
                        TraceLog(LOG_DEBUG, "Selected tile: %d", currentSelectedTile);
                    }
                }
                
                // Get mouse position in world space
                Vector2 mouseScreen = GetMousePosition();
                Vector2 mouseWorld = ScreenToWorldCoords(mouseScreen, background.position);
                
                int tileX, tileY;
                GetTileCoords(mouseWorld, &tileX, &tileY);
                
                if (collisionEditorMode) {
                    // --- COLLISION EDITOR ---
                    // Left click: Set collision (block tile)
                    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                        SetCollision(&background, tileX, tileY, true);
                    }
                    
                    // Right click: Remove collision (unblock tile)
                    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
                        SetCollision(&background, tileX, tileY, false);
                    }
                } else {
                    // --- TILE EDITOR ---
                    // Paint tiles with left mouse button
                    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                        SetTile(&background, tileX, tileY, currentSelectedTile);
                    }
                    
                    // Eyedropper tool with right mouse button
                    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
                        if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT) {
                            currentSelectedTile = background.map[tileY][tileX];
                            TraceLog(LOG_DEBUG, "Eyedropper: Selected tile %d", currentSelectedTile);
                        }
                    }
                }
                
                // Save map with S key
                if (IsKeyPressed(KEY_S)) {
                    SaveMap(&background, "assets/maps/map.dat");
                }
                
                // Load map with L key
                if (IsKeyPressed(KEY_L)) {
                    LoadMap(&background, "assets/maps/map.dat");
                }
            }

            // --- CHECK FOR PAUSE INPUT ---
            if (ShouldOpenPause()) {
                gameState = PAUSED;
                TraceLog(LOG_DEBUG, "Game paused!");
            }
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
            
            // --- ALLOW ESC TO RESUME GAME ---
            if (IsKeyPressed(KEY_ESCAPE)) gameState = PLAYING;
        }

        // --- DRAW ---
        BeginDrawing();
        ClearBackground(BLACK);
        
        Vector2 shakeOffset = (gameState == PLAYING) ? GetScreenShakeOffset(&cameraShake) : (Vector2){0, 0};
        if (shakeOffset.x != 0 || shakeOffset.y != 0) {
            BeginMode2D((Camera2D){
                .offset = {400, 300},
                .target = {400 + shakeOffset.x, 300 + shakeOffset.y},
                .rotation = 0,
                .zoom = 1.0f
            });
        }

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
            // --- DRAW BACKGROUND ---
            DrawBackground(&background);
            
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
            DrawParticles(particles, 200);
            DrawEnemies(&enemyPool);
            
            // --- DRAW MUZZLE FLASH ---
            for (int b = 0; b < bulletCount; b++) {
                if (bullets[b].muzzleFlashTime > 0.0f && bullets[b].muzzleFlashTime > 0.04f) {
                    Vector2 muzzlePos = bullets[b].trailPositions[0];
                    DrawCircleV(muzzlePos, 8.0f, Fade(ORANGE, 0.7f));
                    DrawCircleV(muzzlePos, 5.0f, Fade(YELLOW, 0.9f));
                }
            }
            
            // Draw safe zone (minimum shoot distance)
            DrawCircleLines(player.position.x, player.position.y, MIN_SHOOT_DISTANCE, Fade(RED, 0.3f));
        }
        else if (gameState == PAUSED) {
            // Draw game in background (paused)
            DrawBackground(&background);
            DrawPlayer(&player);
            DrawBullets(bullets, bulletCount);
            DrawEnemies(&enemyPool);
            DrawParticles(particles, 200);
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.7f));
            DrawPauseMenu(screenWidth, screenHeight, pauseMenuSelection);
        }

        // --- MAP EDITOR UI ---
        if (mapEditorMode) {
            // Draw semi-transparent overlay to indicate editor mode
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.1f));
            
            if (!collisionEditorMode) {
                // --- TILE EDITOR UI ---
                // Draw selected tile preview in top-left corner
                if (background.tileSet[currentSelectedTile].id != 0) {
                    DrawRectangle(10, 10, 100, 100, DARKGRAY);
                    DrawRectangleLines(10, 10, 100, 100, LIME);
                    Texture2D previewTex = background.tileSet[currentSelectedTile];
                    DrawTextureEx(previewTex, (Vector2){15, 15}, 0, 2.5f, WHITE);
                }
                
                // Draw tile index below preview
                DrawText(TextFormat("Tile: %d", currentSelectedTile), 10, 120, 16, LIME);
                
                // Draw tile editor controls
                DrawText("TILE EDITOR | Mouse Wheel - Select | LClick - Paint | RClick - Pick", 
                        10, screenHeight - 60, 12, YELLOW);
            } else {
                // --- COLLISION EDITOR UI ---
                DrawText("COLLISION EDITOR", 10, 10, 20, RED);
                DrawText("LClick - Block | RClick - Unblock", 10, 35, 16, YELLOW);
                
                // Draw collision layer visualization
                Vector2 mouseScreen = GetMousePosition();
                Vector2 mouseWorld = ScreenToWorldCoords(mouseScreen, background.position);
                int tileX, tileY;
                GetTileCoords(mouseWorld, &tileX, &tileY);
                
                // Draw all blocked tiles with semi-transparent red overlay
                int startTileX = (int)(background.position.x / TILE_SIZE);
                int startTileY = (int)(background.position.y / TILE_SIZE);
                int endTileX = startTileX + (screenWidth / TILE_SIZE) + 2;
                int endTileY = startTileY + (screenHeight / TILE_SIZE) + 2;
                
                if (startTileX < 0) startTileX = 0;
                if (startTileY < 0) startTileY = 0;
                if (endTileX >= MAP_WIDTH) endTileX = MAP_WIDTH - 1;
                if (endTileY >= MAP_HEIGHT) endTileY = MAP_HEIGHT - 1;
                
                for (int y = startTileY; y <= endTileY; y++) {
                    for (int x = startTileX; x <= endTileX; x++) {
                        if (GetCollision(&background, x, y)) {
                            Vector2 screenPos = {
                                x * TILE_SIZE - background.position.x,
                                y * TILE_SIZE - background.position.y
                            };
                            DrawRectangle(screenPos.x, screenPos.y, TILE_SIZE, TILE_SIZE, Fade(RED, 0.4f));
                        }
                    }
                }
            }
            
            // Draw shared editor controls
            DrawText("M - Toggle Editor | C - Toggle Mode | S - Save | L - Load", 
                    10, screenHeight - 30, 12, YELLOW);
        }

        if (shakeOffset.x != 0 || shakeOffset.y != 0) EndMode2D();

        // --- DRAW HUD (HEALTH BAR) - OUTSIDE CAMERA SHAKE ---
        if (gameState == PLAYING) {
            // Draw bullet counter
            int activeBullets = 0;
            for (int i = 0; i < bulletCount; i++) {
                if (bullets[i].active) activeBullets++;
            }
            DrawText(TextFormat("Bullets: %d/100", bulletCapacity), 10, screenHeight - 30, 20, LIME);
            
            // Draw reload status
            if (reloadTimer > 0) {
                float reloadPercent = (reloadTimer / reloadDuration) * 100.0f;
                DrawText(TextFormat("Reloading: %.0f%%", reloadPercent), 10, screenHeight - 60, 20, RED);
            }
            
            // Draw enemy count
            int activeEnemies = 0;
            for (int i = 0; i < enemyPool.count; i++) {
                if (enemyPool.enemies[i].active) activeEnemies++;
            }
            DrawText(TextFormat("Enemies: %d", activeEnemies), screenWidth - 120, 10, 20, LIME);
            
            // --- PLAYER HEALTH BAR (BOTTOM CENTER) ---
            float barWidth = 300.0f;
            float barHeight = 20.0f;
            float barX = screenWidth / 2.0f - barWidth / 2.0f;
            float barY = screenHeight - 50.0f;
            
            DrawRectangle(barX - 3, barY - 3, barWidth + 6, barHeight + 6, BLACK);
            float healthPercent = player.health / player.maxHealth;
            Color healthColor = (healthPercent > 0.5f) ? GREEN : (healthPercent > 0.25f) ? YELLOW : RED;
            DrawRectangle(barX, barY, barWidth * healthPercent, barHeight, healthColor);
            DrawText(TextFormat("HP: %.0f / %.0f", player.health, player.maxHealth), barX + 50, barY + 2, 16, WHITE);
        }

        EndDrawing();
    }

    // 3. De-Initialization
    UnloadBackground(&background);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
