#ifndef ZBUFFER_H
#define ZBUFFER_H

#include "structures.h"

// -------------------------------------------------------------
// MEMBER 4 ALGORITHM: Z-BUFFER
// -------------------------------------------------------------

// Configures the depth testing attributes
void configureDepthEngine(bool enableZBuffer);

// Performs viewport camera matrix setup to align perspectives and targets
void setupPrimaryCamera(int mode, const Drone& d, float radYaw, bool isLaunching, float launchTimer, Vec3& camPos, Vec3& camTarget);

#endif
