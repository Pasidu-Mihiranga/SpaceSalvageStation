#ifndef ZBUFFER_H
#define ZBUFFER_H

#include "structures.h"

// Z-Buffer (Depth Buffer)

// Enable or disable Z-buffer (depth testing) to show rendering order difference
void configureDepthEngine(bool enableZBuffer);

// Set up camera positions using gluLookAt for first person, third person, and top-down map view
void setupPrimaryCamera(int mode, const Drone& d, float radYaw, bool isLaunching, float launchTimer, Vec3& camPos, Vec3& camTarget);

#endif
