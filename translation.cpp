#include "translation.h"

// -------------------------------------------------------------
// MEMBER 1 ALGORITHM: 3D TRANSLATION IMPLEMENTATION
// -------------------------------------------------------------

void applyTranslation(float tx, float ty, float tz) {
    // Standard OpenGL translation matrix call
    glTranslatef(tx, ty, tz);
}

void translateDrone(Drone& d, float dx, float dy, float dz) {
    // Shift drone position coordinates in 3D space
    d.position.x += dx;
    d.position.y += dy;
    d.position.z += dz;

    // Bounds clipping to keep drone in salvage boundaries
    d.position.x = fmaxf(-18.0f, fminf(18.0f, d.position.x));
    d.position.y = fmaxf(-8.0f, fminf(8.0f, d.position.y));
    d.position.z = fmaxf(-18.0f, fminf(18.0f, d.position.z));
}

void translateCarriedPart(Drone& d, SpacePart& part, float radYaw, float zoomFactor, bool isInspecting) {
    // Calculates a translation target directly in front of the drone's cockpit visor
    float offsetDist = 2.4f;
    float targetX = d.position.x - sin(radYaw) * offsetDist;
    float targetY = d.position.y - 0.5f;
    float targetZ = d.position.z - cos(radYaw) * offsetDist;

    // Smooth linear interpolation (lerp translation) over frame updates
    part.position.x += (targetX - part.position.x) * 0.22f;
    part.position.y += (targetY - part.position.y) * 0.22f;
    part.position.z += (targetZ - part.position.z) * 0.22f;
}
