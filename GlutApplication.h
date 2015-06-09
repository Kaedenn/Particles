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

#ifndef GLUTAPPLICATION_INCLUDED
#define GLUTAPPLICATION_INCLUDED

class GlutApplication
{
private:
    /* Elements: */
    static GlutApplication* globalApplicationObject; // Pointer to the global application object (boo!)
    unsigned int displayMode; // Display mode to use when opening the application window
    bool idleFunctionEnabled; // Flag whether to enable the glut idle function
    const char* applicationName; // Pointer to the name of this application
    
    /* Private methods (glut callback wrappers): */
    static void reshapeFunction(int newWidth,int newHeight); // Registered as glut reshape function; calls reshape method
    static void displayFunction(void); // Registered as glut display function; calls display method
    static void keyboardFunction(unsigned char key,int x,int y); // Registered as glut keyboard function; calls keyboard method
    static void mouseFunction(int button,int state,int x,int y); // Registered as glut mouse function; calls mouse method
    static void motionFunction(int x,int y); // Registered as glut motion function; calls motion method
    static void idleFunction(void); // Registered as glut idle function; calls idle method
    
protected:
    /* Protected methods (to be called from derived class' constructor): */
    void setDisplayMode(unsigned int newDisplayMode); // Sets the display mode for subsequently opened application window
    void enableIdleFunction(void); // Enables the glut idle function
    
public:
    /* Constructors and destructors: */
    GlutApplication(const char* sApplicationName,int& argc,char**& argv); // Initializes glut and the application
    virtual ~GlutApplication(void); // De-initializes the application and shuts down glut
    
    /* Methods: */
    void run(int initialWidth,int initialHeight); // Creates a glut window and runs the glut application
    virtual void initOpenGLState(void); // Called once at startup right after the glut window is created
    virtual void reshape(int newWidth,int newHeight); // Called whenever the glut window size changes
    virtual void display(void); // Called when the glut window needs to be redrawn
    virtual void keyboard(unsigned char key,int x,int y); // Called when a key is pressed
    virtual void mouse(int button,int state,int x,int y); // Called whenever a mouse button changes state
    virtual void motion(int x,int y); // Called whenever the mouse moves while a button is pressed
    virtual void idle(void); // Called whenever glut has nothing better to do; disabled by default to save CPU time
};

#endif
