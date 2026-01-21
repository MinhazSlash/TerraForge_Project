TerraForge Pro - Split Project

This project is a split version of the single-file TerraForge_Pro.cpp you provided.
Files are separated by functionality (terrain, shader, weather, camera, tide, main) to make
navigation and maintenance easier while preserving original behavior and visual quality.

Build (macOS example):
    make

Run:
    ./TerraForge_Pro

Controls:
    1: Day | 2: Sunset | 3: Night
    R: Toggle Rain
    F: Toggle Fog
    WASD / Mouse: Move camera (first-person)


Improvements added in this final split:
- Night lighting slightly brightened so terrain features remain visible.
- Rain rendering adjusted to be clearly visible at night (brighter streaks + subtle glow).
- Tide control added: Z (tide down), X (tide up).

Note: I kept all core visuals and shader code behavior identical where possible; changes are conservative and only tweak lighting, fog, and rain draw passes for better night readability.
