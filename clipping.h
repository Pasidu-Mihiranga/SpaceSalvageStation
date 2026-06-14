#ifndef CLIPPING_H
#define CLIPPING_H

#include "structures.h"

// Clipping & Viewports

// Use glViewport and glScissor to configure and restrict drawing to the side map radar box
void activateScannerViewport(const Scanner& s);

// Turn off scissor testing after drawing the side map
void deactivateScannerViewport();

// Set up perspective projection with dynamic near/far clipping planes for scanner
void configureScannerProjection(const Scanner& s);

// Enable arbitrary user clipping plane to cut canopy and show interior components
void enableCustomClipPlane();

// Disable user clipping plane
void disableCustomClipPlane();

#endif
