#include "translation.h"

// 3D Translation Implementation

void applyTranslation(float tx, float ty, float tz) {
    // Translate the matrix using OpenGL glTranslatef
    glTranslatef(tx, ty, tz);
}

void translateDrone(Drone& d, float dx, float dy, float dz) {
    // Modify the coordinates of the drone in 3D coordinate space
    d.position.x += dx;
    d.position.y += dy;
    d.position.z += dz;

    // Boundary clipping to make sure drone stays inside the map
    d.position.x = fmaxf(-18.0f, fminf(18.0f, d.position.x));
    d.position.y = fmaxf(-8.0f, fminf(8.0f, d.position.y));
    d.position.z = fmaxf(-18.0f, fminf(18.0f, d.position.z));
}

void translateCarriedPart(Drone& d, SpacePart& part, float radYaw, float zoomFactor, bool isInspecting) {
    // Position the carried part slightly in front of the drone cockpit view
    float offsetDist = 2.4f;
    float targetX = d.position.x - sin(radYaw) * offsetDist;
    float targetY = d.position.y - 0.5f;
    float targetZ = d.position.z - cos(radYaw) * offsetDist;

    // Linear Interpolation (Lerp) to make the part follow the drone position smoothly
    part.position.x += (targetX - part.position.x) * 0.22f;
    part.position.y += (targetY - part.position.y) * 0.22f;
    part.position.z += (targetZ - part.position.z) * 0.22f;
}
