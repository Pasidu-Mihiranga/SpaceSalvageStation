#include "rotation.h"

// 3D Rotation Implementation

void applyRotation(float angle, float rx, float ry, float rz) {
    // Standard OpenGL rotation matrix transformation
    glRotatef(angle, rx, ry, rz);
}

void rotatePartManual(SpacePart& part, bool keyLeft, bool keyRight, bool keyUp, bool keyDown, float rotSpeed) {
    // Change rotation angles based on manual user keystrokes
    if (keyLeft) {
        part.rotation.y += rotSpeed;
    }
    if (keyRight) {
        part.rotation.y -= rotSpeed;
    }
    if (keyUp) {
        part.rotation.x += rotSpeed;
    }
    if (keyDown) {
        part.rotation.x -= rotSpeed;
    }
    if (keyStates['u'] || keyStates['U']) {
        part.rotation.z += rotSpeed;
    }
    if (keyStates['o'] || keyStates['O']) {
        part.rotation.z -= rotSpeed;
    }

    // Keep angles within [-360, 360] range to avoid floating-point overflow and precision issues
    if (part.rotation.x > 360.0f) part.rotation.x -= 360.0f;
    if (part.rotation.x < -360.0f) part.rotation.x += 360.0f;
    if (part.rotation.y > 360.0f) part.rotation.y -= 360.0f;
    if (part.rotation.y < -360.0f) part.rotation.y += 360.0f;
    if (part.rotation.z > 360.0f) part.rotation.z -= 360.0f;
    if (part.rotation.z < -360.0f) part.rotation.z += 360.0f;
}

void rotateDebrisFloating(SpacePart& part) {
    // Slow rotational drift around multiple axes for background/floating parts
    part.rotation.y += part.spinSpeed;
    part.rotation.x += part.spinSpeed * 0.2f;

    // Wrap angles around 360 degrees
    if (part.rotation.y > 360.0f) part.rotation.y -= 360.0f;
    if (part.rotation.x > 360.0f) part.rotation.x -= 360.0f;
}
