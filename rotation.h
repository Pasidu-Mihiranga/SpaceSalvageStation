#ifndef ROTATION_H
#define ROTATION_H

#include "structures.h"

// -------------------------------------------------------------
// MEMBER 2 ALGORITHM: 3D ROTATION
// -------------------------------------------------------------

// Helper wrapper around glRotatef
void applyRotation(float angle, float rx, float ry, float rz);

// Manually rotates the carried part in pitch and yaw axes (Arrow Keys input)
void rotatePartManual(SpacePart& part, bool keyLeft, bool keyRight, bool keyUp, bool keyDown, float rotSpeed);

// Automatically rotates floating unattached space junk (Debris spin effect)
void rotateDebrisFloating(SpacePart& part);

#endif
