#ifndef BULLETS_H
#define BULLETS_H
#define MAX_BULLETS 100

#include "raylib.h"

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Vector2 speed;
    float radius;
    bool active;
    Color color;
} Bullet;

void InitBulletPool(Bullet bullets[], int maxBullets);
void ShootBullet(Bullet bullets[], int maxBullets, Vector2 position, Vector2 direction, float speed, Color color);
void UpdateBullets(Bullet bullets[], int maxBullets);
void DrawBullets(Bullet bullets[], int maxBullets);

#endif