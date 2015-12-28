/*
  Height Fields
  By Xu Wu
*/

#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>

int rendertype=2; /*defaut render type is triangle*/
bool save=false;  /*save function is initially closed*/
int amoutofimage=0; /*declare the saved image amout*/

int g_iMenuId;

int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;

CONTROLSTATE g_ControlState = ROTATE;

/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

/* see <your pic directory>/pic.h for type Pic */
Pic * g_pHeightData;



/* Write a screenshot to the specified filename */
void saveScreenshot (char *filename)
{
  int i, j;
  Pic *in = NULL;

  if (filename == NULL)
    return;

  /* Allocate a picture buffer */
  in = pic_alloc(640, 480, 3, NULL);

  printf("File to save to: %s\n", filename);

  for (i=479; i>=0; i--) {
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                 &in->pix[i*in->nx*in->bpp]);
  }

  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);
}


void myinit()
{
  /* setup gl view here */
  glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH); /*declare the shade mode*/
    
}

/* render points function*/
void point(){
    int scale=g_pHeightData->nx; /*image's scale*/
    glBegin(GL_POINTS);
    for(int i=0;i<g_pHeightData->nx;i++){
        for(int j=0;j<g_pHeightData->ny;j++){
            int h=PIC_PIXEL(g_pHeightData, i, j, 0);
            glColor3f(h/(1.0*scale), 0.0, 0.8); /*color gradient allocation*/
            glVertex3i(j,i,h);
        }
    }
    glEnd();
}

/* render wireframes function */
void line(){
    int scale=g_pHeightData->nx;
    for (int i = 0; i < g_pHeightData->ny-1;i++)
    {
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j < g_pHeightData->nx;j++)
        {
            int h1 = PIC_PIXEL(g_pHeightData, j, i, 0);
            int h2 = PIC_PIXEL(g_pHeightData, j, i+1, 0);
            glColor3f(h1/(1.0*scale), 0.0, 0.8); /*color gradient allocation*/
            glVertex3i(j, i, h1);
            glColor3f(h2/(1.0*scale), 0.0, 0.8);
            glVertex3i(j,i+1, h2);
        }
        glEnd();
    }
    
}

/* render triangles function */
void triangle(){
    int scale=g_pHeightData->nx;
    for (int i = 0; i < g_pHeightData->ny-1;i++)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j < g_pHeightData->nx;j++)
        {
            int h1 = PIC_PIXEL(g_pHeightData, j, i, 0);
            int h2 = PIC_PIXEL(g_pHeightData, j, i+1, 0);
            glColor3f(h1/(1.0*scale), 0.0, 0.8); /*color gradient allocation*/
            glVertex3i(j, i, h1);
            glColor3f(h2/(1.0*scale), 0.0, 0.8);
            glVertex3i(j,i+1, h2);
        }
        glEnd();
    }
}

void display()
{
  /* draw 1x1 cube about origin */
  /* replace this code with your height field implementation */
  /* you may also want to precede it with your 
rotation/translation/scaling */
    
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(-220.0, -220.0, g_pHeightData->nx*1.0+100.0, 0.0, 0.0, g_pHeightData->nx*1.0, 0.0, 0.0, 1.0); /* veiwing transformation, put the camera and reference point properly so that rendered object can be viewed, the camera position in z axis must higher than the lagest value of the grayscale 'height'*/
    
    /* rotation/translation/scaling */
    glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);
    glRotatef(g_vLandRotate[0], 0.0, 0.0, 1.0);
    glRotatef(g_vLandRotate[1], 0.0, 1.0, 0.0);
    glRotatef(g_vLandRotate[2], 1.0, 0.0, 0.0);
    glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);
    
    /* render process */
    glPushMatrix();
    
    switch(rendertype){
        case 0:
            point();
            break;
        case 1:
            line();
            break;
        case 2:
            triangle(); /*default mode */
            break;
            
    }
    
    
    glPopMatrix();
    glutSwapBuffers();
}

/* renshape callback function implement */
void reshape(int w, int h)
{
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 0.01, 1000.0); /* projection transformation */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void menufunc(int value)
{
  switch (value)
  {
    case 0:
      exit(0);
      break;
  }
}

void doIdle()
{
  /* do some stuff... */
  /* save image when trigger the keyboard 's' */
    if(save){
        char resultframe[2048];
            sprintf(resultframe,"%03d.jpg",amoutofimage);
        if(amoutofimage<300){
            saveScreenshot(resultframe);
            
        }
        else{
            save=false;
        }
        amoutofimage++;
    }
  /* make the screen update */
  glutPostRedisplay();
}

/* converts mouse drags into information about 
rotation/translation/scaling */
void mousedrag(int x, int y)
{
  int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
  
  switch (g_ControlState)
  {
    case TRANSLATE:  
      if (g_iLeftMouseButton)
      {
        g_vLandTranslate[0] += vMouseDelta[0]*0.01;
        g_vLandTranslate[1] -= vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandTranslate[2] += vMouseDelta[1]*0.01;
      }
      break;
    case ROTATE:
      if (g_iLeftMouseButton)
      {
        g_vLandRotate[0] += vMouseDelta[1];
        g_vLandRotate[1] += vMouseDelta[0];
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandRotate[2] += vMouseDelta[1];
      }
      break;
    case SCALE:
      if (g_iLeftMouseButton)
      {
        g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
        g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
      }
      break;
  }
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
  }
 
  switch(glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      g_ControlState = TRANSLATE;
      break;
    case GLUT_ACTIVE_SHIFT:
      g_ControlState = SCALE;
      break;
    default:
      g_ControlState = ROTATE;
      break;
  }

  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

/* keyboard callback function implement*/
void keyboard(unsigned char c, int x, int y){
    switch(c){
            case 'p':
                rendertype=0;
                break;
            case 'l':
                rendertype=1;
                break;
            case 't':
                rendertype=2;
                break;
            case 's':
                save=true;
                break;
    }
}

int main (int argc, char ** argv)
{
  if (argc<2)
  {  
    printf ("usage: %s heightfield.jpg\n", argv[0]);
    exit(1);
  }

  g_pHeightData = jpeg_read(argv[1], NULL);
  if (!g_pHeightData)
  {
    printf ("error reading %s.\n", argv[1]);
    exit(1);
  }

  glutInit(&argc,argv);
  
  /*
    create a window here..should be double buffered and use depth testing
  
    the code past here will segfault if you don't have a window set up....
    replace the exit once you add those calls.
  */
//  exit(0);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Height Field");
    
  /* tells glut to use a particular display function to redraw */
  glutDisplayFunc(display);
    
  /* allow the user to quit using the right mouse button menu */
  g_iMenuId = glutCreateMenu(menufunc);
  glutSetMenu(g_iMenuId);
  glutAddMenuEntry("Quit",0);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  
  /* replace with any animate code */
  glutIdleFunc(doIdle);

  /* callback for mouse drags */
  glutMotionFunc(mousedrag);
  /* callback for idle mouse movement */
  glutPassiveMotionFunc(mouseidle);
  /* callback for mouse button changes */
  glutMouseFunc(mousebutton);
    
    glutKeyboardFunc(keyboard);

  /* do initialization */
  myinit();
    glutReshapeFunc(reshape);
    glEnable(GL_DEPTH_TEST); /*enable the depth buffering */
  glutMainLoop();
  return(0);
}
