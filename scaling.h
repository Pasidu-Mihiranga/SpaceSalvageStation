#ifndef SCALING_H
#define SCALING_H

#include "structures.h"

// 3D Scaling

// Scale the matrix using OpenGL glScalef
void applyScaling(float sx, float sy, float sz);

// Linear interpolation (Lerp) to animate the zoom factor between 1.0x and 2.2x
void updateInspectZoom(bool isInspecting, float& zoomLerp, float& zoomFactor);

#endif
