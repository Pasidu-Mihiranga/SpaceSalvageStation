#include "rotation.h"

// -------------------------------------------------------------
// MEMBER 2 ALGORITHM: 3D ROTATION IMPLEMENTATION
// -------------------------------------------------------------

void applyRotation(float angle, float rx, float ry, float rz) {
    // Standard OpenGL rotation call
    glRotatef(angle, rx, ry, rz);
}

void rotatePartManual(SpacePart& part, bool keyLeft, bool keyRight, bool keyUp, bool keyDown, float rotSpeed) {
    // Increments/Decrements rotation angles of the local transformation coordinates
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

    // Keep angles within [-360, 360] to prevent precision loss
    if (part.rotation.x > 360.0f) part.rotation.x -= 360.0f;
    if (part.rotation.x < -360.0f) part.rotation.x += 360.0f;
    if (part.rotation.y > 360.0f) part.rotation.y -= 360.0f;
    if (part.rotation.y < -360.0f) part.rotation.y += 360.0f;
    if (part.rotation.z > 360.0f) part.rotation.z -= 360.0f;
    if (part.rotation.z < -360.0f) part.rotation.z += 360.0f;
}

void rotateDebrisFloating(SpacePart& part) {
    // Slow drift rotation around multiple axes for floating orbital debris
    part.rotation.y += part.spinSpeed;
    part.rotation.x += part.spinSpeed * 0.2f;

    // Angle wrapping
    if (part.rotation.y > 360.0f) part.rotation.y -= 360.0f;
    if (part.rotation.x > 360.0f) part.rotation.x -= 360.0f;
}
