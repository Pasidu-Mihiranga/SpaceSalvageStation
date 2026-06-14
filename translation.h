#ifndef TRANSLATION_H
#define TRANSLATION_H

#include "structures.h"

// 3D Translation

// Translate using OpenGL glTranslatef function
void applyTranslation(float tx, float ty, float tz);

// Move drone position in 3D space based on keystrokes
void translateDrone(Drone& d, float dx, float dy, float dz);

// Calculate position of carried part to make it follow drone smoothly using linear interpolation
void translateCarriedPart(Drone& d, SpacePart& part, float radYaw, float zoomFactor, bool isInspecting);

#endif
