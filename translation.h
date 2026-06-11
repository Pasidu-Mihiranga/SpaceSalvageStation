#ifndef TRANSLATION_H
#define TRANSLATION_H

#include "structures.h"

// -------------------------------------------------------------
// MEMBER 1 ALGORITHM: 3D TRANSLATION
// -------------------------------------------------------------

// Helper wrapper around glTranslatef
void applyTranslation(float tx, float ty, float tz);

// Moves the drone in 3D coordinate space based on speed and direction inputs
void translateDrone(Drone& d, float dx, float dy, float dz);

// Calculates and interpolates the position of the carried space debris ahead of the drone
void translateCarriedPart(Drone& d, SpacePart& part, float radYaw, float zoomFactor, bool isInspecting);

#endif
