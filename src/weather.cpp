#include "../include/TerraForge.h"
#include <random>

// Globals
std::vector<Vector3> rainDrops;

// Flags defined in header as extern; define them here
bool isRaining = false;
bool isFoggy = true;

static std::mt19937 rng_weather(1337);
static std::uniform_real_distribution<float> unif01(0.0f,1.0f);

void initRain()
{
    rainDrops.clear();
    rainDrops.reserve(RAIN_COUNT);
    for (int i=0;i<RAIN_COUNT;i++){
        float rx = (unif01(rng_weather)-0.5f) * TERRAIN_RES * CELL_SIZE;
        float rz = (unif01(rng_weather)-0.5f) * TERRAIN_RES * CELL_SIZE;
        float ry = unif01(rng_weather) * 25.0f + 5.0f;
        rainDrops.push_back({rx, ry, rz});
    }
}

void updateRain(float dt)
{
    for (auto &r : rainDrops){
        r.y -= 35.0f * dt;
        if (r.y <= WATER_LEVEL){
            r.x = camera.position.x + (unif01(rng_weather)-0.5f) * 40.0f;
            r.z = camera.position.z + (unif01(rng_weather)-0.5f) * 40.0f;
            r.y = camera.position.y + 10.0f + unif01(rng_weather) * 10.0f;
        }
    }
}
