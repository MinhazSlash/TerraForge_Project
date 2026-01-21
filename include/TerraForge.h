#pragma once

// Central header for TerraForge split project

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <vector>

// ----------------- Configuration -----------------
static const int WIN_W = 1920;
static const int WIN_H = 1080;
static const int TERRAIN_RES = 128;
static const float CELL_SIZE = 0.8f;
static const int RAIN_COUNT = 4000;

// Water level is provided by tide module (default same as original)
extern float WATER_LEVEL;

// Globals
extern float heightMap[TERRAIN_RES][TERRAIN_RES];
extern Camera camera;
extern std::vector<Vector3> rainDrops;

// Time state
enum TimeState { STATE_DAY, STATE_SUNSET, STATE_NIGHT };
extern TimeState currentTimeState;

// Weather flags
extern bool isRaining;
extern bool isFoggy;

// Rendering
extern RenderTexture2D reflectionTarget;
extern Shader terrainShader;
extern const char *vsSource;
extern const char *fsSource;

// Utility
float smoothstep(float edge0, float edge1, float x);

// Terrain API
void initPerm();
void generateTerrain();
Vector3 computeNormalAt(int ix, int iz);
void drawSceneTerrain(bool useShader);

// Rain API
void initRain();
void updateRain(float dt);

// Shader API
void updateShaderUniforms(Color currentFogColor);
void DrawSkyGradient(Color top, Color bottom);

// Camera API
void setupCamera();
void updateCameraControls();


// Tide control API
void increaseWaterLevel(float delta);
void decreaseWaterLevel(float delta);
