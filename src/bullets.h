#ifndef BULLETS_H
#define BULLETS_H
#define MAX_BULLETS 100
#define MAX_PARTICLES 200
#define BULLET_TRAIL_LENGTH 5

#include "raylib.h"

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Vector2 speed;
    float radius;
    float damage;  // Damage dealt to enemies
    bool active;
    Color color;
    Vector2 trailPositions[BULLET_TRAIL_LENGTH];
    int trailIndex;
    float muzzleFlashTime;  // Time remaining for muzzle flash
} Bullet;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float lifetime;
    float maxLifetime;
    Color color;
    bool active;
} Particle;

typedef struct {
    float shakeIntensity;
    float shakeDuration;
} CameraShake;

extern Sound gunShotSound;
extern Sound gunReloadSound;

void InitBulletPool(Bullet bullets[], int maxBullets);
void InitParticlePool(Particle particles[], int maxParticles);
void InitSounds(void);
void ShootBullet(Bullet bullets[], int maxBullets, Vector2 position, Vector2 direction, float speed, Color color);
void UpdateBullets(Bullet bullets[], int maxBullets);
void UpdateParticles(Particle particles[], int maxParticles);
void DrawBullets(Bullet bullets[], int maxBullets);
void DrawParticles(Particle particles[], int maxParticles);
void SetBulletDamage(Bullet bullets[], int maxBullets, float damage);
void SpawnImpactParticles(Particle particles[], int maxParticles, Vector2 position, Vector2 bulletVelocity);
void TriggerScreenShake(CameraShake* shake, float intensity);
void UpdateScreenShake(CameraShake* shake);
Vector2 GetScreenShakeOffset(CameraShake* shake);

#endif