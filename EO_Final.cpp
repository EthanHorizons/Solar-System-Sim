#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctype.h>
#include <time.h>


#ifndef F_PI
#define F_PI		((float)(M_PI))
#define F_2_PI		((float)(2.f*F_PI))
#define F_PI_2		((float)(F_PI/2.f))
#endif


#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif


#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "glut.h"


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// title of these windows:

const char *WINDOWTITLE = "OpenGL / GLUT Sample -- Joe Graphics";
const char *GLUITITLE   = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE  = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 600;

// size of the 3d box to be drawn:

const float BOXSIZE = 2.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT   = 4;
const int MIDDLE = 2;
const int RIGHT  = 1;

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

enum Textures
{
	None,
	Venus,
	Earth,
	Moon,
	Jupiter,
	Saturn,
	Neptune,
	Uranus,
	Pluto
};
// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH   = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
};

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0f, .0f, .0f, 1.f };
const GLenum  FOGMODE     = GL_LINEAR;
const GLfloat FOGDENSITY  = 0.30f;
const GLfloat FOGSTART    = 1.5f;
const GLfloat FOGEND      = 4.f;

// for lighting:

const float	WHITE[ ] = { 1.,1.,1.,1. };

// for animation:

const int MS_PER_CYCLE = 1000000;		// 10000 milliseconds = 10 seconds

const int MAX_TRAIL_POINTS = 50;
// Array to store positions
float trailPositions[MAX_TRAIL_POINTS][3];
int trailIndex = 0;
int trailCount = 0;


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
#define DEMO_Z_FIGHTING
#define DEMO_DEPTH_BUFFER


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
GLuint	BoxList;				// object display list
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
int		NowColor;				// index into Colors[ ]
int		NowProjection;		// ORTHO or PERSP

int		NowTexture;

GLuint RingDL;
GLuint	SphereDL, VenusDL, EarthDL, MoonDL, JupiterDL, SaturnDL, UranusDL, NeptuneDL, PlutoDL, MarsDL, SunDL, SkyDL, MercuryDL;
GLuint	VenusTex, EarthTex, MoonTex, JupiterTex, SaturnTex, UranusTex, NeptuneTex, PlutoTex, MarsTex, SunTex, SkyTex, MercuryTex;

float		VenusRad, EarthRad, MoonRad, JupiterRad, SaturnRad, UranusRad, NeptuneRad, PlutoRad, MarsRad, SunRad, MercuryRad, Radius, Diameter;
float		VenusDi, EarthDi, MoonDi, JupiterDi, SaturnDi, UranusDi, NeptuneDi, PlutoDi, MarsDi, SunDi, MercuryDi;
float		VenusOrb, EarthOrb, MoonOrb, JupiterOrb, SaturnOrb, UranusOrb, NeptuneOrb, PlutoOrb, MarsOrb, Orbit;
float		Timescale = 200;
bool	LightingOn;

float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
float	Time;					// used for animation, this has a value between 0. and 1.
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees


// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );

void	DoTexture(int);

void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void			Axes( float );
void			HsvRgb( float[3], float [3] );
void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);
float			Unit(float [3]);


// utility to create an array from 3 separate values:

// Function to compute orbital period based on Kepler's Third Law
float getOrbitalPeriod(float radius) {
	// Kepler's Third Law: T = k * R^(3/2), where k is a constant
	const float k = 1.0f;  // Proportionality constant (adjustable for scaling)
	return k * pow(radius, 1.5f);  // T = k * R^(3/2)
}
float OrbitTime(float radius) {
	// Calculate the orbital period from Kepler's Third Law
	float orbitalPeriod = getOrbitalPeriod(radius);
	// Normalize the elapsed time to complete one full orbit
	float phase = (Time / orbitalPeriod) * Timescale * 2 * M_PI;  // From 0 to 2?
	return phase;

}
float *
Array3( float a, float b, float c )
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from a multiplier and an array:

float *
MulArray3( float factor, float array0[ ] )
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}


float *
MulArray3(float factor, float a, float b, float c )
{
	static float array[4];

	float* abc = Array3(a, b, c);
	array[0] = factor * abc[0];
	array[1] = factor * abc[1];
	array[2] = factor * abc[2];
	array[3] = 1.;
	return array;
}


float
Ranf( float low, float high )
{
        float r = (float) rand();               // 0 - RAND_MAX
        float t = r  /  (float) RAND_MAX;       // 0. - 1.

        return   low  +  t * ( high - low );
}


// call this if you want to force your program to use
// a different random number sequence every time you run it:
void
TimeOfDaySeed( )
{
	struct tm y2k;
	y2k.tm_hour = 0;    y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 2000; y2k.tm_mon = 0; y2k.tm_mday = 1;

	time_t  now;
	time( &now );
	double seconds = difftime( now, mktime(&y2k) );
	unsigned int seed = (unsigned int)( 1000.*seconds );    // milliseconds
	srand( seed );
}

// these are here for when you need them -- just uncomment the ones you need:

#include "setmaterial.cpp"
#include "setlight.cpp"
#include "osusphere.cpp"
//#include "osucone.cpp"
//#include "osutorus.cpp"
#include "bmptotexture.cpp"
//#include "loadobjfile.cpp"
//#include "keytime.cpp"
//#include "glslprogram.cpp"
//#include "vertexbufferobject.cpp"


// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since glutInit might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitGraphics( );

	// create the display lists that **will not change**:

	InitLists( );

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );

	// setup all the user interface stuff:

	InitMenus( );

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables for Display( ) to find:

	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;							// makes the value of ms between 0 and MS_PER_CYCLE-1
	Time = (float)ms / (float)MS_PER_CYCLE;		// makes the value of Time between 0. and slightly less than 1.

	// for example, if you wanted to spin an object in Display( ), you might call: glRotatef( 360.f*Time,   0., 1., 0. );

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting Display.\n");

	// set which window we want to do the graphics into:
	glutSetWindow( MainWindow );

	// erase the background:
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif


	// specify shading to be flat:

	glShadeModel( GL_FLAT );

	// set the viewport to be a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( NowProjection == ORTHO )
		glOrtho( -2.f, 2.f,     -2.f, 2.f,     0.1f, 1000.f );
	else
		gluPerspective( 70.f, 1.f,	0.1f, 1000.f );

	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// set the eye position, look-at position, and up-vector:

	//gluLookAt( 7.f, 0.f, 3.f,     0.f, 0.f, 0.f,     0.f, 1.f, 0.f );
	if (NowTexture != None) {
		if (NowTexture == Venus) {
			Radius = VenusRad;
		}
		if (NowTexture == Earth) Radius = EarthRad;
		if (NowTexture == Moon) Radius = MarsRad;
		if (NowTexture == Jupiter) Radius = JupiterRad;
		if (NowTexture == Saturn) Radius = SaturnRad;
		if (NowTexture == Neptune) Radius = NeptuneRad;
		if (NowTexture == Uranus) Radius = UranusRad;
		if (NowTexture == Pluto) Radius = MercuryRad;

		gluLookAt((Radius - Radius/3) * sin(OrbitTime(Radius)), Radius/2, (Radius - Radius/3) * cos(OrbitTime(Radius)), (Radius)*sin(OrbitTime(Radius)), 0.f, (Radius)*cos(OrbitTime(Radius)), 0.f, 1.f, 0.f);

	}
	else {
		gluLookAt(7.f, 0.f, 3.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
		glRotatef((GLfloat)Yrot, 0.f, 1.f, 0.f);
		glRotatef((GLfloat)Xrot, 1.f, 0.f, 0.f);
		if (Scale < MINSCALE)
			Scale = MINSCALE;
		glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);
	}
	
	// rotate the scene:


	// uniformly scale the scene:



	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// possibly draw the axes:

	if( AxesOn == 0 )
	{
		glColor3fv( &Colors[NowColor][0] );
		glCallList( AxesList );
	}

	// since we are using glScalef( ), be sure the normals get unitized:

	glEnable( GL_NORMALIZE );

	MercuryRad = .39;
	VenusRad = .72;
	EarthRad = 1;
	MoonRad = 2;
	MarsRad = 1.5;
	JupiterRad = 5.2;
	SaturnRad = 9.5;
	UranusRad = 30;
	NeptuneRad = 24;

	MercuryDi = .05;
	VenusDi = .1;
	EarthDi = .1;
	MoonDi = .5;
	MarsDi = .05;
	JupiterDi = 1.1;
	SaturnDi = .95;
	UranusDi = .4;
	NeptuneDi = .4;


	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, SkyTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glScalef(500, 500, 500);
	glCallList(SkyDL);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	// draw the box object by calling up its display list:
	SetPointLight(GL_LIGHT0, 0, 0, 0, 1, 1, 1);//sun


	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, SunTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glScalef(.3, .3, .3);
	//glTranslatef(0.f, sin(3.14 * Time), cos(3.14 * Time));
	glCallList(SunDL);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	//Ring Orbit patterns

	glPushMatrix();
	glDisable(GL_LIGHTING);
	glScalef(MercuryRad, 0, MercuryRad);
	glCallList(RingDL);
	glPopMatrix();

	glPushMatrix();
	glScalef(VenusRad, 0, VenusRad);
	glCallList(RingDL);
	glPopMatrix();

	glPushMatrix();
	glScalef(EarthRad, 0, EarthRad);
	glCallList(RingDL);
	glPopMatrix();

	glPushMatrix();
	glScalef(MarsRad, 0, MarsRad);
	glCallList(RingDL);
	glPopMatrix();

	glPushMatrix();
	glScalef(JupiterRad, 0, JupiterRad);
	glCallList(RingDL);
	glPopMatrix();

	glPushMatrix();
	glScalef(SaturnRad, 0, SaturnRad);
	glCallList(RingDL);
	glPopMatrix();

	glPushMatrix();
	glScalef(UranusRad, 0, UranusRad);
	glCallList(RingDL);
	glPopMatrix();

	glPushMatrix();
	glScalef(NeptuneRad, 0, NeptuneRad);
	glCallList(RingDL);
	glPopMatrix();

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	//Planets themselves

	glPushMatrix();
	Radius = MercuryRad;
	Diameter = MercuryDi;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, MercuryTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTranslatef((GLfloat) Radius * sin(OrbitTime(Radius)), 0.f, (GLfloat) Radius * cos(OrbitTime(Radius)));
	glScalef(Diameter, Diameter, Diameter);
	glCallList(MercuryDL);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glPushMatrix();
	Radius = VenusRad;
	Diameter = VenusDi;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, VenusTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTranslatef(Radius* sin(OrbitTime(Radius)), 0.f, Radius* cos(OrbitTime(Radius)));
	glScalef(Diameter, Diameter, Diameter);
	glCallList(VenusDL);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	

	glPushMatrix();
	Radius = EarthRad;
	Diameter = EarthDi;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, EarthTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTranslatef(Radius * sin(OrbitTime(Radius)), 0.f , Radius *cos(OrbitTime(Radius)));
	glScalef(Diameter, Diameter, Diameter);
	glCallList(EarthDL);
	//moon
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glScalef(MoonRad, 1, MoonRad);
	glCallList(RingDL);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	Radius = MoonRad;
	Diameter = MoonDi;
	glBindTexture(GL_TEXTURE_2D, MoonTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTranslatef(Radius* sin(10*OrbitTime(Radius)), 0.f, Radius* cos(10*OrbitTime(Radius)));
	glScalef(Diameter, Diameter, Diameter);
	glCallList(MoonDL);

	

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();



	glPushMatrix();
	Radius = MarsRad;
	Diameter = MarsDi;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, MarsTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTranslatef(Radius * sin(OrbitTime(Radius)), 0.f, Radius * cos(OrbitTime(Radius)));
	glScalef(Diameter, Diameter, Diameter);
	glCallList(MarsDL);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glPushMatrix();
	Radius = JupiterRad;
	Diameter = JupiterDi;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, JupiterTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTranslatef(Radius* sin(OrbitTime(Radius)), 0.f, Radius* cos(OrbitTime(Radius)));
	glScalef(Diameter, Diameter, Diameter);
	glCallList(JupiterDL);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glPushMatrix();
	Radius = SaturnRad;
	Diameter = SaturnDi;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, SaturnTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTranslatef(Radius* sin(OrbitTime(Radius)), 0.f, Radius* cos(OrbitTime(Radius)));
	glScalef(Diameter, Diameter, Diameter);
	glCallList(SaturnDL);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glPushMatrix();
	Radius = UranusRad;
	Diameter = UranusDi;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, UranusTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTranslatef(Radius* sin(OrbitTime(Radius)), 0.f, Radius* cos(OrbitTime(Radius)));
	glScalef(Diameter, Diameter, Diameter);
	glCallList(UranusDL);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glPushMatrix();
	Radius = NeptuneRad;
	Diameter = NeptuneDi;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, NeptuneTex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTranslatef(Radius* sin(OrbitTime(Radius)), 0.f, Radius* cos(OrbitTime(Radius)));
	glScalef(Diameter, Diameter, Diameter);
	glCallList(NeptuneDL);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glEnable(GL_TEXTURE_2D);


	/*SetPointLight(GL_LIGHT0, 0, 0, 0, 1, 1, 1);
		glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_TEXTURE_2D);
	*/
	
	/*
	if(LightingOn)
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	else
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	if(NowTexture == Venus) glCallList(VenusDL);
	if(NowTexture == Earth) glCallList(EarthDL);
	if(NowTexture == Moon) glCallList(MoonDL);
	if(NowTexture == Jupiter) glCallList(JupiterDL);
	if(NowTexture == Saturn) glCallList(SaturnDL);
	if(NowTexture == Neptune) glCallList(NeptuneDL);
	if(NowTexture == Uranus) glCallList(UranusDL);
	if(NowTexture == Pluto) glCallList(PlutoDL);
	glDisable(GL_TEXTURE_2D);*/
	glDisable(GL_LIGHTING);

#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.f,   0.f, 1.f, 0.f );
			glCallList( BoxList );
		glPopMatrix( );
	}
#endif


	// draw some gratuitous text that just rotates on top of the scene:
	// i commented out the actual text-drawing calls -- put them back in if you have a use for them
	// a good use for thefirst one might be to have your name on the screen
	// a good use for the second one might be to have vertex numbers on the screen alongside each vertex

	glDisable( GL_DEPTH_TEST );
	glColor3f( 0.f, 1.f, 1.f );
	//DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );


	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates
	char timeStr[50]; // Buffer to hold the converted string
	sprintf(timeStr, "Time Scale: %.2f", Timescale/50);


	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0.f, 100.f,     0.f, 100.f );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1.f, 1.f, 1.f );
	//DoRasterString( 5.f, 5.f, 0.f, (char *)"Text That Doesn't" );
	// 
	DoRasterString(5.f, 15.f, 0.f, (char*)timeStr);
	// swap the double-buffered framebuffers:

	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	NowColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	NowProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

void
DoTexture(int id)
{
	NowTexture = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}



// initialize the glut and OpenGL libraries:
//	also setup callback functions

void
InitGraphics( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc( Animate );

	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// all other setups go here, such as GLSLProgram and KeyTime setups:
	int width, height;
	char* file = (char*)"venus.bmp";
	unsigned char* texture = BmpToTexture(file, &width, &height);
	if(texture == NULL)
		fprintf(stderr, "Cannot open texture '%s'\n", file);
	else
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", file, width, height);
	glGenTextures(1, &VenusTex);
	glBindTexture(GL_TEXTURE_2D, VenusTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);

	int width1, height1;
	char* file1 = (char*)"earth.bmp";
	unsigned char* texture1 = BmpToTexture(file1, &width1, &height1);
	if (texture1 == NULL)
	fprintf(stderr, "Cannot open texture '%s'\n", file1);
	else
	fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", file1, width1, height1);
	glGenTextures(1, &EarthTex);
	glBindTexture(GL_TEXTURE_2D, EarthTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, texture1);
	int width2, height2;
	char* file2 = (char*)"moon.bmp";
	unsigned char* texture2 = BmpToTexture(file2, &width2, &height2);
	if (texture2 == NULL)
		fprintf(stderr, "Cannot open texture '%s'\n", file2);
	else
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", file2, width2, height2);
	glGenTextures(1, &MoonTex);
	glBindTexture(GL_TEXTURE_2D, MoonTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, texture2);

	int width3, height3;
	char* file3 = (char*)"jupiter.bmp";
	unsigned char* texture3 = BmpToTexture(file3, &width3, &height3);
	if (texture3 == NULL)
		fprintf(stderr, "Cannot open texture '%s'\n", file3);
	else
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", file3, width3, height3);
	glGenTextures(1, &JupiterTex);
	glBindTexture(GL_TEXTURE_2D, JupiterTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width3, height3, 0, GL_RGB, GL_UNSIGNED_BYTE, texture3);

	int width4, height4;
	char* file4 = (char*)"saturn.bmp";
	unsigned char* texture4 = BmpToTexture(file4, &width4, &height4);
	if (texture4 == NULL)
		fprintf(stderr, "Cannot open texture '%s'\n", file4);
	else
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", file4, width4, height4);
	glGenTextures(1, &SaturnTex);
	glBindTexture(GL_TEXTURE_2D, SaturnTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width4, height4, 0, GL_RGB, GL_UNSIGNED_BYTE, texture4);
	
	int width5, height5;
	char* file5 = (char*)"neptune.bmp";
	unsigned char* texture5 = BmpToTexture(file5, &width5, &height5);
	if (texture5 == NULL)
		fprintf(stderr, "Cannot open texture '%s'\n", file5);
	else
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", file5, width5, height5);
	glGenTextures(1, &NeptuneTex);
	glBindTexture(GL_TEXTURE_2D, NeptuneTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width5, height5, 0, GL_RGB, GL_UNSIGNED_BYTE, texture5);

	int width6, height6;
	char* file6 = (char*)"uranus.bmp";
	unsigned char* texture6 = BmpToTexture(file6, &width6, &height6);
	if (texture6 == NULL)
		fprintf(stderr, "Cannot open texture '%s'\n", file4);
	else
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", file6, width6, height6);
	glGenTextures(1, &UranusTex);
	glBindTexture(GL_TEXTURE_2D, UranusTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width6, height6, 0, GL_RGB, GL_UNSIGNED_BYTE, texture6);

	int width7, height7;
	char* file7 = (char*)"pluto.bmp";
	unsigned char* texture7 = BmpToTexture(file7, &width7, &height7);
	if (texture7 == NULL)
		fprintf(stderr, "Cannot open texture '%s'\n", file7);
	else
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", file7, width7, height7);
	glGenTextures(1, &PlutoTex);
	glBindTexture(GL_TEXTURE_2D, PlutoTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width7, height7, 0, GL_RGB, GL_UNSIGNED_BYTE, texture7);

	int width8, height8;
	char* file8 = (char*)"mars.bmp";
	unsigned char* texture8 = BmpToTexture(file8, &width8, &height8);
	if (texture8 == NULL)
		fprintf(stderr, "Cannot open texture '%s'\n", file8);
	else
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", file8, width8, height8);
	glGenTextures(1, &MarsTex);
	glBindTexture(GL_TEXTURE_2D, MarsTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width8, height8, 0, GL_RGB, GL_UNSIGNED_BYTE, texture8);

	int width9, height9;
	char* file9 = (char*)"sun.bmp";
	unsigned char* texture9 = BmpToTexture(file9, &width9, &height9);
	if (texture9 == NULL)
		fprintf(stderr, "Cannot open texture '%s'\n", file9);
	else
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", file9, width9, height9);
	glGenTextures(1, &SunTex);
	glBindTexture(GL_TEXTURE_2D, SunTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width9, height9, 0, GL_RGB, GL_UNSIGNED_BYTE, texture9);

	int widtha, heighta;
	char* filea = (char*)"Milky.bmp";
	unsigned char* texturea = BmpToTexture(filea, &widtha, &heighta);
	if (texturea == NULL)
		fprintf(stderr, "Cannot open texture '%s'\n", filea);
	else
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", filea, widtha, heighta);
	glGenTextures(1, &SkyTex);
	glBindTexture(GL_TEXTURE_2D, SkyTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widtha, heighta, 0, GL_RGB, GL_UNSIGNED_BYTE, texturea);

	int widthb, heightb;
	char* fileb = (char*)"mercury.bmp";
	unsigned char* textureb = BmpToTexture(fileb, &widthb, &heightb);
	if (textureb == NULL)
		fprintf(stderr, "Cannot open texture '%s'\n", fileb);
	else
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", fileb, widthb, heightb);
	glGenTextures(1, &MercuryTex);
	glBindTexture(GL_TEXTURE_2D, MercuryTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthb, heightb, 0, GL_RGB, GL_UNSIGNED_BYTE, textureb);
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	float dx = BOXSIZE / 2.f;
	float dy = BOXSIZE / 2.f;
	float dz = BOXSIZE / 2.f;
	glutSetWindow( MainWindow );

	// create the object:

	SphereDL = glGenLists(1);
	glNewList(SphereDL, GL_COMPILE);
	OsuSphere(1., 100 , 100 ); // use enough polygonal detail to look nice
	glEndList();

	VenusDL = glGenLists(1);
	glNewList(VenusDL, GL_COMPILE);
	glPushMatrix();
	glScalef(1.f, 1.f, 1.f); // scale of venus sphere, from the table
	glCallList(SphereDL); // a dl can call another dl that has been
	// previously created
	glPopMatrix();
	glEndList();

	EarthDL = glGenLists(1);
	glNewList(EarthDL, GL_COMPILE);
	glPushMatrix();
	glScalef(1.f, 1.f, 1.f); // scale of venus sphere, from the table
	glCallList(SphereDL); // a dl can call another dl that has been
	// previously created
	glPopMatrix();
	glEndList();

	MoonDL = glGenLists(1);
	glNewList(MoonDL, GL_COMPILE);
	glPushMatrix();
	glScalef(1.f, 1.f, 1.f); // scale of venus sphere, from the table
	glCallList(SphereDL); // a dl can call another dl that has been
	// previously created
	glPopMatrix();
	glEndList();

	JupiterDL = glGenLists(1);
	glNewList(JupiterDL, GL_COMPILE);
	glPushMatrix();
	glScalef(1.f, 1.f, 1.f); // scale of venus sphere, from the table
	glCallList(SphereDL); // a dl can call another dl that has been
	// previously created
	glPopMatrix();
	glEndList();

	SaturnDL = glGenLists(1);
	glNewList(SaturnDL, GL_COMPILE);
	glPushMatrix();
	glScalef(1.f, 1.f, 1.f); // scale of venus sphere, from the table
	glCallList(SphereDL); // a dl can call another dl that has been
	// previously created
	glPopMatrix();
	glEndList();

	NeptuneDL = glGenLists(1);
	glNewList(NeptuneDL, GL_COMPILE);
	glPushMatrix();
	glScalef(1.f, 1.f, 1.f); // scale of venus sphere, from the table
	glCallList(SphereDL); // a dl can call another dl that has been
	// previously created
	glPopMatrix();
	glEndList();

	UranusDL = glGenLists(1);
	glNewList(UranusDL, GL_COMPILE);
	glPushMatrix();
	glScalef(1.f, 1.f, 1.f); // scale of venus sphere, from the table
	glCallList(SphereDL); // a dl can call another dl that has been
	// previously created
	glPopMatrix();
	glEndList();

	PlutoDL = glGenLists(1);
	glNewList(PlutoDL, GL_COMPILE);
	glPushMatrix();
	glScalef(1.f, 1.f, 1.f); // scale of venus sphere, from the table
	glCallList(SphereDL); // a dl can call another dl that has been
	// previously created
	glPopMatrix();
	glEndList();

	MarsDL = glGenLists(1);
	glNewList(MarsDL, GL_COMPILE);
	glPushMatrix();
	glScalef(1.f, 1.f, 1.f); // scale of venus sphere, from the table
	glCallList(SphereDL); // a dl can call another dl that has been
	// previously created
	glPopMatrix();
	glEndList();

	SunDL = glGenLists(1);
	glNewList(SunDL, GL_COMPILE);
	glPushMatrix();
	glScalef(1.f, 1.f, 1.f); // scale of venus sphere, from the table
	glCallList(SphereDL); // a dl can call another dl that has been
	// previously created
	glPopMatrix();
	glEndList();

	SkyDL = glGenLists(1);
	glNewList(SkyDL, GL_COMPILE);
	glPushMatrix();
	glScalef(1.f, 1.f, 1.f); // scale of venus sphere, from the table
	glCallList(SphereDL); // a dl can call another dl that has been
	// previously created
	glPopMatrix();
	glEndList();

	MercuryDL = glGenLists(1);
	glNewList(MercuryDL, GL_COMPILE);
	glPushMatrix();
	glScalef(1.f, 1.f, 1.f); // scale of venus sphere, from the table
	glCallList(SphereDL); // a dl can call another dl that has been
	// previously created
	glPopMatrix();
	glEndList();

	RingDL = glGenLists(1); // Generate a display list
	glNewList(RingDL, GL_COMPILE);

	glColor3f(1.0f, 1.0f, 1.0f); // Set color to white
	glBegin(GL_LINE_LOOP);       // Draw a loop of lines
	for (int i = 0; i < 100; ++i) {
		float angle = 2.0f * M_PI * i / 100; // Angle for each segment
		float x = 1 * cos(angle);
		float z = 1 * sin(angle);
		glVertex3f(x, 0.0f, z); // Set vertex position on the ring
	}
	glEnd();

	glEndList();

#ifdef NOTDEF
		glColor3f(1., 1., 1.);
		glBegin(GL_TRIANGLES);
		glVertex3f(-dx, -dy, dz);
		glVertex3f(0., -dy, dz + 0.5f);
		glVertex3f(dx, -dy, dz);
		glEnd();
#endif

	glEndList( );


	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );
}


// initialize the glui window:

void
InitMenus( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitMenus.\n");

	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(float) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Axis Colors",   colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'o':
		case 'O':
			NowProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			NowProjection = PERSP;
			break;
		case '0':
			NowTexture = None;
			break;
		case '1':
			NowTexture = Venus;
			break;
		case '2':
			NowTexture = Earth;
			break;
		case '3':
			NowTexture = Moon;
			break;
		case '4':
			NowTexture = Jupiter;
			break;
		case '5':
			NowTexture = Saturn;
			break;
		case '6':
			NowTexture = Neptune;
			break;
		case '7':
			NowTexture = Uranus;
			break;
		case '8':
			NowTexture = Pluto;
			break;
		case 'n':
		case 'N':
			Timescale += 50;
			break;
		case 'm':
		case 'M':
			Timescale -= 50;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	ShadowsOn = 0;
	NowColor = YELLOW;
	NowProjection = PERSP;
	Xrot = Yrot = 0.;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = { 0.f, 1.f, 0.f, 1.f };

static float xy[ ] = { -.5f, .5f, .5f, -.5f };

static int xorder[ ] = { 1, 2, -3, 4 };

static float yx[ ] = { 0.f, 0.f, -.5f, .5f };

static float yy[ ] = { 0.f, .6f, 1.f, 1.f };

static int yorder[ ] = { 1, 2, 3, -2, 4 };

static float zx[ ] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[ ] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[ ] = { 1, 2, 3, 4, -5, 6 };

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r=0., g=0., b=0.;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}


float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}


float
Unit( float v[3] )
{
	float dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		v[0] /= dist;
		v[1] /= dist;
		v[2] /= dist;
	}
	return dist;
}
