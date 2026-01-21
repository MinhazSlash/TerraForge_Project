#include "../include/TerraForge.h"
#include <cmath>
#include <random>
#include <algorithm>

// RNG for perm
static std::mt19937 rng(1337);
static std::uniform_real_distribution<float> unif01(0.0f,1.0f);

// Perlin perm table
static int perm[512];

// Heightmap defined in header
float heightMap[TERRAIN_RES][TERRAIN_RES];

void initPerm()
{
    std::uniform_int_distribution<int> dist(0,255);
    int p[256];
    for (int i=0;i<256;i++) p[i]=i;
    for (int i=255;i>0;i--)
    {
        int j = dist(rng) % (i+1);
        std::swap(p[i], p[j]);
    }
    for (int i=0;i<512;i++) perm[i]=p[i & 255];
}

static float fadef(float t){ return t*t*t*(t*(t*6-15)+10); }
static float lerpf(float a,float b,float t){ return a + t*(b-a); }
static float grad(int hash,float x,float y,float z){
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h==12||h==14 ? x : z);
    return (((h & 1) ? -u : u) + ((h & 2) ? -v : v));
}

float perlin(float x,float y,float z=0.0f)
{
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    int Z = (int)floor(z) & 255;
    x -= floor(x); y -= floor(y); z -= floor(z);
    float u = fadef(x), v = fadef(y), w = fadef(z);
    int A = perm[X] + Y, AA = perm[A] + Z, AB = perm[A+1] + Z;
    int B = perm[X+1] + Y, BA = perm[B] + Z, BB = perm[B+1] + Z;
    return lerpf(
        lerpf(lerpf(grad(perm[AA], x, y, z), grad(perm[BA], x-1, y, z), u),
              lerpf(grad(perm[AB], x, y-1, z), grad(perm[BB], x-1, y-1, z), u), v),
        lerpf(lerpf(grad(perm[AA+1], x, y, z-1), grad(perm[BA+1], x-1, y, z-1), u),
              lerpf(grad(perm[AB+1], x, y-1, z-1), grad(perm[BB+1], x-1, y-1, z-1), u), v), w);
}

// fbm parameters (matching original file)
static const float NOISE_SCALE = 0.055f;
static const int OCTAVES = 6;
static const float PERSISTENCE = 0.48f;
static const float LACUNARITY = 2.05f;

float fbm(float x,float y)
{
    float total=0.0f; float amplitude=1.0f; float frequency=NOISE_SCALE; float maxVal=0.0f;
    for (int i=0;i<OCTAVES;i++){
        total += perlin(x*frequency, y*frequency) * amplitude;
        maxVal += amplitude;
        amplitude *= PERSISTENCE;
        frequency *= LACUNARITY;
    }
    return total / maxVal;
}

float smoothstep(float edge0,float edge1,float x)
{
    float t = std::max(0.0f, std::min(1.0f, (x-edge0)/(edge1-edge0)));
    return t*t*(3.0f - 2.0f*t);
}

void generateTerrain()
{
    for (int x=0;x<TERRAIN_RES;x++){
        for (int z=0;z<TERRAIN_RES;z++){
            float nx = x - TERRAIN_RES/2.0f;
            float nz = z - TERRAIN_RES/2.0f;
            float h = fbm(nx, nz) * 16.0f;
            if (h>0) h = pow(h,1.15f);
            float dx = nx / (TERRAIN_RES * 0.45f);
            float dz = nz / (TERRAIN_RES * 0.45f);
            float dist = sqrt(dx*dx + dz*dz);
            float falloff = 1.0f - smoothstep(0.7f, 1.0f, dist);
            falloff = std::max(0.0f, std::min(1.0f, falloff));
            h *= falloff; h -= 2.0f;
            heightMap[x][z] = h;
        }
    }
}

Vector3 computeNormalAt(int ix,int iz)
{
    float l = (ix>0) ? heightMap[ix-1][iz] : heightMap[ix][iz];
    float r = (ix < TERRAIN_RES-1) ? heightMap[ix+1][iz] : heightMap[ix][iz];
    float d = (iz>0) ? heightMap[ix][iz-1] : heightMap[ix][iz];
    float u = (iz < TERRAIN_RES-1) ? heightMap[ix][iz+1] : heightMap[ix][iz];
    Vector3 n = {l - r, 2.0f * CELL_SIZE, d - u};
    return Vector3Normalize(n);
}

void drawSceneTerrain(bool useShader)
{
    if (useShader) BeginShaderMode(terrainShader);
    rlPushMatrix();
    rlBegin(RL_TRIANGLES);
    for (int x=0;x<TERRAIN_RES-1;x++){
        for (int z=0;z<TERRAIN_RES-1;z++){
            float xPos = (x - TERRAIN_RES/2.0f) * CELL_SIZE;
            float zPos = (z - TERRAIN_RES/2.0f) * CELL_SIZE;
            float xPosN = ((x+1) - TERRAIN_RES/2.0f) * CELL_SIZE;
            float zPosN = ((z+1) - TERRAIN_RES/2.0f) * CELL_SIZE;

            Vector3 v0 = {xPos, heightMap[x][z], zPos};
            Vector3 v1 = {xPosN, heightMap[x+1][z], zPos};
            Vector3 v2 = {xPosN, heightMap[x+1][z+1], zPosN};
            Vector3 v3 = {xPos, heightMap[x][z+1], zPosN};

            Vector3 n0 = computeNormalAt(x, z);
            Vector3 n1 = computeNormalAt(x+1, z);
            Vector3 n2 = computeNormalAt(x+1, z+1);
            Vector3 n3 = computeNormalAt(x, z+1);

            rlNormal3f(n0.x, n0.y, n0.z);
            rlVertex3f(v0.x, v0.y, v0.z);
            rlNormal3f(n1.x, n1.y, n1.z);
            rlVertex3f(v1.x, v1.y, v1.z);
            rlNormal3f(n2.x, n2.y, n2.z);
            rlVertex3f(v2.x, v2.y, v2.z);

            rlNormal3f(n0.x, n0.y, n0.z);
            rlVertex3f(v0.x, v0.y, v0.z);
            rlNormal3f(n2.x, n2.y, n2.z);
            rlVertex3f(v2.x, v2.y, v2.z);
            rlNormal3f(n3.x, n3.y, n3.z);
            rlVertex3f(v3.x, v3.y, v3.z);
        }
    }
    rlEnd();
    rlPopMatrix();
    if (useShader) EndShaderMode();
}
