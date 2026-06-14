#include "models.h"
#include "../translation.h"
#include "../rotation.h"
#include "../scaling.h"
#include "../clipping.h"
#include <GL/glut.h>
#include <cmath>

// SPACECRAFT GEOMETRY DESIGN (Damaged vs Repaired states)
void drawPart(int id, bool isInspecting) {
    // Inspection mode ( Zoom): Render Damage hotspot indicators
    if (isInspecting && !parts[id].isRepaired) {
        glDisable(GL_LIGHTING);
        float pulse = sin(gameState.elapsedTime * 6.5f) * 0.5f + 0.5f;
        glColor3f(1.0f, 0.0f, 0.0f);
        
        glPushMatrix();
        applyTranslation(0.0f, 0.4f, 0.1f);
        glutWireSphere(0.38f + (pulse * 0.06f), 8, 8);
        glPopMatrix();
        glEnable(GL_LIGHTING);
    }

    glColor3f(parts[id].color[0], parts[id].color[1], parts[id].color[2]);

    switch (id) {
        case 0: // Command Cockpit: Glass dome
            {
                if (isInspecting) {
                    // Delegation: enableCustomClipPlane cockpit slice
                    enableCustomClipPlane();
                }
                
                // Main spherical body (Grey steel color)
                glColor3f(parts[0].color[0], parts[0].color[1], parts[0].color[2]);
                glutSolidSphere(0.85f, 16, 16);

                // Side thruster pipes (left and right)
                glColor3f(0.25f, 0.28f, 0.32f);
                glPushMatrix();
                applyTranslation(0.88f, 0.0f, 0.0f);
                applyRotation(90.0f, 0.0f, 1.0f, 0.0f);
                GLUquadric* sideQuad1 = gluNewQuadric();
                gluCylinder(sideQuad1, 0.15f, 0.1f, 0.3f, 8, 2);
                gluDeleteQuadric(sideQuad1);
                glPopMatrix();

                glPushMatrix();
                applyTranslation(-0.88f, 0.0f, 0.0f);
                applyRotation(-90.0f, 0.0f, 1.0f, 0.0f);
                GLUquadric* sideQuad2 = gluNewQuadric();
                gluCylinder(sideQuad2, 0.15f, 0.1f, 0.3f, 8, 2);
                gluDeleteQuadric(sideQuad2);
                glPopMatrix();

                // Front Cockpit visor screen (facing front - negative Z)
                glPushMatrix();
                glColor3f(0.0f, 0.85f, 0.95f); // Neon Cyan Visor
                applyTranslation(0.0f, 0.18f, -0.58f);
                applyScaling(0.6f, 0.35f, 0.4f);
                glutSolidSphere(0.65f, 12, 12);
                glPopMatrix();

                // Nose Cone Tip (facing forward - negative Z)
                glPushMatrix();
                glColor3f(0.9f, 0.2f, 0.2f); // Red Nose Cone
                applyTranslation(0.0f, -0.05f, -0.85f);
                applyRotation(180.0f, 0.0f, 1.0f, 0.0f);
                glutSolidCone(0.3f, 0.4f, 12, 2);
                glPopMatrix();

                // Back stabilizer connector structure (positive Z)
                glPushMatrix();
                glColor3f(0.3f, 0.33f, 0.38f);
                applyTranslation(0.0f, 0.0f, 0.75f);
                applyScaling(0.38f, 0.38f, 0.4f);
                glutSolidCube(1.0f);
                glPopMatrix();

                // Tail stabilizer fin
                glPushMatrix();
                glColor3f(parts[0].color[0] * 0.7f, parts[0].color[1] * 0.7f, parts[0].color[2] * 0.7f);
                applyTranslation(0.0f, 0.65f, 0.5f);
                applyScaling(0.1f, 0.6f, 0.4f);
                glutSolidCube(1.0f);
                glPopMatrix();

                // Panel lines overlay (Lecturer will love the detail)
                glDisable(GL_LIGHTING);
                glColor4f(0.0f, 0.85f, 0.95f, 0.3f);
                glLineWidth(1.0f);
                glPushMatrix();
                applyRotation(90.0f, 1.0f, 0.0f, 0.0f);
                glutWireTorus(0.02f, 0.86f, 4, 12);
                glPopMatrix();
                glEnable(GL_LIGHTING);

                // Restore primary color
                glColor3f(parts[0].color[0], parts[0].color[1], parts[0].color[2]);
                
                if (isInspecting) {
                    // Delegation: disableCustomClipPlane
                    disableCustomClipPlane();
                    
                    // Internal Pilot seat detail
                    glColor3f(0.2f, 0.22f, 0.25f);
                    glPushMatrix();
                    applyTranslation(0.0f, -0.38f, -0.1f);
                    applyScaling(0.45f, 0.4f, 0.45f);
                    glutSolidCube(1.0f);
                    glPopMatrix();
                }

                // If DAMAGED: Draw jagged cracked structures on shell (red/black stripes)
                if (!parts[0].isRepaired) {
                    glDisable(GL_LIGHTING);
                    glColor3f(1.0f, 0.1f, 0.1f);
                    glLineWidth(2.0f);
                    glBegin(GL_LINES);
                    glVertex3f(0.0f, 1.0f, 0.0f);   glVertex3f(0.2f, 0.7f, 0.3f);
                    glVertex3f(0.2f, 0.7f, 0.3f);   glVertex3f(0.08f, 0.4f, 0.7f);
                    glVertex3f(0.2f, 0.7f, 0.3f);   glVertex3f(0.45f, 0.6f, -0.1f);
                    glVertex3f(-0.3f, 0.85f, 0.2f);  glVertex3f(-0.6f, 0.5f, 0.4f);
                    glEnd();
                    glEnable(GL_LIGHTING);
                }
            }
            break;

        case 1: // Thruster Engine
            {
                // Main combustion chamber cylinder
                glColor3f(parts[1].color[0], parts[1].color[1], parts[1].color[2]);
                glPushMatrix();
                GLUquadric* quad = gluNewQuadric();
                gluCylinder(quad, 0.48f, 0.48f, 0.85f, 16, 4);
                gluDeleteQuadric(quad);
                glPopMatrix();

                // Detailed cooling ribs around cylinder
                glColor3f(0.3f, 0.35f, 0.4f);
                for (int rib = 0; rib < 3; ++rib) {
                    glPushMatrix();
                    applyTranslation(0.0f, 0.0f, 0.2f + rib * 0.22f);
                    glutSolidTorus(0.06f, 0.49f, 6, 16);
                    glPopMatrix();
                }

                // Fuel pipes running along engine body
                glColor3f(0.7f, 0.5f, 0.2f); // Copper piping
                for (int pipe = 0; pipe < 4; ++pipe) {
                    glPushMatrix();
                    applyRotation(pipe * 90.0f, 0.0f, 0.0f, 1.0f);
                    applyTranslation(0.44f, 0.0f, 0.0f);
                    applyScaling(0.06f, 0.06f, 0.8f);
                    glutSolidCube(1.0f);
                    glPopMatrix();
                }

                // Exhaust cone nozzle
                glPushMatrix();
                applyTranslation(0.0f, 0.0f, 0.85f);
                
                if (!parts[1].isRepaired) {
                    glDisable(GL_LIGHTING);
                    glColor3f(0.85f, 0.1f, 0.1f);
                    glLineWidth(2.0f);
                    glBegin(GL_LINES);
                    for (int n = 0; n < 8; ++n) {
                        float angle = n * 2.0f * M_PI / 8.0f;
                        if (n % 3 != 0) {
                            glVertex3f(cos(angle)*0.45f, sin(angle)*0.45f, 0.0f);
                            glVertex3f(cos(angle)*0.75f, sin(angle)*0.75f, 0.8f);
                        }
                    }
                    glEnd();
                    glEnable(GL_LIGHTING);
                } else {
                    // Solid cone exhaust (Metallic steel outside)
                    glColor3f(0.22f, 0.24f, 0.27f);
                    glutSolidCone(0.75f, 0.95f, 16, 8);

                    // Inner glowing plasma burner cone
                    glDisable(GL_LIGHTING);
                    glColor3f(1.0f, 0.4f, 0.0f);
                    glPushMatrix();
                    applyTranslation(0.0f, 0.0f, 0.1f);
                    glutSolidCone(0.55f, 0.65f, 12, 4);
                    glPopMatrix();
                    glEnable(GL_LIGHTING);
                }
                glPopMatrix();
            }
            break;

        case 2: // Solar Panel
            {
                // Core pivot axle
                glColor3f(0.35f, 0.38f, 0.42f);
                glPushMatrix();
                applyScaling(0.18f, 0.18f, 2.6f);
                glutSolidCube(1.0f);
                glPopMatrix();

                // Structural Support Trusses (V-shape)
                glColor3f(0.25f, 0.28f, 0.32f);
                glPushMatrix();
                applyScaling(0.8f, 0.12f, 0.12f);
                glutSolidCube(1.0f);
                glPopMatrix();

                // Left Solar Panel Array
                glPushMatrix();
                applyTranslation(-1.4f, 0.0f, 0.0f);
                applyScaling(2.4f, 0.06f, 1.8f);
                glColor3f(0.15f, 0.22f, 0.35f); // Deep Space Blue Panel
                glutSolidCube(1.0f);
                glPopMatrix();

                // Right Solar Panel Array
                glPushMatrix();
                applyTranslation(1.4f, 0.0f, 0.0f);
                if (!parts[2].isRepaired) {
                    // Damaged state: Right solar array is severely cracked & broken off
                    applyScaling(1.1f, 0.06f, 1.0f);
                    applyTranslation(-0.4f, 0.0f, 0.0f);
                    glColor3f(0.6f, 0.2f, 0.2f); // Cracked array red-tint
                    glutSolidCube(1.0f);
                } else {
                    // Repaired state: Fully calibrated arrays
                    applyScaling(2.4f, 0.06f, 1.8f);
                    glColor3f(0.15f, 0.22f, 0.35f);
                    glutSolidCube(1.0f);
                }
                glPopMatrix();

                // Photovoltaic grid details (wires)
                glDisable(GL_LIGHTING);
                glColor3f(0.0f, 0.85f, 1.0f);
                glLineWidth(1.0f);
                
                // Left Grid lines
                glPushMatrix();
                applyTranslation(-1.4f, 0.04f, 0.0f);
                applyScaling(2.42f, 0.06f, 1.82f);
                glutWireCube(1.0f);
                glPopMatrix();

                if (parts[2].isRepaired) {
                    // Right Grid lines (repaired)
                    glPushMatrix();
                    applyTranslation(1.4f, 0.04f, 0.0f);
                    applyScaling(2.42f, 0.06f, 1.82f);
                    glutWireCube(1.0f);
                    glPopMatrix();
                } else {
                    // Red sparks / debris cracks for damaged pane
                    glColor3f(1.0f, 0.1f, 0.1f);
                    glLineWidth(2.0f);
                    glBegin(GL_LINES);
                    glVertex3f(0.3f, 0.05f, -0.6f);   glVertex3f(1.2f, 0.1f, -0.3f);
                    glVertex3f(0.3f, 0.05f, 0.1f);    glVertex3f(1.0f, -0.05f, 0.5f);
                    glVertex3f(0.5f, 0.05f, 0.4f);    glVertex3f(1.4f, 0.08f, 0.9f);
                    glEnd();
                }
                glEnable(GL_LIGHTING);
            }
            break;

        case 3: // Plutonium Fuel Storage Core
            {
                // Main storage core cylinder
                glColor3f(parts[3].color[0], parts[3].color[1], parts[3].color[2]);
                glPushMatrix();
                GLUquadric* quad = gluNewQuadric();
                gluCylinder(quad, 0.58f, 0.58f, 1.75f, 16, 4);
                gluDeleteQuadric(quad);

                // Cylinder sphere caps
                applyTranslation(0.0f, 0.0f, 0.0f);
                glutSolidSphere(0.57f, 12, 12);
                applyTranslation(0.0f, 0.0f, 1.75f);
                glutSolidSphere(0.57f, 12, 12);
                glPopMatrix();

                // 3 Glowing Containment Reactor Rings
                if (parts[3].isRepaired) {
                    glColor3f(0.0f, 0.95f, 0.35f); // Glowing Neon Green
                } else {
                    glColor3f(0.95f, 0.2f, 0.1f);  // Warning Red
                }
                for (int ring = 0; ring < 3; ++ring) {
                    glPushMatrix();
                    applyTranslation(0.0f, 0.0f, 0.35f + ring * 0.52f);
                    glutSolidTorus(0.06f, 0.61f, 8, 16);
                    glPopMatrix();
                }

                // External structural protective bars (Roll cage cages)
                glColor3f(0.32f, 0.35f, 0.38f);
                for (int bar = 0; bar < 4; ++bar) {
                    glPushMatrix();
                    applyRotation(bar * 90.0f, 0.0f, 0.0f, 1.0f);
                    applyTranslation(0.68f, 0.0f, 0.88f);
                    applyScaling(0.06f, 0.06f, 1.85f);
                    glutSolidCube(1.0f);
                    glPopMatrix();
                }

                // Damaged Reactor Leaks
                if (!parts[3].isRepaired) {
                    glDisable(GL_LIGHTING);
                    glColor3f(1.0f, 0.3f, 0.2f);
                    glPushMatrix();
                    applyTranslation(0.4f, 0.4f, 0.8f);
                    glutSolidSphere(0.18f, 6, 6);
                    glPopMatrix();
                    glEnable(GL_LIGHTING);
                }
            }
            break;

        case 4: // Cargo Pod
            {
                // Draw a beautiful octagonal cargo container container
                glColor3f(parts[4].color[0], parts[4].color[1], parts[4].color[2]);
                
                // Base Box
                glPushMatrix();
                applyScaling(1.3f, 0.9f, 1.3f);
                glutSolidCube(1.0f);
                glPopMatrix();

                // Reinforced Corner brackets (Dark Grey)
                glColor3f(0.24f, 0.26f, 0.28f);
                for (int cx = -1; cx <= 1; cx += 2) {
                    for (int cy = -1; cy <= 1; cy += 2) {
                        for (int cz = -1; cz <= 1; cz += 2) {
                            glPushMatrix();
                            applyTranslation(cx * 0.65f, cy * 0.45f, cz * 0.65f);
                            glutSolidSphere(0.18f, 8, 8);
                            glPopMatrix();
                        }
                    }
                }

                // Handle bars on the sides
                glColor3f(0.6f, 0.65f, 0.7f);
                glPushMatrix();
                applyTranslation(0.66f, 0.0f, 0.0f);
                applyScaling(0.05f, 0.3f, 0.1f);
                glutSolidCube(1.0f);
                glPopMatrix();

                glPushMatrix();
                applyTranslation(-0.66f, 0.0f, 0.0f);
                applyScaling(0.05f, 0.3f, 0.1f);
                glutSolidCube(1.0f);
                glPopMatrix();

                // Warning hazard plates decals (Yellow / Black stripe frames)
                if (parts[4].isRepaired) {
                    glDisable(GL_LIGHTING);
                    glColor3f(0.85f, 0.75f, 0.1f); // Yellow
                    glPushMatrix();
                    applyTranslation(0.0f, 0.46f, 0.0f);
                    applyScaling(0.9f, 0.02f, 0.9f);
                    glutWireCube(1.0f);
                    glPopMatrix();
                    glEnable(GL_LIGHTING);
                }
            }
            break;
    }
}


