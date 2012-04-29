/*
 * kinect.h
 *
 *  Created on: Feb 15, 2012
 *      Author: simon
 */

#ifndef KINECT_H_
#define KINECT_H_

#include <XnCppWrapper.h>
#include <GL/gl.h>
#include <GL/glut.h>

int start();

void motorAngle(int angle);

void kinectDisplay (void);

static void displayError(const char *on_what);

void sendMessage(char * message);

void initUDP(char * server);

void controlUDP(char * server);

void closeUDP();


#endif /* KINECT_H_ */
