#include "clipping.h"

// -------------------------------------------------------------
// MEMBER 5 ALGORITHM: CLIPPING IMPLEMENTATION
// -------------------------------------------------------------

void activateScannerViewport(const Scanner& s) {
    // Shifts target rendering zone to secondary quadrant
    glViewport(s.viewportX, s.viewportY, s.viewportW, s.viewportH);

    // Enables scissoring to restrict pixel output strictly within bounds
    glEnable(GL_SCISSOR_TEST);
    glScissor(s.viewportX, s.viewportY, s.viewportW, s.viewportH);
}

void deactivateScannerViewport() {
    // Restores default scissor test state
    glDisable(GL_SCISSOR_TEST);
}

void configureScannerProjection(const Scanner& s) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Creates a tight view frustum: objects beyond s.farClip are clipped out in real-time
    gluPerspective(45.0, (float)s.viewportW / s.viewportH, s.nearClip, s.farClip);
    glMatrixMode(GL_MODELVIEW);
}

void enableCustomClipPlane() {
    // Slices spheres/canopies along y = 0.0 plane to inspect interior components
    double planeEq[4] = {0.0, 1.0, 0.0, 0.0};
    glEnable(GL_CLIP_PLANE1);
    glClipPlane(GL_CLIP_PLANE1, planeEq);
}

void disableCustomClipPlane() {
    // Disable cockpit arbitrary slice plane
    glDisable(GL_CLIP_PLANE1);
}
