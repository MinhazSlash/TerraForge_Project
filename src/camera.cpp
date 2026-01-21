#include "../include/TerraForge.h"

Camera camera;

void setupCamera()
{
    camera.position = {0.0f, 12.0f, 20.0f};
    camera.target = {0.0f, 2.0f, 0.0f};
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

void updateCameraControls()
{
    // Keep original behavior: first-person style camera from raylib
    UpdateCamera(&camera, CAMERA_FIRST_PERSON);
}
