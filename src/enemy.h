#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"

#define MAX_ENEMIES 50
#define SLIME_FRAMES 4
#define SLIME_SPEED 1.5f
#define SLIME_WANDER_TIMER 2.0f
#define KNOCKBACK_STRENGTH 3.0f
#define KNOCKBACK_DECAY 0.95f
#define SLIME_DAMAGE_MIN 5.0f
#define SLIME_DAMAGE_MAX 15.0f

typedef enum {
    ENEMY_SLIME
} EnemyType;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Vector2 knockbackVelocity;  // Knockback force
    float speed;
    float radius;
    float health;
    float maxHealth;
    bool active;
    EnemyType type;
    Texture2D idleTextures[SLIME_FRAMES];
    float animationCounter;
    float wanderTimer;  // Timer for wandering behavior
    Vector2 wanderDirection;  // Current wander direction
    float scale;  // For flipping: 1.0f = normal, -1.0f = flipped
    Color color;
    float damageMin;    // Minimum damage dealt
    float damageMax;    // Maximum damage dealt
    float lastDamageTime; // Cooldown to prevent multiple hits per frame
} Enemy;

typedef struct {
    Enemy enemies[MAX_ENEMIES];
    int count;
} EnemyPool;

EnemyPool InitEnemyPool(void);
void SpawnSlime(EnemyPool* pool, Vector2 position);
void UpdateEnemies(EnemyPool* pool, Vector2 playerPos, int screenWidth, int screenHeight);
void DrawEnemies(EnemyPool* pool);
void DamageEnemy(EnemyPool* pool, int index, float damage);
void UpdateEnemyMovement(Enemy* enemy, Vector2 playerPos, int screenWidth, int screenHeight);
void TakeDamage(Enemy* enemy, float damage, Vector2 knockbackDirection);
void ApplyKnockback(Enemy* enemy, Vector2 direction);
float GetRandomDamage(Enemy* enemy);

#endif
