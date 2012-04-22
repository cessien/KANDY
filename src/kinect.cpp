/*******************************************************************************
*                                                                              *
*   PrimeSense NITE 1.3 - Players Sample                                       *
*   Copyright (C) 2010 PrimeSense Ltd.                                         *
*                                                                              *
*******************************************************************************/

#include <XnOpenNI.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>
#include "XnVNite.h"
#include <GL/glut.h>
//#include "SceneDrawer.h"
#include "kinect.h"


xn::Context g_Context;
xn::ScriptNode g_ScriptNode;
xn::DepthGenerator g_DepthGenerator;
//xn::UserGenerator g_UserGenerator;

XnVSessionManager * g_pSessionManager;
//XnVBroadcaster* broadcaster;

unsigned char *image;
xn::SceneMetaData sceneMD;
xn::DepthMetaData depthMD;
xn::ImageMetaData imageMD;
const XnDepthPixel* pDepth;
#define MAX_DEPTH 10000
float g_pDepthHist[MAX_DEPTH];
unsigned int nValue, nIndex, nX, nY, nNumberOfPoints;
XnUInt16 g_nXRes, g_nYRes;
unsigned char* pDestImage;
unsigned char* pDepthTexBuf;

// this function is called each frame
void kinectDisplay (void)
{
	// Setup the OpenGL viewpoint
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();


	g_DepthGenerator.GetMetaData(depthMD);
	glOrtho(0, depthMD.XRes(), depthMD.YRes(), 0, -1.0, 1.0);


	// Read next available data
	g_Context.WaitAnyUpdateAll();
	g_pSessionManager->Update(&g_Context);

	//initialize loop variables
	nValue = 0;
	nIndex = 0;
	nX = 0; nY = 0;
	nNumberOfPoints = 0;
	g_nXRes = depthMD.XRes();
	g_nYRes = depthMD.YRes();

	// Process the data
	pDepth = depthMD.Data();

	memset(g_pDepthHist, 0, MAX_DEPTH*sizeof(float));
	for (nY=0; nY<g_nYRes; nY++)
	{
		for (nX=0; nX<g_nXRes; nX++)
		{
			nValue = *pDepth;

			if (nValue != 0)
			{
				g_pDepthHist[nValue]++;
				nNumberOfPoints++;
			}

			pDepth++;
		}
	}

	for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
	{
		g_pDepthHist[nIndex] += g_pDepthHist[nIndex-1];
	}
	if (nNumberOfPoints)
	{
		for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
		{
			g_pDepthHist[nIndex] = (unsigned int)(256 * (1.0f - (g_pDepthHist[nIndex] / nNumberOfPoints)));
		}
	}
	pDepth = (short unsigned int*)depthMD.Data();
	for (nY=0; nY<g_nYRes; nY++)
	{
		for (nX=0; nX < g_nXRes; nX++)
		{

			nValue = *pDepth;
//			//printf("Depth: %i \n",nValue);

			pDestImage[0] = nValue * .15;
			pDestImage[1] = nValue * .70;
			pDestImage[2] = nValue * .15;
			pDestImage[3] = 255;


			pDepth++;
			pDestImage+=4;
		}

		pDestImage += (640 - g_nXRes) *4;
	}

	// Display the OpenGL texture map
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, pDepthTexBuf);

	//	DrawDepthMap(depthMD, sceneMD, focus, imageMD);

}

// callback for session start
void XN_CALLBACK_TYPE SessionStarting(const XnPoint3D& ptPosition, void* UserCxt)
{
	printf("Session start: (%f,%f,%f)\n", ptPosition.X, ptPosition.Y, ptPosition.Z);
//	g_SessionState = IN_SESSION;
}
// Callback for session end
void XN_CALLBACK_TYPE SessionEnding(void* UserCxt)
{
	printf("Session end\n");
//	g_SessionState = NOT_IN_SESSION;
}

// Callback for when the focus is in progress
void XN_CALLBACK_TYPE FocusProgress(const XnChar* strFocus, const XnPoint3D& ptPosition, XnFloat fProgress, void* UserCxt)
{
//	g_UserGenerator.
	printf("Focus progress: %s @(%f,%f,%f): %f\n", strFocus, ptPosition.X, ptPosition.Y, ptPosition.Z, fProgress);
}


#include <XnUSB.h>
#define VID_MICROSOFT 0x45e
#define PID_NUI_MOTOR 0x02b0
XN_USB_DEV_HANDLE dev;

void InitKinect(){
	pDestImage = (unsigned char *)malloc(sizeof(unsigned char)*640*480*4);

	XnStatus rc = XN_STATUS_OK;
	xn::EnumerationErrors errors;

	rc = g_Context.InitFromXmlFile(SAMPLE_XML_PATH, g_ScriptNode, &errors);
	rc = g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
//	rc = g_Context.FindExistingNode(XN_NODE_TYPE_USER, g_UserGenerator);

	g_pSessionManager = new XnVSessionManager();
	rc = g_pSessionManager->Initialize(&g_Context, "Click,Wave", "RaiseHand");
	g_pSessionManager->RegisterSession(NULL, SessionStarting, SessionEnding, FocusProgress);

	rc = g_Context.StartGeneratingAll();

	motorAngle(10);

}

void motorAngle(int angle){
//	printf("%i\n",angle);
	xnUSBInit();
	xnUSBOpenDevice(VID_MICROSOFT, PID_NUI_MOTOR, NULL, NULL, &dev);
	uint8_t empty[0x1];
	angle = angle * 2;
	xnUSBSendControl(dev,
				  	  XN_USB_CONTROL_TYPE_VENDOR,
					  0x31,
					  (XnUInt16)angle,
					  0x0,
					  empty,
					  0x0, 0);
	xnUSBCloseDevice(dev);
}




