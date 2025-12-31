#include "bullets.h"
#include "raymath.h"
#include <cstdlib>

Sound gunShotSound = {0};
Sound gunReloadSound = {0};

void InitSounds(void) {
    gunShotSound = LoadSound("assets/sounds/mixkit-game-gun-shot-1662.mp3");
    if (gunShotSound.frameCount == 0) {
        TraceLog(LOG_ERROR, "FAILED to load gun shot sound!");
        TraceLog(LOG_ERROR, "Make sure file exists at: assets/sounds/mixkit-game-gun-shot-1662.mp3");
    } else {
        TraceLog(LOG_INFO, "Gun shot sound loaded successfully! Frames: %d", gunShotSound.frameCount);
        SetSoundVolume(gunShotSound, 0.7f);  // Set volume to 70%
    }
    
    gunReloadSound = LoadSound("assets/sounds/gunreload.mp3");
    if (gunReloadSound.frameCount == 0) {
        TraceLog(LOG_ERROR, "FAILED to load reload sound!");
    } else {
        TraceLog(LOG_INFO, "Reload sound loaded successfully!");
        SetSoundVolume(gunReloadSound, 0.7f);  // Set volume to 70%
    }
}

void InitBulletPool(Bullet bullets[], int maxBullets) {
    for (int i = 0; i < maxBullets; i++) {
        bullets[i].active = false;
        bullets[i].muzzleFlashTime = 0.0f;
        for (int j = 0; j < BULLET_TRAIL_LENGTH; j++) {
            bullets[i].trailPositions[j] = (Vector2){0, 0};
        }
        bullets[i].trailIndex = 0;
    }
}

void InitParticlePool(Particle particles[], int maxParticles) {
    for (int i = 0; i < maxParticles; i++) {
        particles[i].active = false;
    }
}

void ShootBullet(Bullet bullets[], int maxBullets, Vector2 position, Vector2 direction, float speed, Color color) {
    for (int i = 0; i < maxBullets; i++) {
        if (!bullets[i].active) {
            bullets[i].active = true;
            bullets[i].position = position;
            bullets[i].radius = 3.0f;
            bullets[i].color = color;
            bullets[i].damage = 10.0f;
            bullets[i].muzzleFlashTime = 0.05f;  // 50ms muzzle flash
            
            // Initialize trail
            for (int j = 0; j < BULLET_TRAIL_LENGTH; j++) {
                bullets[i].trailPositions[j] = position;
            }
            bullets[i].trailIndex = 0;
            
            // Calculate direction towards target
            Vector2 dirToTarget = Vector2Subtract(direction, position);
            Vector2 normalizedDir = Vector2Normalize(dirToTarget);
            bullets[i].velocity = Vector2Scale(normalizedDir, speed);
            
            // --- PLAY GUN SHOT SOUND WITH MULTI-CHANNEL LAYERING ---
            if (gunShotSound.frameCount > 0 && IsSoundValid(gunShotSound)) {
                // Add slight volume variation (0.65 - 0.75) for richer sound
                float volumeVariation = 0.65f + ((float)(rand() % 100) / 1000.0f);
                
                // Add slight pitch variation by adjusting playback (optional, requires audio processing)
                // For now, Raylib handles automatic channel mixing
                
                SetSoundVolume(gunShotSound, volumeVariation);
                PlaySound(gunShotSound);
                TraceLog(LOG_DEBUG, "Gun shot sound played! Volume: %.2f", volumeVariation);
            } else {
                TraceLog(LOG_WARNING, "Could not play gun sound");
            }
            
            break;
        }
    }
}

void UpdateBullets(Bullet bullets[], int maxBullets) {
    for (int i = 0; i < maxBullets; i++) {
        if (bullets[i].active) {
            // --- UPDATE TRAIL ---
            bullets[i].trailPositions[bullets[i].trailIndex] = bullets[i].position;
            bullets[i].trailIndex = (bullets[i].trailIndex + 1) % BULLET_TRAIL_LENGTH;
            
            // Update position
            bullets[i].position = Vector2Add(bullets[i].position, bullets[i].velocity);
            
            // Update muzzle flash
            if (bullets[i].muzzleFlashTime > 0.0f) {
                bullets[i].muzzleFlashTime -= GetFrameTime();
            }
            
            // Deactivate if off-screen
            if (bullets[i].position.x < -50 || bullets[i].position.x > GetScreenWidth() + 50 || 
                bullets[i].position.y < -50 || bullets[i].position.y > GetScreenHeight() + 50) {
                bullets[i].active = false;
            }
        }
    }
}

void UpdateParticles(Particle particles[], int maxParticles) {
    for (int i = 0; i < maxParticles; i++) {
        if (particles[i].active) {
            particles[i].lifetime -= GetFrameTime();
            particles[i].position = Vector2Add(particles[i].position, particles[i].velocity);
            
            if (particles[i].lifetime <= 0.0f) {
                particles[i].active = false;
            }
        }
    }
}

void DrawBullets(Bullet bullets[], int maxBullets) {
    for (int i = 0; i < maxBullets; i++) {
        if (bullets[i].active) {
            // --- DRAW BULLET TRAIL ---
            for (int j = 0; j < BULLET_TRAIL_LENGTH - 1; j++) {
                int currIdx = (bullets[i].trailIndex + j) % BULLET_TRAIL_LENGTH;
                int nextIdx = (bullets[i].trailIndex + j + 1) % BULLET_TRAIL_LENGTH;
                
                Vector2 currPos = bullets[i].trailPositions[currIdx];
                Vector2 nextPos = bullets[i].trailPositions[nextIdx];
                
                // Fade alpha based on trail position
                float alpha = 1.0f - ((float)j / (float)BULLET_TRAIL_LENGTH);
                Color trailColor = Fade(bullets[i].color, alpha * 0.6f);
                
                DrawLineEx(currPos, nextPos, 2.0f, trailColor);
            }
            
            // Draw bullet core
            DrawCircleV(bullets[i].position, bullets[i].radius, bullets[i].color);
        }
    }
}

void DrawParticles(Particle particles[], int maxParticles) {
    for (int i = 0; i < maxParticles; i++) {
        if (particles[i].active) {
            float alpha = particles[i].lifetime / particles[i].maxLifetime;
            Color particleColor = Fade(particles[i].color, alpha);
            DrawCircleV(particles[i].position, 2.0f, particleColor);
        }
    }
}

void SpawnImpactParticles(Particle particles[], int maxParticles, Vector2 position, Vector2 bulletVelocity) {
    Vector2 oppositeDir = Vector2Scale(bulletVelocity, -0.5f);
    
    int particleCount = 6 + (rand() % 3);  // 6-8 particles
    for (int i = 0; i < particleCount; i++) {
        for (int j = 0; j < maxParticles; j++) {
            if (!particles[j].active) {
                particles[j].active = true;
                particles[j].position = position;
                particles[j].maxLifetime = 0.3f + ((float)(rand() % 100) / 500.0f);
                particles[j].lifetime = particles[j].maxLifetime;
                
                // Spread particles in opposite direction
                float angle = ((float)i / (float)particleCount) * 2.0f * 3.14159f;
                float spread = 2.0f + ((float)(rand() % 100) / 50.0f);
                particles[j].velocity = Vector2{
                    cosf(angle) * spread + oppositeDir.x,
                    sinf(angle) * spread + oppositeDir.y
                };
                
                particles[j].color = YELLOW;
                break;
            }
        }
    }
}

void SetBulletDamage(Bullet bullets[], int maxBullets, float damage) {
    for (int i = 0; i < maxBullets; i++) {
        bullets[i].damage = damage;
    }
}

void TriggerScreenShake(CameraShake* shake, float intensity) {
    shake->shakeIntensity = intensity;
    shake->shakeDuration = 0.1f;  // 100ms shake
}

void UpdateScreenShake(CameraShake* shake) {
    if (shake->shakeDuration > 0.0f) {
        shake->shakeDuration -= GetFrameTime();
    }
}

Vector2 GetScreenShakeOffset(CameraShake* shake) {
    if (shake->shakeDuration <= 0.0f) return (Vector2){0, 0};
    
    float progress = 1.0f - (shake->shakeDuration / 0.1f);
    float intensity = shake->shakeIntensity * (1.0f - progress);
    
    return (Vector2){
        ((float)(rand() % 100) - 50.0f) / 50.0f * intensity,
        ((float)(rand() % 100) - 50.0f) / 50.0f * intensity
    };
}