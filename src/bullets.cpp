#include "bullets.h"
#include "raymath.h"

void InitBulletPool(Bullet bullets[], int maxBullets) {
    for (int i = 0; i < maxBullets; i++) bullets[i].active = false;
}

void ShootBullet(Bullet bullets[], int maxBullets, Vector2 position, Vector2 direction, float speed, Color color) {
    for (int i = 0; i < maxBullets; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].position = position;
            bullets[i].radius = 3.0f;
            bullets[i].color = color;
            bullets[i].damage = 10.0f;  // Default damage
            
            // Calculate direction towards target (direction is the target position)
            Vector2 dirToTarget = Vector2Subtract(direction, position);
            Vector2 normalizedDir = Vector2Normalize(dirToTarget);
            bullets[i].velocity = Vector2Scale(normalizedDir, speed);
            break;
        }
    }
}

void UpdateBullets(Bullet bullets[], int maxBullets) {
    for (int i = 0; i < maxBullets; i++) {
        if (bullets[i].active) {
            bullets[i].position = Vector2Add(bullets[i].position, bullets[i].velocity);
            
            // Deactivate if off-screen
            if (bullets[i].position.x < 0 || bullets[i].position.x > 800 || 
                bullets[i].position.y < 0 || bullets[i].position.y > 600) {
                bullets[i].active = false;
            }
        }
    }
}

void DrawBullets(Bullet bullets[], int maxBullets) {
    for (int i = 0; i < maxBullets; i++) {
        if (bullets[i].active) {
            DrawCircleV(bullets[i].position, bullets[i].radius, bullets[i].color);
        }
    }
}

void SetBulletDamage(Bullet bullets[], int maxBullets, float damage) {
    for (int i = 0; i < maxBullets; i++) {
        bullets[i].damage = damage;
    }
}