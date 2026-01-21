#include "../include/TerraForge.h"
#include <iostream>

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(WIN_W, WIN_H, "TerraForge Pro [Split Edition]");

    initPerm();
    generateTerrain();
    initRain();

    setupCamera();
    DisableCursor();

    reflectionTarget = LoadRenderTexture(WIN_W/2, WIN_H/2);
    terrainShader = LoadShaderFromMemory(vsSource, fsSource);

    float lastTime = GetTime();

    while (!WindowShouldClose())
    {
        float now = GetTime();
        float dt = now - lastTime;
        lastTime = now;

        updateCameraControls();

        // Inputs for Time of Day
        if (IsKeyPressed(KEY_ONE)) currentTimeState = STATE_DAY;
        if (IsKeyPressed(KEY_TWO)) currentTimeState = STATE_SUNSET;
        if (IsKeyPressed(KEY_THREE)) currentTimeState = STATE_NIGHT;

        // Inputs for Weather
        if (IsKeyPressed(KEY_R)) isRaining = !isRaining;
        if (IsKeyPressed(KEY_F)) isFoggy = !isFoggy;

        // Tide control (Z: lower, X: raise)
        if (IsKeyPressed(KEY_Z)) decreaseWaterLevel(0.2f);
        if (IsKeyPressed(KEY_X)) increaseWaterLevel(0.2f);

        static float waveTime = 0.0f;
        waveTime += dt;
        if (isRaining) updateRain(dt);

        // Calculate Sky Colors based on State
        Color skyTop, skyBot;
        Color rainTop = {50,55,60,255};
        Color rainBot = {80,85,90,255};

        switch (currentTimeState)
        {
            case STATE_DAY:
                skyTop = {70,130,220,255};
                skyBot = {180,220,255,255};
                break;
            case STATE_SUNSET:
                skyTop = {60,30,90,255};
                skyBot = {255,140,50,255};
                rainTop = {50,40,60,255};
                rainBot = {100,80,70,255};
                break;
            case STATE_NIGHT:
                // Slightly brighter night so scene details (and rain) remain visible
                skyTop = {18,20,40,255};
                skyBot = {35,45,70,255};
                rainTop = {40,45,55,255};
                rainBot = {80,90,100,255};
                break;
        }

        if (isRaining)
        {
            skyTop = ColorAlphaBlend(skyTop, rainTop, ColorAlpha(WHITE, 0.8f));
            skyBot = ColorAlphaBlend(skyBot, rainBot, ColorAlpha(WHITE, 0.8f));
        }

        updateShaderUniforms(skyBot);

        // Pass 1: Reflection
        BeginTextureMode(reflectionTarget);
        ClearBackground(skyTop);

        Camera refCam = camera;
        float distFromWater = camera.position.y - WATER_LEVEL;
        refCam.position.y = WATER_LEVEL - distFromWater;
        refCam.target.y = WATER_LEVEL - (camera.target.y - WATER_LEVEL);

        BeginMode3D(refCam);
        rlDisableBackfaceCulling();
        drawSceneTerrain(true);
        rlEnableBackfaceCulling();
        EndMode3D();
        EndTextureMode();

        // Pass 2: Main Scene
        BeginDrawing();
        ClearBackground(skyTop);

        rlMatrixMode(RL_PROJECTION);
        rlPushMatrix(); rlLoadIdentity();
        rlMatrixMode(RL_MODELVIEW);
        rlPushMatrix(); rlLoadIdentity();
        DrawSkyGradient(skyTop, skyBot);
        rlPopMatrix(); rlMatrixMode(RL_PROJECTION); rlPopMatrix(); rlMatrixMode(RL_MODELVIEW);

        BeginMode3D(camera);
        drawSceneTerrain(true);

        // Water Surface
        rlPushMatrix();
        rlEnableTexture(reflectionTarget.texture.id);
        rlSetTexture(reflectionTarget.texture.id);
        rlBegin(RL_QUADS);
        rlColor4ub(255,255,255,220);

        float wSize = TERRAIN_RES * CELL_SIZE * 0.5f;
        float uvOffset = waveTime * 0.02f;

        rlTexCoord2f(0.0f, 0.0f + uvOffset);
        rlVertex3f(-wSize, WATER_LEVEL, -wSize);
        rlTexCoord2f(1.0f, 0.0f + uvOffset);
        rlVertex3f(wSize, WATER_LEVEL, -wSize);
        rlTexCoord2f(1.0f, 1.0f + uvOffset);
        rlVertex3f(wSize, WATER_LEVEL, wSize);
        rlTexCoord2f(0.0f, 1.0f + uvOffset);
        rlVertex3f(-wSize, WATER_LEVEL, wSize);
        rlEnd();
        rlDisableTexture();
        rlPopMatrix();

        // Water Tint
        Color waterTint;
        if (currentTimeState == STATE_SUNSET) waterTint = {50,40,80,110};
        else if (currentTimeState == STATE_NIGHT) waterTint = {10,20,40,140};
        else waterTint = {0,70,140,110};
        DrawPlane((Vector3){0, WATER_LEVEL + 0.01f, 0}, (Vector2){wSize*2, wSize*2}, waterTint);

        // Rain
        if (isRaining)
        {
            // Draw primary rain streaks (brighter at night)
            rlBegin(RL_LINES);
            Color baseCol = (currentTimeState == STATE_NIGHT) ? Color{170,180,210,180} : Color{200,210,225,120};
            rlColor4ub(baseCol.r, baseCol.g, baseCol.b, baseCol.a);
            for (auto &r : rainDrops){
                if (Vector3Distance(camera.position, r) < 45.0f){
                    // main streak
                    rlVertex3f(r.x, r.y, r.z);
                    rlVertex3f(r.x, r.y + 0.7f, r.z);
                }
            }
            rlEnd();

            // Add a softer secondary pass for 'glow' to improve visibility at night
            rlBegin(RL_LINES);
            Color glowCol = (currentTimeState == STATE_NIGHT) ? Color{200,210,230,80} : Color{220,230,240,40};
            rlColor4ub(glowCol.r, glowCol.g, glowCol.b, glowCol.a);
            for (auto &r : rainDrops){
                if (Vector3Distance(camera.position, r) < 30.0f){
                    // shorter faint streak to suggest scattering
                    rlVertex3f(r.x, r.y + 0.15f, r.z);
                    rlVertex3f(r.x, r.y + 0.5f, r.z);
                }
            }
            rlEnd();
        }

        EndMode3D();

        DrawFPS(10,10);
        DrawText("1: Day | 2: Sunset | 3: Night", 10, 35, 20, WHITE);
        DrawText("R: Rain | F: Fog | WASD: Move", 10, 60, 20, WHITE);
        DrawText("Z: Tide Down | X: Tide Up", 10, 85, 20, WHITE);

        EndDrawing();
    }

    UnloadShader(terrainShader);
    UnloadRenderTexture(reflectionTarget);
    CloseWindow();
    return 0;
}
