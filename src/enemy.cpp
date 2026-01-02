#include "enemy.h"
#include "raymath.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <ctime>

extern Shader slimeShader;
extern int squashLoc;
extern Sound enemyDamageSound;

EnemyPool InitEnemyPool(void) {
    EnemyPool pool = {0};
    pool.count = 0;
    srand((unsigned int)time(NULL));  // Seed random number generator
    return pool;
}

void SpawnSlime(EnemyPool* pool, Vector2 position) {
    if (pool->count >= MAX_ENEMIES) return;
    
    Enemy slime = {0};
    slime.position = position;
    slime.speed = SLIME_SPEED;
    slime.radius = 15.0f;
    slime.health = 30.0f;
    slime.maxHealth = 30.0f;
    slime.active = true;
    slime.type = ENEMY_SLIME;
    slime.color = WHITE;
    slime.animationCounter = 0.0f;
    slime.wanderTimer = SLIME_WANDER_TIMER;
    slime.velocity = (Vector2){0, 0};
    slime.knockbackVelocity = (Vector2){0, 0};
    slime.wanderDirection = (Vector2){1, 0};
    slime.scale = 1.0f;
    slime.damageMin = SLIME_DAMAGE_MIN;
    slime.damageMax = SLIME_DAMAGE_MAX;
    slime.lastDamageTime = 0.0f;
    
    // Load slime animation frames
    for (int i = 0; i < SLIME_FRAMES; i++) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "assets/txtures/enemy/slime/Grass 1.png");
        slime.idleTextures[i] = LoadTexture(filepath);
        if (slime.idleTextures[i].id == 0) {
            TraceLog(LOG_WARNING, "Failed to load slime texture: %s", filepath);
        }
    }
    
    pool->enemies[pool->count] = slime;
    pool->count++;
}

float GetRandomDamage(Enemy* enemy) {
    // Generate random damage within the range
    float randomDamage = enemy->damageMin + (float)(rand() % (int)(enemy->damageMax - enemy->damageMin + 1));
    return randomDamage;
}

void ApplyKnockback(Enemy* enemy, Vector2 direction) {
    Vector2 normalizedDir = Vector2Normalize(direction);
    enemy->knockbackVelocity = Vector2Scale(normalizedDir, KNOCKBACK_STRENGTH);
}

void UpdateEnemyMovement(Enemy* enemy, Vector2 playerPos, int screenWidth, int screenHeight) {
    if (enemy->type == ENEMY_SLIME) {
        // --- 1. CALCULATE HOP PHASE ---
        // We use a sine wave to create a pulsing movement.
        // Higher multiplier (e.g., 6.0f) makes the slime hop faster.
        float hopCycle = sinf(enemy->animationCounter * 6.0f); 

        // Calculate direction toward player
        Vector2 dirToPlayer = Vector2Subtract(playerPos, enemy->position);
        float distToPlayer = Vector2Length(dirToPlayer);
        
        if (distToPlayer > 5.0f) {
            Vector2 normalizedDir = Vector2Normalize(dirToPlayer);
            
            // --- 2. MOVE ONLY DURING JUMP PHASE ---
            // If hopCycle is positive, the slime is in the "air" moving forward.
            if (hopCycle > 0.0f) {
                float jumpSpeed = enemy->speed * 1.8f; // Boost speed during the jump
                enemy->velocity = Vector2Scale(normalizedDir, jumpSpeed);
            } else {
                // If hopCycle is negative, the slime has "landed."
                // Apply heavy friction to simulate sticking to the ground.
                enemy->velocity = Vector2Scale(enemy->velocity, 0.82f); 
            }
            
            // Flip based on movement direction
            if (normalizedDir.x < 0) enemy->scale = -1.0f;
            else if (normalizedDir.x > 0) enemy->scale = 1.0f;
        } else {
            enemy->velocity = Vector2Scale(enemy->velocity, 0.8f);
        }
        
        // --- 3. APPLY FORCES WITH DELTA TIME ---
        // Decay knockback velocity
        enemy->knockbackVelocity = Vector2Scale(enemy->knockbackVelocity, KNOCKBACK_DECAY);
        
        // Combine forces and apply Delta Time for frame-rate independence.
        // We multiply by 60.0f to keep the scale consistent with your previous 60FPS balancing.
        Vector2 totalVelocity = Vector2Add(enemy->velocity, enemy->knockbackVelocity);
        enemy->position = Vector2Add(enemy->position, Vector2Scale(totalVelocity, GetFrameTime() * 60.0f));
        
        // --- 4. BOUNDS CHECK ---
        if (enemy->position.x < enemy->radius) enemy->position.x = enemy->radius;
        if (enemy->position.x > screenWidth - enemy->radius) enemy->position.x = screenWidth - enemy->radius;
        if (enemy->position.y < enemy->radius) enemy->position.y = enemy->radius;
        if (enemy->position.y > screenHeight - enemy->radius) enemy->position.y = screenHeight - enemy->radius;
    }
}

void TakeDamage(Enemy* enemy, float damage, Vector2 knockbackDirection) {
    if (!enemy->active) return;
    
    enemy->health -= damage;
    
    // Visual feedback - change color briefly
    enemy->color = RED;
    
    // --- PLAY ENEMY DAMAGE SOUND ---
    if (enemyDamageSound.frameCount > 0 && IsSoundValid(enemyDamageSound)) {
        PlaySound(enemyDamageSound);
        TraceLog(LOG_DEBUG, "Enemy damage sound played!");
    }
    
    // Apply knockback
    ApplyKnockback(enemy, knockbackDirection);
    
    // Deactivate if health reaches 0
    if (enemy->health <= 0) {
        enemy->active = false;
        enemy->health = 0;
    }
}

void UpdateEnemies(EnemyPool* pool, Vector2 playerPos, int screenWidth, int screenHeight) {
    // PASS 1: Update State, Animation, and Base Movement
    for (int i = 0; i < pool->count; i++) {
        if (!pool->enemies[i].active) continue; //
        
        Enemy* enemy = &pool->enemies[i];
        
        // 1. Update timers and animation
        enemy->animationCounter += GetFrameTime();
        if (enemy->lastDamageTime > 0.0f) {
            enemy->lastDamageTime -= GetFrameTime();
        }
        
        // 2. Fix Color Recovery: Move from RED (255,0,0) back to WHITE (255,255,255)
        // Instead of subtracting from Red, we add back to Green and Blue
        if (enemy->color.g < 255) {
            int recoveryRate = 10; 
            enemy->color.g = (enemy->color.g + recoveryRate > 255) ? 255 : enemy->color.g + recoveryRate;
            enemy->color.b = (enemy->color.b + recoveryRate > 255) ? 255 : enemy->color.b + recoveryRate;
        }
        
        // 3. Apply base movement
        UpdateEnemyMovement(enemy, playerPos, screenWidth, screenHeight);
    }
    
    // PASS 2: Separation Resolution and Final Bounds Check
    // We do this in a separate pass so separation doesn't get "overwritten" by movement
    for (int i = 0; i < pool->count; i++) {
        if (!pool->enemies[i].active) continue;
        
        Enemy* enemy = &pool->enemies[i];

        // Separation Logic (O(N^2))
        for (int j = i + 1; j < pool->count; j++) {
            if (!pool->enemies[j].active) continue;
            
            Enemy* neighbor = &pool->enemies[j];
            float distance = Vector2Distance(enemy->position, neighbor->position);
            float minDistance = enemy->radius + neighbor->radius + 2.0f; // Added small buffer
            
            if (distance < minDistance && distance > 0.0f) {
                Vector2 pushDir = Vector2Normalize(Vector2Subtract(enemy->position, neighbor->position));
                float overlap = minDistance - distance;
                
                // Use a scalar for smoother separation (0.5f means they share the push)
                float pushForce = overlap * 0.5f;
                
                enemy->position = Vector2Add(enemy->position, Vector2Scale(pushDir, pushForce));
                neighbor->position = Vector2Subtract(neighbor->position, Vector2Scale(pushDir, pushForce));
            }
        }

        // Final Constraint: Keep in screen bounds
        // This must happen AFTER separation to ensure enemies aren't pushed off-screen
        if (enemy->position.x < enemy->radius) enemy->position.x = enemy->radius;
        if (enemy->position.x > screenWidth - enemy->radius) enemy->position.x = screenWidth - enemy->radius;
        if (enemy->position.y < enemy->radius) enemy->position.y = enemy->radius;
        if (enemy->position.y > screenHeight - enemy->radius) enemy->position.y = screenHeight - enemy->radius;
    }
}

void DrawEnemies(EnemyPool* pool) {
    for (int i = 0; i < pool->count; i++) {
        if (!pool->enemies[i].active) continue;
        
        Enemy* enemy = &pool->enemies[i];
        
        if (enemy->type == ENEMY_SLIME) {
            // --- 1. CALCULATE SQUASH FACTOR ---
            float squash = sinf(enemy->animationCounter * 3.0f) * 0.15f;
            
            // --- 2. OUTER GLOW/MEMBRANE LAYER ---
            // Creates the translucent outer boundary
            DrawCircleGradient(enemy->position.x, enemy->position.y,
                              enemy->radius * 1.2f, Fade(GREEN, 0.2f), Fade(GREEN, 0.05f));
            
            // --- 3. TEXTURE LAYER WITH SHADER ---
            if (enemy->idleTextures[0].id != 0) {
                Texture2D tex = enemy->idleTextures[0];
                float diameter = enemy->radius * 2.0f;
                
                // Apply squash/stretch scaling
                float scaleX = (diameter / tex.width) * (1.0f + squash * 0.5f);
                float scaleY = (diameter / tex.height) * (1.0f - squash);
                
                Rectangle source = {0, 0, (float)tex.width, (float)tex.height};
                Rectangle dest = {
                    enemy->position.x,
                    enemy->position.y,
                    tex.width * scaleX * enemy->scale,
                    tex.height * scaleY
                };
                
                // Bottom-anchored origin
                Vector2 origin = {fabsf(dest.width) / 2.0f, dest.height};
                
                // Apply shader if available
                if (slimeShader.id != 0) {
                    BeginShaderMode(slimeShader);
                    SetShaderValue(slimeShader, squashLoc, &squash, SHADER_UNIFORM_FLOAT);
                    DrawTexturePro(tex, source, dest, origin, 0, enemy->color);
                    EndShaderMode();
                } else {
                    // Fallback without shader
                    DrawTexturePro(tex, source, dest, origin, 0, enemy->color);
                }
            } else {
                // Fallback: Draw circle if texture failed
                DrawCircleV(enemy->position, enemy->radius, enemy->color);
            }
            
            // --- 4. METABALL LAYER (Inner Blob Effect) ---
            // Multiple circles at different opacities create jelly look
            float innerRadius1 = enemy->radius * 0.7f;
            float innerRadius2 = enemy->radius * 0.4f;
            
            DrawCircle(enemy->position.x - enemy->radius * 0.3f, 
                      enemy->position.y - enemy->radius * 0.2f,
                      innerRadius1, Fade(GREEN, 0.15f));
            DrawCircle(enemy->position.x + enemy->radius * 0.2f,
                      enemy->position.y + enemy->radius * 0.1f,
                      innerRadius2, Fade(GREEN, 0.1f));
            
            // --- 5. DYNAMIC SHADOW (INVERSE TO SQUASH) ---
            // Shadow expands when slime compresses
            float shadowWidth = enemy->radius * (1.5f + squash);
            float shadowHeight = enemy->radius * 0.4f;
            float shadowOpacity = 0.4f - (squash * 0.1f);
            
            DrawEllipse(enemy->position.x, enemy->position.y + 2.0f,
                       shadowWidth, shadowHeight, Fade(BLACK, shadowOpacity));
            
            // --- 6. HEALTH BAR (POSITIONED RELATIVE TO RADIUS) ---
            if (enemy->health < enemy->maxHealth) {
                float barWidth = enemy->radius * 2.2f;
                float barHeight = 4.0f;
                float barX = enemy->position.x - barWidth / 2.0f;
                float barY = enemy->position.y - enemy->radius - 15.0f;
                
                // Background
                DrawRectangle(barX, barY, barWidth, barHeight, BLACK);
                // Health
                float healthPercent = enemy->health / enemy->maxHealth;
                DrawRectangle(barX, barY, barWidth * healthPercent, barHeight, GREEN);
            }
        }
    }
}

void DamageEnemy(EnemyPool* pool, int index, float damage) {
    if (index < 0 || index >= pool->count) return;
    
    Enemy* enemy = &pool->enemies[index];
    Vector2 knockbackDir = (Vector2){0, -1};
    TakeDamage(enemy, damage, knockbackDir);
}
