#ifndef SCALING_H
#define SCALING_H

#include "structures.h"

// -------------------------------------------------------------
// MEMBER 3 ALGORITHM: 3D SCALING
// -------------------------------------------------------------

// Helper wrapper around glScalef
void applyScaling(float sx, float sy, float sz);

// Interpolates scaling ratios to create smooth diagnostic zoom inspect frames
void updateInspectZoom(bool isInspecting, float& zoomLerp, float& zoomFactor);

#endif
