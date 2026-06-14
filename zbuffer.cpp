#include "zbuffer.h"

// Z-Buffer Implementation

void configureDepthEngine(bool enableZBuffer) {
    if (enableZBuffer) {
        // Enable depth test comparison to render closest surfaces properly
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL); // Accept fragment if it is closer or equal to current depth
    } else {
        // Disable depth test to show visual overlapping issue (Painter's algorithm failure)
        glDisable(GL_DEPTH_TEST);
    }
}

void setupPrimaryCamera(int mode, const Drone& d, float radYaw, bool isLaunching, float launchTimer, Vec3& camPos, Vec3& camTarget) {
    if (isLaunching) {
        // Static camera viewing position for dramatic launch sequence
        float shake = 0.0f;
        if (launchTimer < 2.5f && launchTimer > 0.0f) {
            // Apply slight random camera shake offset
            shake = ((float)rand() / RAND_MAX - 0.5f) * 0.12f;
        }
        gluLookAt(-14.0f + shake, 6.0f + shake, 5.0f,
                  0.0f, 2.5f + shake, -4.0f,
                  0.0f, 1.0f, 0.0f);
        return;
    }

    switch (mode) {
        case 0: // Third Person Follow Camera
            // Calculate camera position and look-at target based on drone yaw angle
            camPos.x = d.position.x + sin(radYaw) * 6.5f;
            camPos.y = d.position.y + 2.8f;
            camPos.z = d.position.z + cos(radYaw) * 6.5f;

            camTarget.x = d.position.x - sin(radYaw) * 2.0f;
            camTarget.y = d.position.y - 0.3f;
            camTarget.z = d.position.z - cos(radYaw) * 2.0f;

            gluLookAt(camPos.x, camPos.y, camPos.z,
                      camTarget.x, camTarget.y, camTarget.z,
                      0.0f, 1.0f, 0.0f);
            break;

        case 1: // Top-Down Map view
            // Look straight down from above (Y coordinate high up)
            gluLookAt(0.0f, 26.0f, 0.001f,
                      0.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, -1.0f);
            break;

        case 2: // First Person Pilot View
            // Position camera exactly at the pilot seat facing forward
            camPos.x = d.position.x - sin(radYaw) * 0.7f;
            camPos.y = d.position.y;
            camPos.z = d.position.z - cos(radYaw) * 0.7f;

            camTarget.x = d.position.x - sin(radYaw) * 10.0f;
            camTarget.y = d.position.y;
            camTarget.z = d.position.z - cos(radYaw) * 10.0f;

            gluLookAt(camPos.x, camPos.y, camPos.z,
                      camTarget.x, camTarget.y, camTarget.z,
                      0.0f, 1.0f, 0.0f);
            break;
    }
}
