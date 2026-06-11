#include "scaling.h"

// -------------------------------------------------------------
// MEMBER 3 ALGORITHM: 3D SCALING IMPLEMENTATION
// -------------------------------------------------------------

void applyScaling(float sx, float sy, float sz) {
    // Standard OpenGL scaling call
    glScalef(sx, sy, sz);
}

void updateInspectZoom(bool isInspecting, float& zoomLerp, float& zoomFactor) {
    // Member 3 Scaling animation loop
    if (isInspecting) {
        // Enlarge zoom factor: Lerps scale upwards to 2.2x
        zoomLerp = fminf(1.0f, zoomLerp + 0.05f);
    } else {
        // Retract zoom factor: Lerps scale back down to normal (1.0x)
        zoomLerp = fmaxf(0.0f, zoomLerp - 0.05f);
    }
    
    // Linear scale translation
    zoomFactor = 1.0f + (1.2f * zoomLerp);
}
