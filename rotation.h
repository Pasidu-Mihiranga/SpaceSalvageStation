#ifndef ROTATION_H
#define ROTATION_H

#include "structures.h"

// 3D Rotation

// Rotate using OpenGL glRotatef function
void applyRotation(float angle, float rx, float ry, float rz);

// Rotate the carried part in pitch, yaw, and roll based on key inputs
void rotatePartManual(SpacePart& part, bool keyLeft, bool keyRight, bool keyUp, bool keyDown, float rotSpeed);

// Automatically rotate floating debris to simulate floating in space
void rotateDebrisFloating(SpacePart& part);

#endif
