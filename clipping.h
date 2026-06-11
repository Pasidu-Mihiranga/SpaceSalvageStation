#ifndef CLIPPING_H
#define CLIPPING_H

#include "structures.h"

// -------------------------------------------------------------
// MEMBER 5 ALGORITHM: CLIPPING & VIEWPORTS
// -------------------------------------------------------------

// Configures and enables the scissor boundaries for the secondary viewport
void activateScannerViewport(const Scanner& s);

// Restores scissor test after viewport rendering is completed
void deactivateScannerViewport();

// Sets up a custom narrow perspective frustum clipping range
void configureScannerProjection(const Scanner& s);

// Enables a custom arbitrary clipping plane (slicing canopy cockpit canopy)
void enableCustomClipPlane();

// Disables the custom arbitrary clipping plane
void disableCustomClipPlane();

#endif
