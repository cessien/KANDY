/****************************************************************************
KANDY Server Application
- A demo application showing the capabilities of the Praser platform
****************************************************************************/

#include <GL/glui.h>
#include "kinect.h"
#include <boost/thread/thread.hpp>

int main_window;
int width = 1024;
int height = 600;

void myGlutIdle( void )
{
  if ( glutGetWindow() != main_window )
    glutSetWindow(main_window);

  glutPostRedisplay();
}


void Reshape( int x, int y )
{
  glutPostRedisplay();
}


void myGlutDisplay( void ) {
	glClearColor( .0f, .0f, .0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glColor4f(1,1,1,1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 1);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	kinectDisplay();
	glBegin(GL_POLYGON);
		  glTexCoord2f(1.0,0.0); glVertex3f(-1, 1, 0.0);
		  glTexCoord2f(0.0,0.0); glVertex3f(1, 1, 0.0);
		  glTexCoord2f(0.0,1.0); glVertex3f(1, -1, 0.0);
		  glTexCoord2f(1.0,1.0); glVertex3f(-1, -1, 0.0);
	glEnd();
	glDisable (GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	//******* Debug Info *******************//
	glColor4f(0,1,0,1);
	glPointSize(6);
	glBegin(GL_LINES);
		glVertex3f(-1,0,0); glVertex3f(1,0,0);
		glVertex3f(-.5,1,0); glVertex3f(-.5,-1,0);
		glVertex3f(.5,1,0); glVertex3f(.5,-1,0);
	glEnd();
	//**************************************

	glutSwapBuffers();
}



void KinectThread(){
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowPosition( 50, 50 );
	glutInitWindowSize( width, height );
	glViewport(0,0,width,height);
	main_window = glutCreateWindow( "Kinect KANDY Prototype" );
	glutDisplayFunc(myGlutDisplay);

	glutReshapeFunc( Reshape );

	glutInitWindowPosition( 1500, 50 );
	glutInitWindowSize( 1024, 768 );

	glutIdleFunc( myGlutIdle );

	glutMainLoop();
}

extern volatile bool basic_data;

void UDPMessageThread(){
	initUDP("192.168.1.156");

	for(;;){
		if(basic_data){
//			printf("Sending message\n");
//			sleep(100);
			sendMessage("Vibrate");
			basic_data = false;
		}
	}
}

void UDPAcceptThread(){
	for(;;){
		controlUDP("192.168.1.156");
	}
}


/**************************************** main() ********************/
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	start();


	using namespace boost;
	  thread thread1(KinectThread);
	  thread thread2(UDPMessageThread);
	  thread thread3(UDPAcceptThread);

//	task2();
	  thread1.join();
	  thread2.join();

	return EXIT_SUCCESS;
}


