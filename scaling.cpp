#include "scaling.h"

// 3D Scaling Implementation

void applyScaling(float sx, float sy, float sz) {
    // Call standard OpenGL scaling function to adjust size of elements on X, Y, Z axes
    glScalef(sx, sy, sz);
}

void updateInspectZoom(bool isInspecting, float& zoomLerp, float& zoomFactor) {
    // Zoom animation loop for inspect mode
    if (isInspecting) {
        // Increment zoomLerp value to transition scale up to 2.2x (maximum)
        zoomLerp = fminf(1.0f, zoomLerp + 0.05f);
    } else {
        // Decrement zoomLerp value to revert scale back to 1.0x (normal)
        zoomLerp = fmaxf(0.0f, zoomLerp - 0.05f);
    }
    
    // Calculate final zoom factor from linear interpolation (Lerp)
    // When zoomLerp = 0.0, zoomFactor = 1.0. When zoomLerp = 1.0, zoomFactor = 2.2.
    zoomFactor = 1.0f + (1.2f * zoomLerp);
}
