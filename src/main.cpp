/****************************************************************************
KANDY Application
- A demo application showing the capabilities of the KANDY platform

  -----------------------------------------------------------------------
	2/14/2012  Simon ma, Charles Essien
****************************************************************************/

#include <string.h>
#include "kinect.h"
#include <boost/thread/thread.hpp>

#define width 1024
#define height 600

int main_window;


void myGlutIdle( void )
{
  /* According to the GLUT specification, the current window is
     undefined during an idle callback.  So we need to explicitly change
     it if necessary */
  if ( glutGetWindow() != main_window )
    glutSetWindow(main_window);

  glutPostRedisplay();
}

void myGlutDisplay( void ) {
  glClearColor( .0f, .0f, .0f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glColor4f(1,1,1,1);
  glPushMatrix();


  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 1);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  kinectDisplay();

  glBegin(GL_POLYGON);
		  glTexCoord2f(0.0,1.0); glVertex3f(-1, 1, 0.0);
		  glTexCoord2f(1.0,1.0); glVertex3f(1,  1, 0.0);
		  glTexCoord2f(1.0,0.0); glVertex3f(1, -1, 0.0);
		  glTexCoord2f(0.0,0.0); glVertex3f(-1,-1, 0.0);
  glEnd();
  glDisable(GL_TEXTURE_2D);
  glPopMatrix();

  glutSwapBuffers();
}

void task1(){
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowPosition( 50, 50 );
	glutInitWindowSize( width, height );
	glViewport(0,0,width,height);
	main_window = glutCreateWindow( "KANDY Server Prototype" );
	glutDisplayFunc(myGlutDisplay);
	glutIdleFunc( myGlutIdle );

	glutMainLoop();
}

void task2(){

}


/**************************************** main() ********************/
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	InitKinect();


  using namespace boost;
//  thread thread1(task1);
//  thread thread2(task2);

  task1();
//  thread1.join();
//  thread2.join();

  return EXIT_SUCCESS;
}
