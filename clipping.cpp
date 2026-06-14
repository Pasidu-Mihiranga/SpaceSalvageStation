#include "clipping.h"

// Clipping Implementation

void activateScannerViewport(const Scanner& s) {
    // Map the normalized device coordinates to the mini-map rectangle viewport
    glViewport(s.viewportX, s.viewportY, s.viewportW, s.viewportH);

    // Enable scissor test to clip any pixels rendered outside the viewport box boundaries
    glEnable(GL_SCISSOR_TEST);
    glScissor(s.viewportX, s.viewportY, s.viewportW, s.viewportH);
}

void deactivateScannerViewport() {
    // Disable scissor testing to allow full-screen rendering again
    glDisable(GL_SCISSOR_TEST);
}

void configureScannerProjection(const Scanner& s) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Use nearClip and farClip in perspective projection to clip distant objects
    gluPerspective(45.0, (float)s.viewportW / s.viewportH, s.nearClip, s.farClip);
    glMatrixMode(GL_MODELVIEW);
}

void enableCustomClipPlane() {
    // Define an arbitrary clipping plane at y = 0.0 (pointing up) to slice cockpit and see inside components
    double planeEq[4] = {0.0, 1.0, 0.0, 0.0};
    glEnable(GL_CLIP_PLANE1);
    glClipPlane(GL_CLIP_PLANE1, planeEq);
}

void disableCustomClipPlane() {
    // Turn off the custom slice plane
    glDisable(GL_CLIP_PLANE1);
}
