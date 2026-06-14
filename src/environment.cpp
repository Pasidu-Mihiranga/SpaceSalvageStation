#include "environment.h"
#include "models.h"
#include "../translation.h"
#include "../rotation.h"
#include "../scaling.h"
#include <GL/glut.h>
#include <cmath>

std::vector<Vec3> stars;
const int NUM_STARS = 400;

void initStars() {
    stars.clear();
    for (int i = 0; i < NUM_STARS; ++i) {
        float theta = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
        float phi = acos(2.0f * ((float)rand() / RAND_MAX) - 1.0f);
        float radius = 120.0f + ((float)rand() / RAND_MAX) * 80.0f; // Generate random positions in shell

        float x = radius * sin(phi) * cos(theta);
        float y = radius * sin(phi) * sin(theta);
        float z = radius * cos(phi);
        stars.push_back(Vec3(x, y, z));
    }
}


void drawStarfield() {
    glColor3f(0.8f, 0.8f, 0.9f);
    glPointSize(1.2f);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < stars.size(); ++i) {
        glVertex3f(stars[i].x, stars[i].y, stars[i].z);
    }
    glEnd();
}


// Drawing routine for the space station dock
void drawDock() {
    // Draw platform base
    glPushMatrix();
    glColor3f(0.22f, 0.25f, 0.28f);
    applyTranslation(0.0f, 0.5f, -4.0f);
    applyScaling(12.0f, 0.6f, 16.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw runway warning stripes
    glPushMatrix();
    glColor3f(0.85f, 0.75f, 0.1f);
    applyTranslation(-5.9f, 0.9f, -4.0f);
    applyScaling(0.3f, 0.3f, 16.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.85f, 0.75f, 0.1f);
    applyTranslation(5.9f, 0.9f, -4.0f);
    applyScaling(0.3f, 0.3f, 16.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw tower structure
    glPushMatrix();
    glColor3f(0.18f, 0.2f, 0.25f);
    applyTranslation(0.0f, 4.5f, -11.5f); // Back center
    applyScaling(1.2f, 8.0f, 1.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw command cabin cabin
    glPushMatrix();
    glColor3f(0.2f, 0.4f, 0.5f);
    applyTranslation(0.0f, 9.0f, -11.5f);
    applyScaling(2.0f, 1.5f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw radio antenna pole
    glDisable(GL_LIGHTING);
    glColor3f(0.7f, 0.7f, 0.7f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 9.75f, -11.5f);
    glVertex3f(0.0f, 12.0f, -11.5f);
    glEnd();

    // Rotate and draw radar dish
    float radarRot = gameState.elapsedTime * 60.0f;
    glPushMatrix();
    applyTranslation(0.0f, 10.0f, -11.5f);
    applyRotation(radarRot, 0.0f, 1.0f, 0.0f);
    applyRotation(-30.0f, 1.0f, 0.0f, 0.0f); // Tilted dish
    glColor3f(0.4f, 0.45f, 0.5f);
    glutSolidCone(0.8f, 0.5f, 8, 2);
    glPopMatrix();

    // Animate blinking red warning beacon
    float blink = sin(gameState.elapsedTime * 5.0f) * 0.5f + 0.5f;
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    applyTranslation(0.0f, 12.1f, -11.5f);
    glutSolidSphere(0.18f + (blink * 0.05f), 8, 8);
    glPopMatrix();
    glEnable(GL_LIGHTING);

    // Draw sequential runway guidance lights
    glDisable(GL_LIGHTING);
    for (int j = 0; j < 8; ++j) {
        float zPos = -11.0f + (j * 2.8f);
        float pulseSeed = sin(gameState.elapsedTime * 3.0f - (j * 0.5f)) * 0.5f + 0.5f;

        glColor3f(0.0f, 0.8f * pulseSeed, 0.2f * pulseSeed);
        glPushMatrix();
        applyTranslation(-5.7f, 0.82f, zPos);
        glutSolidSphere(0.12f, 6, 6);
        glPopMatrix();

        glPushMatrix();
        applyTranslation(5.7f, 0.82f, zPos);
        glutSolidSphere(0.12f, 6, 6);
        glPopMatrix();
    }
    glEnable(GL_LIGHTING);

    // 5. DOCK SLOT HOLOGRAPHIC MARKERS — shows WHERE to place parts
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (size_t i = 0; i < parts.size(); ++i) {
        if (parts[i].state != ATTACHED) {
            // Only highlight current level's target slots
            bool isTarget = (gameState.currentLevel == 1 && (int)i == 0) ||
                            (gameState.currentLevel == 2 && (int)i == 1) ||
                            (gameState.currentLevel == 3 && (int)i == 2) ||
                            (gameState.currentLevel == 4 && ((int)i == 3 || (int)i == 4));
            if (!isTarget) continue;

            float pulse = sin(gameState.elapsedTime * 3.0f + i * 1.5f) * 0.3f + 0.7f;

            glPushMatrix();
            applyTranslation(parts[i].dockSlot.x, parts[i].dockSlot.y, parts[i].dockSlot.z);

            // Draw holographic ghost template showing correct docking orientation
            glPushMatrix();
            applyRotation(parts[i].targetRotation.x, 1, 0, 0);
            applyRotation(parts[i].targetRotation.y, 0, 1, 0);
            applyRotation(parts[i].targetRotation.z, 0, 0, 1);
            applyScaling(parts[i].scale, parts[i].scale, parts[i].scale);

            // Temporarily set wireframe mode
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glColor4f(0.0f, 1.0f, 0.4f, 0.25f * pulse);
            drawPart(parts[i].id, false);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glPopMatrix();

            // Inner pulsing sphere core indicator
            glColor4f(0.0f, 0.9f, 0.4f, 0.12f * pulse);
            glutWireSphere(0.7f, 8, 8);

            // Rotating targeting ring around the slot
            glPushMatrix();
            applyRotation(gameState.elapsedTime * 45.0f + i * 90.0f, 0.0f, 1.0f, 0.0f);
            applyRotation(90.0f, 1.0f, 0.0f, 0.0f);
            glColor4f(0.0f, 1.0f, 0.3f, 0.35f * pulse);
            glutWireTorus(0.04f, 1.2f, 4, 16);
            glPopMatrix();

            // Vertical guide beam above slot
            glColor4f(0.0f, 0.9f, 0.4f, 0.12f);
            glLineWidth(1.5f);
            glBegin(GL_LINES);
            glVertex3f(0.0f, -0.5f, 0.0f);
            glVertex3f(0.0f, 6.0f, 0.0f);
            glEnd();

            // 3D label above the slot
            glColor3f(0.0f, 1.0f, 0.4f);
            renderText3D(-0.6f, 2.2f, 0.0f, "[DOCK SLOT]", GLUT_BITMAP_HELVETICA_10);
            renderText3D(-0.8f, 1.8f, 0.0f, parts[i].name.c_str(), GLUT_BITMAP_HELVETICA_10);

            glPopMatrix();
        }
    }
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}


// ASSEMBLED SHIP HULL & STRUCTURAL CHASSIS
// Draws connecting fuselage, wings, tail fins, and details
// between docked parts so they form one cohesive spacecraft.
void drawShipHull() {
    // Only draw hull when at least one part is docked
    int attachedCount = 0;
    for (int i = 0; i < 5; i++) {
        if (parts[i].state == ATTACHED) attachedCount++;
    }
    if (attachedCount < 1) return;

    // During launch, match vertical offset & scale of attached parts
    float yOff = 0.0f;
    float sc = 1.0f;
    if (gameState.isLaunching && gameState.launchTimer > 2.5f) {
        float lt = gameState.launchTimer - 2.5f;
        yOff = lt * lt * 1.8f;
        sc = fmaxf(0.0f, 1.0f - lt * 0.15f);
    }

    glPushMatrix();
    applyTranslation(0.0f, yOff, 0.0f);
    applyScaling(sc, sc, sc);

    // 1. CENTRAL FUSELAGE KEEL
    // Upper fuselage plate
    glColor3f(0.18f, 0.20f, 0.24f);
    glPushMatrix();
    applyTranslation(0.0f, 1.92f, -4.0f);
    applyScaling(0.88f, 0.07f, 7.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Lower fuselage plate (belly)
    glPushMatrix();
    applyTranslation(0.0f, 1.08f, -4.0f);
    applyScaling(0.88f, 0.07f, 7.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Left fuselage wall
    glColor3f(0.20f, 0.22f, 0.26f);
    glPushMatrix();
    applyTranslation(-0.44f, 1.50f, -4.0f);
    applyScaling(0.05f, 0.78f, 7.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Right fuselage wall
    glPushMatrix();
    applyTranslation(0.44f, 1.50f, -4.0f);
    applyScaling(0.05f, 0.78f, 7.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 2. NOSE FAIRING
    glColor3f(0.22f, 0.25f, 0.30f);
    glPushMatrix();
    applyTranslation(0.0f, 1.50f, -0.4f);
    glutSolidCone(0.42f, 1.4f, 10, 4);
    glPopMatrix();

    // Nose cone accent ring
    glColor3f(0.28f, 0.30f, 0.35f);
    glPushMatrix();
    applyTranslation(0.0f, 1.50f, -0.35f);
    glutSolidTorus(0.05f, 0.40f, 6, 12);
    glPopMatrix();

    // 3. TAIL SECTION
    // Vertical stabilizer (dorsal fin)
    glColor3f(0.16f, 0.18f, 0.22f);
    glPushMatrix();
    applyTranslation(0.0f, 2.85f, -7.25f);
    applyScaling(0.06f, 2.2f, 1.1f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Fin tip cap
    glColor3f(0.14f, 0.16f, 0.20f);
    glPushMatrix();
    applyTranslation(0.0f, 3.98f, -7.25f);
    applyScaling(0.06f, 0.12f, 0.55f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Left horizontal stabilizer
    glColor3f(0.18f, 0.20f, 0.24f);
    glPushMatrix();
    applyTranslation(-1.1f, 1.60f, -7.35f);
    applyScaling(2.2f, 0.05f, 0.75f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Right horizontal stabilizer
    glPushMatrix();
    applyTranslation(1.1f, 1.60f, -7.35f);
    applyScaling(2.2f, 0.05f, 0.75f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Ventral keel fin (small bottom fin)
    glColor3f(0.15f, 0.17f, 0.21f);
    glPushMatrix();
    applyTranslation(0.0f, 0.85f, -7.0f);
    applyScaling(0.05f, 0.55f, 0.7f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 4. WING PYLONS
    // Left wing pylon (to solar panel at x=-4)
    glColor3f(0.22f, 0.24f, 0.28f);
    glPushMatrix();
    applyTranslation(-2.1f, 1.78f, -4.0f);
    applyScaling(4.4f, 0.14f, 0.38f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Left pylon lower strut (angled brace)
    glColor3f(0.19f, 0.21f, 0.25f);
    glPushMatrix();
    applyTranslation(-1.5f, 1.42f, -3.55f);
    applyRotation(-10.0f, 0.0f, 0.0f, 1.0f);
    applyScaling(2.4f, 0.07f, 0.12f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Left pylon upper strut
    glPushMatrix();
    applyTranslation(-1.5f, 1.92f, -4.45f);
    applyRotation(10.0f, 0.0f, 0.0f, 1.0f);
    applyScaling(2.4f, 0.07f, 0.12f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Right wing pylon (to cargo at x=+4)
    glColor3f(0.22f, 0.24f, 0.28f);
    glPushMatrix();
    applyTranslation(2.1f, 1.78f, -4.0f);
    applyScaling(4.4f, 0.14f, 0.38f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Right pylon lower strut
    glColor3f(0.19f, 0.21f, 0.25f);
    glPushMatrix();
    applyTranslation(1.5f, 1.42f, -3.55f);
    applyRotation(10.0f, 0.0f, 0.0f, 1.0f);
    applyScaling(2.4f, 0.07f, 0.12f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Right pylon upper strut
    glPushMatrix();
    applyTranslation(1.5f, 1.92f, -4.45f);
    applyRotation(-10.0f, 0.0f, 0.0f, 1.0f);
    applyScaling(2.4f, 0.07f, 0.12f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 5. DORSAL SPINE RIDGE
    glColor3f(0.14f, 0.16f, 0.20f);
    glPushMatrix();
    applyTranslation(0.0f, 2.08f, -3.5f);
    applyScaling(0.20f, 0.18f, 5.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 6. FUSELAGE CROSS-FRAME RIBS
    glColor3f(0.24f, 0.26f, 0.30f);
    for (int rib = 0; rib < 8; rib++) {
        float zz = -0.5f - rib * 0.95f;
        glPushMatrix();
        applyTranslation(0.0f, 1.50f, zz);
        applyScaling(0.93f, 0.80f, 0.04f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // 7. ARMOR BULKHEADS
    glColor3f(0.26f, 0.28f, 0.32f);
    // Forward bulkhead
    glPushMatrix();
    applyTranslation(0.0f, 1.50f, -0.45f);
    applyScaling(0.95f, 0.78f, 0.06f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Mid bulkhead (between cockpit and fuel core)
    glPushMatrix();
    applyTranslation(0.0f, 1.50f, -2.5f);
    applyScaling(0.95f, 0.78f, 0.06f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Rear bulkhead (between fuel core and engine)
    glPushMatrix();
    applyTranslation(0.0f, 1.50f, -5.5f);
    applyScaling(0.95f, 0.78f, 0.06f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 8. ENGINE MOUNT COLLAR
    glColor3f(0.15f, 0.17f, 0.20f);
    glPushMatrix();
    applyTranslation(0.0f, 1.55f, -6.5f);
    glutSolidTorus(0.10f, 0.52f, 8, 16);
    glPopMatrix();

    // Second intake ring further back
    glPushMatrix();
    applyTranslation(0.0f, 1.55f, -7.0f);
    glutSolidTorus(0.08f, 0.60f, 8, 16);
    glPopMatrix();

    // 9. HULL PANEL LINES (Wireframe Detail)
    glDisable(GL_LIGHTING);
    glColor4f(0.0f, 0.65f, 0.85f, 0.22f);
    glLineWidth(1.0f);

    // Longitudinal top panel lines
    glBegin(GL_LINES);
    glVertex3f(0.0f, 1.97f, -0.4f);   glVertex3f(0.0f, 1.97f, -7.6f);
    glVertex3f(-0.36f, 1.97f, -0.4f);  glVertex3f(-0.36f, 1.97f, -7.6f);
    glVertex3f(0.36f, 1.97f, -0.4f);   glVertex3f(0.36f, 1.97f, -7.6f);
    glEnd();

    // Side panel lines
    glBegin(GL_LINES);
    glVertex3f(-0.48f, 1.72f, -0.4f);  glVertex3f(-0.48f, 1.72f, -7.6f);
    glVertex3f(0.48f, 1.72f, -0.4f);   glVertex3f(0.48f, 1.72f, -7.6f);
    glVertex3f(-0.48f, 1.28f, -0.4f);  glVertex3f(-0.48f, 1.28f, -7.6f);
    glVertex3f(0.48f, 1.28f, -0.4f);   glVertex3f(0.48f, 1.28f, -7.6f);
    glEnd();

    // Wing pylon edge highlights
    glColor4f(0.0f, 0.8f, 0.3f, 0.18f);
    glBegin(GL_LINES);
    glVertex3f(-0.48f, 1.85f, -3.80f); glVertex3f(-4.3f, 1.85f, -3.80f);
    glVertex3f(-0.48f, 1.85f, -4.20f); glVertex3f(-4.3f, 1.85f, -4.20f);
    glVertex3f(0.48f, 1.85f, -3.80f);  glVertex3f(4.3f, 1.85f, -3.80f);
    glVertex3f(0.48f, 1.85f, -4.20f);  glVertex3f(4.3f, 1.85f, -4.20f);
    glEnd();

    // 10. RUNNING LIGHTS & STROBES
    float navPulse = sin(gameState.elapsedTime * 3.5f) * 0.4f + 0.6f;

    // Port navigation light (GREEN - left wingtip)
    glColor3f(0.0f, 0.9f * navPulse, 0.0f);
    glPushMatrix();
    applyTranslation(-4.3f, 1.88f, -4.0f);
    glutSolidSphere(0.09f, 6, 6);
    glPopMatrix();

    // Starboard navigation light (RED - right wingtip)
    glColor3f(0.9f * navPulse, 0.0f, 0.0f);
    glPushMatrix();
    applyTranslation(4.3f, 1.88f, -4.0f);
    glutSolidSphere(0.09f, 6, 6);
    glPopMatrix();

    // Dorsal strobe (WHITE blink)
    float strobeBlink = sin(gameState.elapsedTime * 8.0f) > 0.7f ? 1.0f : 0.12f;
    glColor3f(strobeBlink, strobeBlink, strobeBlink);
    glPushMatrix();
    applyTranslation(0.0f, 2.20f, -3.0f);
    glutSolidSphere(0.07f, 6, 6);
    glPopMatrix();

    // Tail anti-collision beacon (AMBER)
    float tailBlink = sin(gameState.elapsedTime * 5.0f) > 0.0f ? 1.0f : 0.25f;
    glColor3f(1.0f * tailBlink, 0.5f * tailBlink, 0.0f);
    glPushMatrix();
    applyTranslation(0.0f, 4.06f, -7.25f);
    glutSolidSphere(0.10f, 6, 6);
    glPopMatrix();

    // Forward approach lights (dual CYAN)
    float fwdPulse = sin(gameState.elapsedTime * 2.0f) * 0.3f + 0.7f;
    glColor3f(0.0f, 0.7f * fwdPulse, 0.85f * fwdPulse);
    glPushMatrix();
    applyTranslation(-0.32f, 1.50f, -0.35f);
    glutSolidSphere(0.06f, 6, 6);
    glPopMatrix();
    glPushMatrix();
    applyTranslation(0.32f, 1.50f, -0.35f);
    glutSolidSphere(0.06f, 6, 6);
    glPopMatrix();

    glEnable(GL_LIGHTING);

    // 11. WING ROOT WARNING STRIPES
    glColor3f(0.85f, 0.75f, 0.1f);
    glPushMatrix();
    applyTranslation(-0.50f, 1.78f, -3.88f);
    applyScaling(0.12f, 0.12f, 0.55f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    applyTranslation(0.50f, 1.78f, -3.88f);
    applyScaling(0.12f, 0.12f, 0.55f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 12. HULL DESIGNATION MARKINGS
    // Ship registry text on fuselage side
    glDisable(GL_LIGHTING);
    glColor3f(0.6f, 0.65f, 0.7f);
    renderText3D(-0.43f, 1.65f, -2.8f, "SSV-07", GLUT_BITMAP_HELVETICA_10);
    renderText3D(0.47f, 1.65f, -2.8f, "SSV-07", GLUT_BITMAP_HELVETICA_10);
    glEnable(GL_LIGHTING);

    glPopMatrix(); // End launch offset transform
}


// DRONE VISUAL DESIGN
void drawDrone() {
    // 1. Drone Cockpit chassis sphere
    glColor3f(0.35f, 0.45f, 0.52f);
    glutSolidSphere(0.8f, 16, 16);

    // Front Visor / Sensor Core
    glColor3f(0.0f, 0.85f, 0.9f);
    glPushMatrix();
    applyTranslation(0.0f, 0.15f, -0.62f);
    glutSolidSphere(0.35f, 12, 12);
    glPopMatrix();

    // Headlight visual spotlight beam cone (semi-transparent)
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 0.85f, 0.08f + 0.12f * drone.thrusterIntensity);
    glPushMatrix();
    applyTranslation(0.0f, 0.15f, -0.65f);
    applyRotation(180.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCone(0.5f, 3.8f, 16, 4); // Volumetric beam
    glPopMatrix();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    // 2. Manipulator Robotic Arms
    glColor3f(0.2f, 0.25f, 0.3f);
    if (drone.carriedPart != -1) {
        // Extended grasping arms
        // Left arm clamp
        glPushMatrix();
        applyTranslation(0.75f, -0.2f, -0.4f);
        applyRotation(-25.0f, 0.0f, 1.0f, 0.0f);
        applyScaling(0.15f, 0.15f, 0.8f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Right arm clamp
        glPushMatrix();
        applyTranslation(-0.75f, -0.2f, -0.4f);
        applyRotation(25.0f, 0.0f, 1.0f, 0.0f);
        applyScaling(0.15f, 0.15f, 0.8f);
        glutSolidCube(1.0f);
        glPopMatrix();
    } else {
        // Folded searching arms (slow breathing swing)
        // Left Folded arm
        glPushMatrix();
        applyTranslation(0.75f, -0.2f, -0.1f);
        applyRotation(40.0f + drone.armRotation, 0.0f, 1.0f, 0.0f);
        applyScaling(0.15f, 0.15f, 0.6f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Right Folded arm
        glPushMatrix();
        applyTranslation(-0.75f, -0.2f, -0.1f);
        applyRotation(-40.0f - drone.armRotation, 0.0f, 1.0f, 0.0f);
        applyScaling(0.15f, 0.15f, 0.6f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // 3. Side Engine Mount Blocks
    glColor3f(0.18f, 0.22f, 0.26f);
    glPushMatrix();
    applyTranslation(0.85f, 0.0f, 0.1f);
    applyScaling(0.4f, 0.3f, 0.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    applyTranslation(-0.85f, 0.0f, 0.1f);
    applyScaling(0.4f, 0.3f, 0.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 4. Rear Engine cone nozzle
    glColor3f(0.12f, 0.12f, 0.15f);
    glPushMatrix();
    applyTranslation(0.0f, -0.3f, 0.7f);
    applyRotation(180.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCone(0.28f, 0.55f, 10, 10);
    glPopMatrix();

    // Glowing core flame cone inside engine
    if (drone.thrusterIntensity > 0.05f) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 0.45f * drone.thrusterIntensity, 0.05f);
        glPushMatrix();
        applyTranslation(0.0f, -0.3f, 0.8f);
        applyScaling(1.0f, 1.0f, drone.thrusterIntensity * 1.5f);
        glutSolidCone(0.16f, 0.6f, 8, 4);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }
}


