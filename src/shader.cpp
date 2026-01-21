#include "../include/TerraForge.h"
#include <cmath>

// Globals
TimeState currentTimeState = STATE_DAY;

Shader terrainShader;
RenderTexture2D reflectionTarget; // defined here but overriden in main

const char *vsSource = R"GLSL(
#version 330
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;
layout(location = 2) in vec3 vertexNormal;

uniform mat4 mvp;
uniform mat4 matModel;

out vec3 vNormal;
out vec3 vPosition;
out float vSlope;

void main() {
    vec4 worldPos = matModel * vec4(vertexPosition, 1.0);
    vPosition = worldPos.xyz;
    vec3 N = normalize(mat3(matModel) * vertexNormal);
    vNormal = N;
    vSlope = 1.0 - N.y;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)GLSL";

const char *fsSource = R"GLSL(
#version 330
in vec3 vNormal;
in vec3 vPosition;
in float vSlope;
out vec4 fragColor;

uniform vec3 lightDir;
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 cameraPos;
uniform float fogDensity;
uniform vec3 fogColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-lightDir);
    float lam = max(dot(N, L), 0.0);
    
    vec3 colDeep = vec3(0.08, 0.06, 0.04);
    vec3 colGrass = vec3(0.15, 0.40, 0.10);
    vec3 colRock = vec3(0.38, 0.36, 0.34);
    vec3 colSnow = vec3(0.98, 0.98, 1.0);

    vec3 finalMat;
    float h = vPosition.y;

    float grassMix = smoothstep(-2.0, 1.5, h);
    finalMat = mix(colDeep, colGrass, grassMix);

    float slopeFactor = smoothstep(0.25, 0.55, vSlope);
    finalMat = mix(finalMat, colRock, slopeFactor);
    
    float snowLine = 6.0;
    float snowFactor = smoothstep(snowLine, snowLine + 2.0, h);
    snowFactor *= (1.0 - smoothstep(0.4, 0.6, vSlope));
    finalMat = mix(finalMat, colSnow, snowFactor);

    vec3 lighting = ambientColor + (diffuseColor * lam);
    vec3 surfaceColor = finalMat * lighting;
    
    float d = length(cameraPos - vPosition);
    float fogFactor = 1.0 - exp(-fogDensity * d * d * 0.4);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    fragColor = vec4(mix(surfaceColor, fogColor, fogFactor), 1.0);
}
)GLSL";

void updateShaderUniforms(Color currentFogColor)
{
    Vector3 sunDir;
    Vector3 ambient, diffuse;

    switch (currentTimeState)
    {
    case STATE_DAY:
        sunDir = {0.2f, 0.8f, 0.1f};
        ambient = {0.3f, 0.3f, 0.35f};
        diffuse = {0.9f, 0.9f, 0.85f};
        break;
    case STATE_SUNSET:
        sunDir = {0.8f, 0.1f, 0.2f};
        ambient = {0.35f, 0.22f, 0.25f};
        diffuse = {0.9f, 0.5f, 0.2f};
        break;
    case STATE_NIGHT:
        // Slightly brighter night lighting so features (and rain) remain visible
        sunDir = {-0.2f, 0.6f, -0.2f};
        ambient = {0.12f, 0.12f, 0.18f};
        diffuse = {0.32f, 0.32f, 0.40f};
        break;
    }

    if (isRaining)
    {
        Vector3 stormAmb = {0.2f, 0.2f, 0.25f};
        Vector3 stormDiff = {0.3f, 0.3f, 0.35f};
        ambient.x = ambient.x * 0.3f + stormAmb.x * 0.7f;
        ambient.y = ambient.y * 0.3f + stormAmb.y * 0.7f;
        ambient.z = ambient.z * 0.3f + stormAmb.z * 0.7f;
        diffuse.x = diffuse.x * 0.2f + stormDiff.x * 0.8f;
        diffuse.y = diffuse.y * 0.2f + stormDiff.y * 0.8f;
        diffuse.z = diffuse.z * 0.2f + stormDiff.z * 0.8f;
    }

    sunDir = Vector3Normalize(sunDir);

    // Adjust fog density to balance visibility: rain increases fog, but night keeps slightly higher ambient
    float fogD = isFoggy ? (isRaining ? 0.035f : (currentTimeState == STATE_NIGHT ? 0.0065f : 0.004f)) : 0.0005f;
    Vector3 fogVec = {currentFogColor.r/255.0f, currentFogColor.g/255.0f, currentFogColor.b/255.0f};

    SetShaderValue(terrainShader, GetShaderLocation(terrainShader, "lightDir"), &sunDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(terrainShader, GetShaderLocation(terrainShader, "ambientColor"), &ambient, SHADER_UNIFORM_VEC3);
    SetShaderValue(terrainShader, GetShaderLocation(terrainShader, "diffuseColor"), &diffuse, SHADER_UNIFORM_VEC3);
    SetShaderValue(terrainShader, GetShaderLocation(terrainShader, "cameraPos"), &camera.position, SHADER_UNIFORM_VEC3);
    SetShaderValue(terrainShader, GetShaderLocation(terrainShader, "fogDensity"), &fogD, SHADER_UNIFORM_FLOAT);
    SetShaderValue(terrainShader, GetShaderLocation(terrainShader, "fogColor"), &fogVec, SHADER_UNIFORM_VEC3);
}

void DrawSkyGradient(Color top, Color bottom)
{
    rlDisableDepthMask();
    rlDisableBackfaceCulling();
    rlBegin(RL_QUADS);
    rlColor4ub(top.r, top.g, top.b, 255);
    rlVertex3f(-1.0f, 1.0f, 0.999f);
    rlVertex3f(1.0f, 1.0f, 0.999f);
    rlColor4ub(bottom.r, bottom.g, bottom.b, 255);
    rlVertex3f(1.0f, -1.0f, 0.999f);
    rlVertex3f(-1.0f, -1.0f, 0.999f);
    rlEnd();
    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}
