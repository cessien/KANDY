/*******************************************************************************
*                                                                              *
*   PrimeSense NITE 1.3 - Players Sample                                       *
*   Copyright (C) 2010 PrimeSense Ltd.                                         *
*                                                                              *
*******************************************************************************/

#include <XnOpenNI.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>
#include <GL/glut.h>
#include <cmath>
#include "kinect.h"
//*********************************
//Networking Includes
  #include <stdio.h>
  #include <unistd.h>
  #include <stdlib.h>
  #include <errno.h>
  #include <string.h>
  #include <time.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
//********************************

xn::Context g_Context;
xn::ScriptNode g_ScriptNode;
xn::DepthGenerator g_DepthGenerator;

#define GL_WIN_SIZE_X 720
#define GL_WIN_SIZE_Y 480

xn::DepthMetaData depthMD;
#define MAX_DEPTH 10000
float g_pDepthHist[MAX_DEPTH];

unsigned char* pDepthTexBuf;
static int texWidth, texHeight;
unsigned int nValue, nIndex, nX, nY, nNumberOfPoints;
XnUInt16 g_nXRes, g_nYRes;
unsigned char* pDestImage;
const XnDepthPixel* pDepth;
const XnRGB24Pixel* pixel;
const XnLabel* pLabels;
XnLabel label;
XnUserID* aUsers;
XnUInt16 nUsers;
XnPoint3D com;

volatile bool basic_data = false;

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
	texWidth = 640;
	texHeight = 480;

	nValue = 0;
	nIndex = 0;
	nX = 0; nY = 0;
	nNumberOfPoints = 0;
	g_nXRes = depthMD.XRes();
	g_nYRes = depthMD.YRes();

	pDestImage = pDepthTexBuf;

	pDepth = depthMD.Data();

	// Calculate the accumulative histogram
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
			pDestImage[0] = g_pDepthHist[nValue];//pixel->nRed;
			pDestImage[1] = g_pDepthHist[nValue];//pixel->nGreen;
			pDestImage[2] = g_pDepthHist[nValue];//pixel->nBlue;
			pDestImage[3] = 255;

			if (g_pDepthHist[nValue] > 155) {
				pDestImage[0] = 255;
				pDestImage[1] = 0;
				pDestImage[2] = 0;
				basic_data = true;
//				sendMessage("Vibrate");
			}


			pixel++;
			pDepth++;
			pDestImage+=4;
		}

		pDestImage += (texWidth - g_nXRes) *4;
	}

	// Display the OpenGL texture map
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pDepthTexBuf);
	//DrawDepthMap(depthMD);

	glPopMatrix();
}


#define SAMPLE_XML_PATH "Sample-User.xml"

#define CHECK_RC(rc, what)											\
	if (rc != XN_STATUS_OK)											\
	{																\
		printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
		return rc;													\
	}

#define CHECK_ERRORS(rc, errors, what)		\
	if (rc == XN_STATUS_NO_NODE_PRESENT)	\
{										\
	XnChar strError[1024];				\
	errors.ToString(strError, 1024);	\
	printf("%s\n", strError);			\
	return (rc);						\
}

extern unsigned char* pDepthTexBuf;
extern XnUserID* aUsers;

#include <XnUSB.h>
#define VID_MICROSOFT 0x45e
#define PID_NUI_MOTOR 0x02b0
XN_USB_DEV_HANDLE dev;

int start(){
	pDepthTexBuf = (unsigned char *)malloc(sizeof(char)*(640*480*4));
	aUsers  = (XnUserID *)malloc(sizeof(XnUserID)*15);

	XnStatus rc = XN_STATUS_OK;
	xn::EnumerationErrors errors;

	rc = g_Context.InitFromXmlFile(SAMPLE_XML_PATH, g_ScriptNode, &errors);
	CHECK_ERRORS(rc, errors, "InitFromXmlFile");
	CHECK_RC(rc, "InitFromXml");

	rc = g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
	CHECK_RC(rc, "Find depth generator");

	rc = g_Context.StartGeneratingAll();
	CHECK_RC(rc, "StartGenerating");

	motorAngle(10);

	return 0;
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

//UDP Globals
int z;
struct sockaddr_in adr_inet; // AF_INET
struct sockaddr_in adr_clnt; // AF_INET
unsigned int len_inet;                // length
int s;                       // Socket
char dgram[512];             // Recv buffer

/*
 * This function reports the error and
 * exits back to the shell:
 */
static void displayError(const char *on_what) {
	fputs(strerror(errno),stderr);
	fputs(": ",stderr);
	fputs(on_what,stderr);
	fputc('\n',stderr);
	exit(1);
}

void sendMessage(char *message) {
	z = sendto(s, message, 7, 0, (struct sockaddr *)&adr_clnt, len_inet);     // Client address length
	if ( z < 0 ) {
		displayError("sendto(2)");
	}
//	sleep(100);
	return;
}

void initUDP(char * server){
	printf("Waiting for client...\n");
	memset(&adr_inet,0,sizeof adr_inet);
	adr_inet.sin_family = AF_INET;
	adr_inet.sin_port = htons(10001);
	adr_inet.sin_addr.s_addr =
			inet_addr(server);

	/*
	 * Create a UDP socket to use:
	 */
	s = socket(AF_INET,SOCK_DGRAM,0);
	if ( s == -1 ) {
		displayError("socket()");
	}

	if ( adr_inet.sin_addr.s_addr == INADDR_NONE ) {
		displayError("bad address.");
	}
	len_inet = sizeof adr_inet;

	//Bind the address to a datagram socket (UDP)
	z = bind(s, (struct sockaddr *)&adr_inet, len_inet);
	if ( z == -1 ) {
		displayError("bind()");
	}


	z = recvfrom(s,	dgram, sizeof dgram, 0, (struct sockaddr *)&adr_clnt, &len_inet);
	if ( z < 0 ) {
		displayError("recvfrom(2)");
	}

	printf("Result from %s port %u :\n\t'%s'\n",
	        inet_ntoa(adr_clnt.sin_addr),
	                 (unsigned)ntohs(adr_clnt.sin_port),
	                 dgram);
	return;
}

void controlUDP(char * server){
	printf("Waiting for client...\n");
	memset(&adr_inet,0,sizeof adr_inet);
	adr_inet.sin_family = AF_INET;
	adr_inet.sin_port = htons(10001);
	adr_inet.sin_addr.s_addr =
			inet_addr(server);


	z = recvfrom(s,	dgram, sizeof dgram, 0, (struct sockaddr *)&adr_clnt, &len_inet);
	if ( z < 0 ) {
		displayError("recvfrom(2)");
	}

	printf("Result from %s port %u :\n\t'%s'\n",
	        inet_ntoa(adr_clnt.sin_addr),
	                 (unsigned)ntohs(adr_clnt.sin_port),
	                 dgram);
	return;
}

void closeUDP(){
	close(s);
	return;
}

