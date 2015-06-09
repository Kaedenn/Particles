/***********************************************************************
GlutApplication - Simple base class to create glut applications without
having to deal with scores of global variables.
Copyright (c) 2005 Oliver Kreylos

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

#include "GlutApplication.h"

#include <GL/glut.h>

/****************************************
Static elements of class GlutApplication:
****************************************/

GlutApplication* GlutApplication::globalApplicationObject=0;

/********************************
Methods of class GlutApplication:
********************************/

void GlutApplication::reshapeFunction(int newWidth,int newHeight)
{
    /* Call reshape method of the global glut application object: */
    globalApplicationObject->reshape(newWidth,newHeight);
}

void GlutApplication::displayFunction(void)
{
    /* Call display method of the global glut application object: */
    globalApplicationObject->display();
}

void GlutApplication::keyboardFunction(unsigned char key,int x,int y)
{
    /* Call keyboard method of the global glut application object: */
    globalApplicationObject->keyboard(key,x,y);
}

void GlutApplication::mouseFunction(int button,int state,int x,int y)
{
    /* Call mouse method of the global glut application object: */
    globalApplicationObject->mouse(button,state,x,y);
}

void GlutApplication::motionFunction(int x,int y)
{
    /* Call motion method of the global glut application object: */
    globalApplicationObject->motion(x,y);
}

void GlutApplication::idleFunction(void)
{
    /* Call idle method of the global glut application object: */
    globalApplicationObject->idle();
}

void GlutApplication::setDisplayMode(unsigned int newDisplayMode)
{
    displayMode=newDisplayMode;
}

void GlutApplication::enableIdleFunction(void)
{
    idleFunctionEnabled=true;
}

GlutApplication::GlutApplication(const char* sApplicationName,int& argc,char**& argv)
    :displayMode(GLUT_RGB),
     idleFunctionEnabled(false),
     applicationName(sApplicationName)
{
    /* Initialize glut: */
    glutInit(&argc,argv);
    
    /* Set the global application object pointer: */
    globalApplicationObject=this;
}

GlutApplication::~GlutApplication(void)
{
    /* Reset the global application object pointer: */
    globalApplicationObject=0;
}

void GlutApplication::run(int initialWidth,int initialHeight)
{
    /* Create a glut window: */
    glutInitDisplayMode(displayMode);
    glutInitWindowSize(initialWidth,initialHeight);
    glutCreateWindow(applicationName);
    
    /* Call the OpenGL initialization method: */
    initOpenGLState();
    
    /* Register the glut callbacks: */
    glutReshapeFunc(reshapeFunction);
    glutDisplayFunc(displayFunction);
    glutKeyboardFunc(keyboardFunction);
    glutMouseFunc(mouseFunction);
    glutMotionFunc(motionFunction);
    if(idleFunctionEnabled)
        glutIdleFunc(idleFunction);
    
    /* Execute the glut main loop: */
    glutMainLoop();
}

void GlutApplication::initOpenGLState(void)
{
}

void GlutApplication::reshape(int newWidth,int newHeight)
{
    (void)newWidth;
    (void)newHeight;
}

void GlutApplication::display(void)
{
}

void GlutApplication::keyboard(unsigned char key,int x,int y)
{
    globalApplicationObject->keyboard(key,x,y);
}

void GlutApplication::mouse(int button,int state,int x,int y)
{
    (void)button;
    (void)state;
    (void)x;
    (void)y;
}

void GlutApplication::motion(int x,int y)
{
    (void)x;
    (void)y;
}

void GlutApplication::idle(void)
{
}
