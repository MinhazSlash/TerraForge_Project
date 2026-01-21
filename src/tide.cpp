#include "../include/TerraForge.h"
#include <algorithm>

// Keep the original fixed water level value, but expose symbol for modularity
float WATER_LEVEL = 1.8f;

// Tide control helpers
void increaseWaterLevel(float delta)
{
    WATER_LEVEL = std::min(WATER_LEVEL + delta, 12.0f);
}

void decreaseWaterLevel(float delta)
{
    WATER_LEVEL = std::max(WATER_LEVEL - delta, -6.0f);
}

// No additional logic here to preserve original behavior; functions above allow runtime control.
