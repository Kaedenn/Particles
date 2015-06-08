/***********************************************************************
CollisionBoxTest - Test program for the collision box hard sphere
simulation data structure.
Copyright (c) 2005-2012 Oliver Kreylos

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
***********************************************************************/

#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include <strings.h>
#include <Misc/Timer.h>
#include <Math/Math.h>
#include <Math/Constants.h>
#include <Math/Random.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/GLGeometryWrappers.h>

#include "GlutApplication.h"
#include "CollisionBox.h"

class CollisionBoxTest : public GlutApplication
{
private:
    /* Embedded classes: */
    typedef CollisionBox<double, 2> MyCollisionBox;
    
    typedef MyCollisionBox::Vector Vector;
    typedef MyCollisionBox::Scalar Scalar;
    typedef MyCollisionBox::Point Point;
    typedef MyCollisionBox::Box Box;

private:
    /* Elements: */
    MyCollisionBox* collisionBox; // The collision box data structure
    int winSize[2]; // Size of display window in pixels
    Scalar winMin[2], winMax[2]; // Bounds of display window in collision box coordinates
    Scalar sphereRadius;
    Point spherePosition; // New position for the collision sphere
    Misc::Timer applicationTimer; // Free-running high-resolution timer
    double lastApplicationTime; // Application time at last invocation of frame method
    GLuint particleListId; // Display list ID to render 2D particles
    float rgb[3];
    bool showFps;
    
public:
    /* Constructors and destructors: */
    CollisionBoxTest(int& argc, char**& argv); // Initializes the glut toolkit and the application
    ~CollisionBoxTest(void); // Shuts down the glut toolkit
    
    /* Methods: */
    virtual void initOpenGLState(void);
    virtual void reshape(int newWidth, int newHeight);
    virtual void display(void);
    virtual void keyboard(unsigned char, int x, int y);
    virtual void motion(int x, int y);
    virtual void idle(void);
};

/*********************************
Methods of class CollisionBoxTest:
*********************************/

CollisionBoxTest::CollisionBoxTest(int& argc, char**& argv)
 : GlutApplication("Collision Detection Test", argc, argv), 
   collisionBox(0), 
   lastApplicationTime(applicationTimer.peekTime()), 
   particleListId(0),
   showFps(false)
{
    /* Set glut display mode: */
    setDisplayMode(GLUT_RGB|GLUT_DOUBLE);
    enableIdleFunction();

    Extra::Debug::Enable(Extra::Debug::DEBUG_VERBOSE);

    /* Parse arguments */
    rgb[0] = rgb[1] = rgb[2] = 0.5f;
    int boxSize = 256;
    sphereRadius = 25.0;
    int numParticles = 1000;
    Vector latentForce;
    Scalar friction = 0;
    Scalar speedRange = 4.0;
    bool stopped = false;
    for (int argi = 1; argi < argc; ++argi) {
        if (argv[argi][0] == '-') {
            /* Parameters with values */
            if (argi+1 < argc) {
                if (!strcasecmp(argv[argi], "--size") || !strcasecmp(argv[argi], "-s")) {
                    boxSize = atoi(argv[argi+1]);
                } else if (!strcasecmp(argv[argi], "--radius")) {
                    sphereRadius = atof(argv[argi+1]);
                } else if (!strcasecmp(argv[argi], "-r")) {
                    rgb[0] = atof(argv[argi+1]);
                } else if (!strcasecmp(argv[argi], "-g")) {
                    rgb[1] = atof(argv[argi+1]);
                } else if (!strcasecmp(argv[argi], "-b")) {
                    rgb[2] = atof(argv[argi+1]);
                } else if (!strcasecmp(argv[argi], "--gravity")) {
                    latentForce[1] = atof(argv[argi+1]);
                } else if (!strcasecmp(argv[argi], "--friction")) {
                    friction = atof(argv[argi+1]);
                } else if (!strcasecmp(argv[argi], "--speedrange")) {
                    speedRange = atof(argv[argi+1]);
                }
                ++argi;
            }
            /* Parameters without values */
            if (!strcasecmp(argv[argi], "--stopped")) {
                speedRange = 0.0;
            } else if (!strcasecmp(argv[argi], "--fps")) {
                showFps = true;
            }
        } else {
            /* Unnamed parameter */
            numParticles = atoi(argv[argi]);
        }
    }
    
    /* Create a collision box: */
    Point min(0), max(boxSize);
    Scalar particleRadius = 1.0; /* Values > 1.0 cause freezing */
    collisionBox = new MyCollisionBox(Box(min, max), particleRadius, sphereRadius);
    collisionBox->setLatentForce(latentForce);
    collisionBox->setFriction(friction);
    spherePosition = collisionBox->getSphere();
    
    /* Create a few particles: */
    const Box& boundaries = collisionBox->getBoundaries();
    int particleIndex;
    for (particleIndex = 0; particleIndex < numParticles; ++particleIndex) {
        const int maxNumTries = 200;
        int tries;
        for (tries = 0; tries < maxNumTries; ++tries) {
            Point p;
            Vector v;
            for (int j = 0; j < MyCollisionBox::dimension; ++j) {
                p[j] = Math::randUniformCC(boundaries.min[j]+Scalar(1), 
                                           boundaries.max[j]-boundaries.min[j]-Scalar(2));
                if (!stopped) {
                    v[j] = Scalar(Math::randUniformCC(-speedRange, speedRange));
                }
            }
            
            /* Try adding the new particle: */
            if (collisionBox->addParticle(p, v)) {
                break;
            }
        }
        
        if (tries == maxNumTries) { // Could not add particles after N tries; assume box is full
            break;
        }
    }
    
    if (particleIndex < numParticles) {
        /* Print a warning message: */
        std::cout << "Could only add " << particleIndex
                  << " particles to collision box" << std::endl;
    }
}

CollisionBoxTest::~CollisionBoxTest(void)
{
    delete collisionBox;
}

void CollisionBoxTest::initOpenGLState(void)
{
    glDisable(GL_LIGHTING);
    glPointSize(1.0f);
    glLineWidth(1.0f);
    
    /* Draw a particle: */
    particleListId = glGenLists(1);
    glNewList(particleListId, GL_COMPILE);
    glBegin(GL_POLYGON);
    const int nsteps = 8; /* Octagon is close enough to a circle this small */
    for (int i = 0; i < nsteps; ++i) {
        float angle = 2.0f*Math::Constants<float>::pi*float(i)/float(nsteps);
        glVertex2f(Math::cos(angle), Math::sin(angle));
    }
    glEnd();
    glEndList();
}

void CollisionBoxTest::reshape(int newWidth, int newHeight)
{
    winSize[0] = newWidth;
    winSize[1] = newHeight;
    glViewport(0, 0, newWidth, newHeight);
    
    const Box& bounds = collisionBox->getBoundaries();
    double windowAspect = double(newWidth)/double(newHeight);
    double boxWidth = bounds.getSize(0);
    double boxHeight = bounds.getSize(1);
    double boxCenterX = double(bounds.min[0]+bounds.max[0])*0.5;
    double boxCenterY = double(bounds.min[1]+bounds.max[1])*0.5;
    double boxAspect = boxWidth/boxHeight;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double ht, wt;
    if (windowAspect >= boxAspect) {
        ht = boxHeight*0.6;
        wt = ht*windowAspect/boxAspect;
    } else {
        wt = boxWidth*0.6;
        ht = wt*boxAspect/windowAspect;
    }
    winMin[0] = Scalar(boxCenterX-wt);
    winMax[0] = Scalar(boxCenterX+wt);
    winMin[1] = Scalar(boxCenterY-ht);
    winMax[1] = Scalar(boxCenterY+ht);
    glOrtho(boxCenterX-wt, boxCenterX+wt, boxCenterY-ht, boxCenterY+ht, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void CollisionBoxTest::display(void)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    /* Render the collision box: */
    const Box& boundaries = collisionBox->getBoundaries();
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex(boundaries.getVertex(0));
    glVertex(boundaries.getVertex(1));
    glVertex(boundaries.getVertex(3));
    glVertex(boundaries.getVertex(2));
    glEnd();
    
    if (MyCollisionBox::dimension == 2) {
        /* Render the collision sphere: */
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glColor3f(rgb[0], rgb[1], rgb[2]);
        /* glColor3f(0.5f, 0.5f, 0.5f); */ 
        glBegin(GL_POLYGON);
        Point sp = collisionBox->getSphere();
        for (int i = 0; i < 64; ++i) {
            float angle = 2.0f*Math::Constants<float>::pi*float(i)/float(64);
            glVertex2f(sp[0]+Math::cos(angle)*sphereRadius, sp[1]+Math::sin(angle)*sphereRadius);
        }
        glEnd();
        
        /* Render all particles: */
        glPushMatrix();
        Point currentPos = Point::origin;
        auto fn = [this, &currentPos](const MyCollisionBox::Particle& p) {
            glTranslate(p.getPosition()-currentPos);
            glCallList(particleListId);
            currentPos = p.getPosition();
        };
        collisionBox->getParticles().forEach(fn);

        glPopMatrix();
        glDisable(GL_BLEND);
    }
    else
    {
        glBegin(GL_POINTS);
        glColor3f(1.0f, 1.0f, 1.0f);
        auto fn = [](const MyCollisionBox::Particle& p) {
            glVertex(p.getPosition());
        };
        collisionBox->getParticles().forEach(fn);
        glEnd();
    }
    
    glutSwapBuffers();
}

void CollisionBoxTest::keyboard(unsigned char key, int x, int y)
{
    if (key == 0x1b) /* KEY_ESCAPE, not provided by glut for some reason */
    {
       delete collisionBox;
       std::exit(0);
    }
    std::cout << "Key: " << std::hex << (int)key << " at " << x << ", " << y << std::endl;
}

void CollisionBoxTest::motion(int x, int y)
{
    /* Calculate the new mouse position in box coordinates: */
    double mouseX = winMin[0]+double(x)*(winMax[0]-winMin[0])/double(winSize[0]);
    double mouseY = winMin[1]+double(winSize[1]-1-y)*(winMax[1]-winMin[1])/double(winSize[1]);
    spherePosition[0] = Scalar(mouseX);
    spherePosition[1] = Scalar(mouseY);
}

void CollisionBoxTest::idle(void)
{
    static const int frameCountMax = 30;
    static int frameCounter = 0;
    static double frameTimer = applicationTimer.peekTime();

    /* Simulate by the current frame time: */
    double newApplicationTime = applicationTimer.peekTime();
    double timeStep = newApplicationTime-lastApplicationTime;
    if (timeStep>0.1) {
        /* Go into "catch-up" mode: */
        timeStep = 0.1;
        collisionBox->setAttenuation(Scalar(0.5));
    } else {
        collisionBox->setAttenuation(Scalar(0.9));
    }
    collisionBox->moveSphere(spherePosition, 0.1);
    collisionBox->simulate(timeStep);
    lastApplicationTime = newApplicationTime;
    glutPostRedisplay();

    if (showFps) {
        if (++frameCounter >= frameCountMax) {
            double fps = frameCounter/(newApplicationTime - frameTimer);
            std::cout << fps << std::endl;
            frameCounter = 0;
            frameTimer = newApplicationTime;
        }
    }
}

int main(int argc, char* argv[])
{
    try
    {
        /* Create an application object: */
        CollisionBoxTest app(argc, argv);
        
        /* Run the application with an initial window size of 800x600: */
        app.run(800, 600);
        
        /* Exit: */
        return 0;
    }
    catch(std::runtime_error err)
    {
        /* Print error message: */
        std::cerr<<"Caught exception "<<err.what()<<std::endl;
        
        /* Signal error to OS: */
        return 1;
    }
}
