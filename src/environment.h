#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "../structures.h"
#include <vector>

extern std::vector<Vec3> stars;
extern const int NUM_STARS;

void initStars();
void drawStarfield();
void drawDock();
void drawShipHull();
void drawDrone();

#endif
