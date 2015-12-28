/*
roller coaster
By Xu Wu

 */

#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include "pic.h"
#include <string.h>
#include <cmath>


bool save=false;  /*save function is initially closed*/
int amoutofimage=0; /*declare the saved image amout*/

const GLfloat CUBE_SIZE = 50.0f;// sky cube size

GLuint texture[7]; // texture arrays 

// camera ride parameters
float lookat_u=0.0f; // u value for the ride
int spline_num=0; // the # of splines for the ride
int point_num=0;  //  the # of points for the ride
float eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz; // parameters for glulookat function



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


/* represents one control point along the spline */
struct point {
   double x;
   double y;
   double z;
};

// points and vectors initailization
  struct point lookat_pos, lookat_T, lookat_N, lookat_B,v0,v1,v2,v3,v4,v5,v6,v7,k0,k1,k2,k3,k4,k5,k6,k7;
  struct point* lookat_control_matrix;
  // lookat_control_matrix=new point [4];

/* spline struct which contains how many control points, and an array of control points */
struct spline {
   int numControlPoints;
   struct point *points;
};

/* the spline array */
struct spline *g_Splines;

/* total number of splines */
int g_iNumOfSplines;


int loadSplines(char *argv) {
  char *cName = (char *)malloc(128 * sizeof(char));
  FILE *fileList;
  FILE *fileSpline;
  int iType, i = 0, j, iLength;


  /* load the track file */
  fileList = fopen(argv, "r");
  if (fileList == NULL) {
    printf ("can't open file\n");
    exit(1);
  }
  
  /* stores the number of splines in a global variable */
  fscanf(fileList, "%d", &g_iNumOfSplines);

  g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

  /* reads through the spline files */
  for (j = 0; j < g_iNumOfSplines; j++) {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL) {
      printf ("can't open file\n");
      exit(1);
    }

    /* gets length for spline file */
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    /* allocate memory for all the points */
    g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
    g_Splines[j].numControlPoints = iLength;

    /* saves the data to the struct */
    while (fscanf(fileSpline, "%lf %lf %lf", 
	   &g_Splines[j].points[i].x, 
	   &g_Splines[j].points[i].y, 
	   &g_Splines[j].points[i].z) != EOF) {
      i++;
    }
  }

  free(cName);

  return 0;
}

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


//load pictures
void textload(int i, char *filename){
  Pic* img;
  img = jpeg_read(filename, NULL);
  glBindTexture(GL_TEXTURE_2D, texture[i]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->nx, img->ny, 0, GL_RGB, GL_UNSIGNED_BYTE, &img->pix[0]);
  pic_free(img);
}

//get control matrix with 4 points
struct point* getControlMatrix(int spline_num1, int point_num1)
{
  struct point* control_matrix;
  control_matrix=new point [4];
  for(int i=0;i<4;i++){
    control_matrix[i].x = g_Splines[spline_num1].points[point_num1+i].x;
    control_matrix[i].y = g_Splines[spline_num1].points[point_num1+i].y;
    control_matrix[i].z = g_Splines[spline_num1].points[point_num1+i].z;
  }
  return control_matrix;
}

void caculate_unit_vector(float &x, float &y, float &z){
    float mag=sqrt(x*x+y*y+z*z);
    x=x/mag;
    y=y/mag;
    z=z/mag;
}

// T,N,B caculation
void TNB_handler(float u){
    if(point_num==0&&u==0){
      float t0_x=-0.5*lookat_control_matrix[0].x+0.5*lookat_control_matrix[2].x;
      float t0_y=-0.5*lookat_control_matrix[0].y+0.5*lookat_control_matrix[2].y;
      float t0_z=-0.5*lookat_control_matrix[0].z+0.5*lookat_control_matrix[2].z;
      caculate_unit_vector(t0_x,t0_y,t0_z);
      lookat_T.x=t0_x;
      lookat_T.y=t0_y;
      lookat_T.z=t0_z;

      float v_x=0;
      float v_y=1;
      float v_z=0;

      float n0_x = t0_y * v_z - t0_z * v_y;
      float n0_y = t0_z * v_x - t0_x * v_z;
      float n0_z = t0_x * v_y - t0_y * v_x;
      caculate_unit_vector(n0_x,n0_y,n0_z);
      lookat_N.x=n0_x;
      lookat_N.y=n0_y;
      lookat_N.z=n0_z;

      float b0_x= t0_y*n0_z- t0_z*n0_y;
      float b0_y= t0_z*n0_x- t0_x*n0_z;
      float b0_z= t0_x*n0_y- t0_y*n0_x;
      caculate_unit_vector(b0_x,b0_y,b0_z);
      lookat_B.x=b0_x;
      lookat_B.y=b0_y;
      lookat_B.z=b0_z;

    }

    else{
      float tx = (-0.5*3*u*u+2*u-0.5*1)*lookat_control_matrix[0].x+(1.5*3*u*u-2.5*2*u)*lookat_control_matrix[1].x+(-1.5*3*u*u+2*2*u+0.5*1)*lookat_control_matrix[2].x+(0.5*3*u*u-0.5*2*u)*lookat_control_matrix[3].x;
      float ty = (-0.5*3*u*u+2*u-0.5*1)*lookat_control_matrix[0].y+(1.5*3*u*u-2.5*2*u)*lookat_control_matrix[1].y+(-1.5*3*u*u+2*2*u+0.5*1)*lookat_control_matrix[2].y+(0.5*3*u*u-0.5*2*u)*lookat_control_matrix[3].y;
      float tz = (-0.5*3*u*u+2*u-0.5*1)*lookat_control_matrix[0].z+(1.5*3*u*u-2.5*2*u)*lookat_control_matrix[1].z+(-1.5*3*u*u+2*2*u+0.5*1)*lookat_control_matrix[2].z+(0.5*3*u*u-0.5*2*u)*lookat_control_matrix[3].z;
      caculate_unit_vector(tx,ty,tz);
      lookat_T.x=tx;
      lookat_T.y=ty;
      lookat_T.z=tz;

      float b0_x=lookat_B.x;
      float b0_y=lookat_B.y;
      float b0_z=lookat_B.z;

      float nx=b0_y*tz-b0_z*ty;
      float ny=b0_z*tx-b0_x*tz;
      float nz=b0_x*ty-b0_y*tx;
      caculate_unit_vector(nx,ny,nz);
      lookat_N.x=nx;
      lookat_N.y=ny;
      lookat_N.z=nz;

      float bx=(ty*nz-tz*ny);
      float by=(tz*nx-tx*nz);
      float bz=(tx*ny-ty*nx);
      lookat_B.x=bx;
      lookat_B.y=by;
      lookat_B.z=bz;   

    }

}


void myinit(){
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST); 

// load textures 
  glGenTextures(1,&texture[0]);
  textload(0,"ground.jpg");

  glGenTextures(1,&texture[1]);
  textload(1,"sky.jpg");

  glGenTextures(1,&texture[2]);
  textload(2,"sky.jpg");

  glGenTextures(1,&texture[3]);
  textload(3,"sky.jpg");

  glGenTextures(1,&texture[4]);
  textload(4,"sky.jpg");

  glGenTextures(1,&texture[5]);
  textload(5,"sky.jpg");

  glGenTextures(1,&texture[6]);
  textload(6,"trail.jpg");

}

void splines(){
    

  for(int spline_num2=0;spline_num2<g_iNumOfSplines;spline_num2++){
        

    struct point* c_matrix;
    float p0x,p0y,p0z,t0x,t0y,t0z,n0x,n0y,n0z,b0x,b0y,b0z,p1x,p1y,p1z,t1x,t1y,t1z,n1x,n1y,n1z,b1x,b1y,b1z;
    float v0x=0;
    float v0y=1;
    float v0z=0;
    float v1x=0;
    float v1y=1;
    float v1z=0;
    float w1=0.02,w2=0.03,h=0.005;

// for every control point, plug in u from 0 to 1
  for(int i=0;i<g_Splines[spline_num2].numControlPoints-3;i++){
    c_matrix=new point[4];
    c_matrix=getControlMatrix(spline_num2,i);

    for(float u=0;u<1;u+=0.001){
      //point p0 position
      p0x=(-0.5*u*u*u+u*u-0.5*u)*c_matrix[0].x+(1.5*u*u*u-2.5*u*u+1)*c_matrix[1].x+(-1.5*u*u*u+2*u*u+0.5*u)*c_matrix[2].x+(0.5*u*u*u-0.5*u*u)*c_matrix[3].x;
      p0y=(-0.5*u*u*u+u*u-0.5*u)*c_matrix[0].y+(1.5*u*u*u-2.5*u*u+1)*c_matrix[1].y+(-1.5*u*u*u+2*u*u+0.5*u)*c_matrix[2].y+(0.5*u*u*u-0.5*u*u)*c_matrix[3].y;
      p0z=(-0.5*u*u*u+u*u-0.5*u)*c_matrix[0].z+(1.5*u*u*u-2.5*u*u+1)*c_matrix[1].z+(-1.5*u*u*u+2*u*u+0.5*u)*c_matrix[2].z+(0.5*u*u*u-0.5*u*u)*c_matrix[3].z;
      
      // consider the first point with arbitary V
      if(i==0&&u==0){
      t0x=-0.5*c_matrix[0].x+0.5*c_matrix[2].x;
      t0y=-0.5*c_matrix[0].y+0.5*c_matrix[2].y;
      t0z=-0.5*c_matrix[0].z+0.5*c_matrix[2].z;
      caculate_unit_vector(t0x,t0y,t0z);


      n0x = t0y * v0z - t0z * v0y;
      n0y = t0z * v0x - t0x * v0z;
      n0z = t0x * v0y - t0y * v0x;
      caculate_unit_vector(n0x,n0y,n0z);


      b0x= t0y*n0z- t0z*n0y;
      b0y= t0z*n0x- t0x*n0z;
      b0z= t0x*n0y- t0y*n0x;
      caculate_unit_vector(b0x,b0y,b0z);


    }

    else{
      t0x = (-0.5*3*u*u+2*u-0.5*1)*c_matrix[0].x+(1.5*3*u*u-2.5*2*u)*c_matrix[1].x+(-1.5*3*u*u+2*2*u+0.5*1)*c_matrix[2].x+(0.5*3*u*u-0.5*2*u)*c_matrix[3].x;
      t0y = (-0.5*3*u*u+2*u-0.5*1)*c_matrix[0].y+(1.5*3*u*u-2.5*2*u)*c_matrix[1].y+(-1.5*3*u*u+2*2*u+0.5*1)*c_matrix[2].y+(0.5*3*u*u-0.5*2*u)*c_matrix[3].y;
      t0z = (-0.5*3*u*u+2*u-0.5*1)*c_matrix[0].z+(1.5*3*u*u-2.5*2*u)*c_matrix[1].z+(-1.5*3*u*u+2*2*u+0.5*1)*c_matrix[2].z+(0.5*3*u*u-0.5*2*u)*c_matrix[3].z;
      caculate_unit_vector(t0x,t0y,t0z);
 

      n0x=b0y*t0z-b0z*t0y;
      n0y=b0z*t0x-b0x*t0z;
      n0z=b0x*t0y-b0y*t0x;
      caculate_unit_vector(n0x,n0y,n0z);

      b0x= t0y*n0z- t0z*n0y;
      b0y= t0z*n0x- t0x*n0z;
      b0z= t0x*n0y- t0y*n0x;
      caculate_unit_vector(b0x,b0y,b0z);
    }
//left trail
    //right bot point
    v0.x=p0x-h*n0x-w1*b0x;
    v0.y=p0y-h*n0y-w1*b0y;
    v0.z=p0z-h*n0z-w1*b0z;

    //right top point
    v1.x=p0x+h*n0x-w1*b0x;
    v1.y=p0y+h*n0y-w1*b0y;
    v1.z=p0z+h*n0z-w1*b0z;

    //left top point
    v2.x=p0x+h*n0x-w2*b0x;
    v2.y=p0y+h*n0y-w2*b0y;
    v2.z=p0z+h*n0z-w2*b0z;

    //left bot point
    v3.x=p0x-h*n0x-w2*b0x;
    v3.y=p0y-h*n0y-w2*b0y;
    v3.z=p0z-h*n0z-w2*b0z;
//right trail
    //right bot point
    k0.x=p0x-h*n0x+w2*b0x;
    k0.y=p0y-h*n0y+w2*b0y;
    k0.z=p0z-h*n0z+w2*b0z;

    //right top point
    k1.x=p0x+h*n0x+w2*b0x;
    k1.y=p0y+h*n0y+w2*b0y;
    k1.z=p0z+h*n0z+w2*b0z;

    //left top point
    k2.x=p0x+h*n0x+w1*b0x;
    k2.y=p0y+h*n0y+w1*b0y;
    k2.z=p0z+h*n0z+w1*b0z;

    //left bot point
    k3.x=p0x-h*n0x+w1*b0x;
    k3.y=p0y-h*n0y+w1*b0y;
    k3.z=p0z-h*n0z+w1*b0z;


//next point p1 which will be connected
      float u1=u+0.001;
    //point p1 position
      p1x=(-0.5*u1*u1*u1+u1*u1-0.5*u1)*c_matrix[0].x+(1.5*u1*u1*u1-2.5*u1*u1+1)*c_matrix[1].x+(-1.5*u1*u1*u1+2*u1*u1+0.5*u1)*c_matrix[2].x+(0.5*u1*u1*u1-0.5*u1*u1)*c_matrix[3].x;
      p1y=(-0.5*u1*u1*u1+u1*u1-0.5*u1)*c_matrix[0].y+(1.5*u1*u1*u1-2.5*u1*u1+1)*c_matrix[1].y+(-1.5*u1*u1*u1+2*u1*u1+0.5*u1)*c_matrix[2].y+(0.5*u1*u1*u1-0.5*u1*u1)*c_matrix[3].y;
      p1z=(-0.5*u1*u1*u1+u1*u1-0.5*u1)*c_matrix[0].z+(1.5*u1*u1*u1-2.5*u1*u1+1)*c_matrix[1].z+(-1.5*u1*u1*u1+2*u1*u1+0.5*u1)*c_matrix[2].z+(0.5*u1*u1*u1-0.5*u1*u1)*c_matrix[3].z;
      
      //tangent vecotor
      t1x = (-0.5*3*u1*u1+2*u1-0.5*1)*c_matrix[0].x+(1.5*3*u1*u1-2.5*2*u1)*c_matrix[1].x+(-1.5*3*u1*u1+2*2*u1+0.5*1)*c_matrix[2].x+(0.5*3*u1*u1-0.5*2*u1)*c_matrix[3].x;
      t1y = (-0.5*3*u1*u1+2*u1-0.5*1)*c_matrix[0].y+(1.5*3*u1*u1-2.5*2*u1)*c_matrix[1].y+(-1.5*3*u1*u1+2*2*u1+0.5*1)*c_matrix[2].y+(0.5*3*u1*u1-0.5*2*u1)*c_matrix[3].y;
      t1z = (-0.5*3*u1*u1+2*u1-0.5*1)*c_matrix[0].z+(1.5*3*u1*u1-2.5*2*u1)*c_matrix[1].z+(-1.5*3*u1*u1+2*2*u1+0.5*1)*c_matrix[2].z+(0.5*3*u1*u1-0.5*2*u1)*c_matrix[3].z;
      caculate_unit_vector(t1x,t1y,t1z);


      n1x=b0y*t1z-b0z*t1y;
      n1y=b0z*t1x-b0x*t1z;
      n1z=b0x*t1y-b0y*t1x;
      caculate_unit_vector(n1x,n1y,n1z);

      b1x= t1y*n1z- t1z*n1y;
      b1y= t1z*n1x- t1x*n1z;
      b1z= t1x*n1y- t1y*n1x;
      caculate_unit_vector(b1x,b1y,b1z);
    
//left trail
    //right bot point
    v4.x=p1x-h*n1x-w1*b1x;
    v4.y=p1y-h*n1y-w1*b1y;
    v4.z=p1z-h*n1z-w1*b1z;

    //right top point
    v5.x=p1x+h*n1x-w1*b1x;
    v5.y=p1y+h*n1y-w1*b1y;
    v5.z=p1z+h*n1z-w1*b1z;

    //left top point
    v6.x=p1x+h*n1x-w2*b1x;
    v6.y=p1y+h*n1y-w2*b1y;
    v6.z=p1z+h*n1z-w2*b1z;

    //left bot point
    v7.x=p1x-h*n1x-w2*b1x;
    v7.y=p1y-h*n1y-w2*b1y;
    v7.z=p1z-h*n1z-w2*b1z;
//right trail
    //right bot point
    k4.x=p1x-h*n1x+w2*b1x;
    k4.y=p1y-h*n1y+w2*b1y;
    k4.z=p1z-h*n1z+w2*b1z;

    //right top point
    k5.x=p1x+h*n1x+w2*b1x;
    k5.y=p1y+h*n1y+w2*b1y;
    k5.z=p1z+h*n1z+w2*b1z;

    //left top point
    k6.x=p1x+h*n1x+w1*b1x;
    k6.y=p1y+h*n1y+w1*b1y;
    k6.z=p1z+h*n1z+w1*b1z;

    //left bot point
    k7.x=p1x-h*n1x+w1*b1x;
    k7.y=p1y-h*n1y+w1*b1y;
    k7.z=p1z-h*n1z+w1*b1z;



  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, texture[6]);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_REPLACE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBegin(GL_QUADS);
    // glColor3f(0,0,1.0);
  // left trail
    glVertex3f(v0.x,v0.y,v0.z);
    glVertex3f(v1.x,v1.y,v1.z);
    glVertex3f(v5.x,v5.y,v5.z);
    glVertex3f(v4.x,v4.y,v4.z);

    glVertex3f(v1.x,v1.y,v1.z);
    glVertex3f(v2.x,v2.y,v2.z);
    glVertex3f(v6.x,v6.y,v6.z);
    glVertex3f(v5.x,v5.y,v5.z);

    glVertex3f(v2.x,v2.y,v2.z);
    glVertex3f(v3.x,v3.y,v3.z);
    glVertex3f(v7.x,v7.y,v7.z);
    glVertex3f(v6.x,v6.y,v6.z);

    glVertex3f(v3.x,v3.y,v3.z);
    glVertex3f(v7.x,v7.y,v7.z);
    glVertex3f(v4.x,v4.y,v4.z);
    glVertex3f(v0.x,v0.y,v0.z);

//right trail
    glVertex3f(k0.x,k0.y,k0.z);
    glVertex3f(k1.x,k1.y,k1.z);
    glVertex3f(k5.x,k5.y,k5.z);
    glVertex3f(k4.x,k4.y,k4.z);

    glVertex3f(k1.x,k1.y,k1.z);
    glVertex3f(k2.x,k2.y,k2.z);
    glVertex3f(k6.x,k6.y,k6.z);
    glVertex3f(k5.x,k5.y,k5.z);

    glVertex3f(k2.x,k2.y,k2.z);
    glVertex3f(k3.x,k3.y,k3.z);
    glVertex3f(k7.x,k7.y,k7.z);
    glVertex3f(k6.x,k6.y,k6.z);

    glVertex3f(k3.x,k3.y,k3.z);
    glVertex3f(k7.x,k7.y,k7.z);
    glVertex3f(k4.x,k4.y,k4.z);
    glVertex3f(k0.x,k0.y,k0.z);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    }
  }
}
}

// sky cube render 
void sky_cube(){  

  // top-sky
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture[1]);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  


  glBegin(GL_QUADS);
  glTexCoord2f(0.0,0.0); glVertex3f(-CUBE_SIZE,-CUBE_SIZE,CUBE_SIZE);
  glTexCoord2f(1.0,0.0); glVertex3f(CUBE_SIZE,-CUBE_SIZE,CUBE_SIZE);
  glTexCoord2f(1.0,1.0); glVertex3f(CUBE_SIZE,CUBE_SIZE,CUBE_SIZE);
  glTexCoord2f(0.0,1.0); glVertex3f(-CUBE_SIZE,CUBE_SIZE,CUBE_SIZE);

  
  glEnd();
  // glDisable(GL_TEXTURE_2D);



//bottom-ground
  glBindTexture(GL_TEXTURE_2D, texture[0]);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  

  glBegin(GL_QUADS);
  glTexCoord2f(1.0,0.0); glVertex3f(-CUBE_SIZE,-CUBE_SIZE,-CUBE_SIZE);
  glTexCoord2f(1.0,1.0); glVertex3f(-CUBE_SIZE,CUBE_SIZE,-CUBE_SIZE);
  glTexCoord2f(0.0,1.0); glVertex3f(CUBE_SIZE,CUBE_SIZE,-CUBE_SIZE);
  glTexCoord2f(0.0,0.0); glVertex3f(CUBE_SIZE,-CUBE_SIZE,-CUBE_SIZE);

  glEnd();
  

  //cube face1
  glBindTexture(GL_TEXTURE_2D, texture[3]);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBegin(GL_QUADS);
  glTexCoord2f(0.0,0.0); glVertex3f(-CUBE_SIZE,CUBE_SIZE,CUBE_SIZE);
  glTexCoord2f(1.0,0.0); glVertex3f(CUBE_SIZE,CUBE_SIZE,CUBE_SIZE);
  glTexCoord2f(1.0,1.0); glVertex3f(CUBE_SIZE,CUBE_SIZE,-CUBE_SIZE);
  glTexCoord2f(0.0,1.0); glVertex3f(-CUBE_SIZE,CUBE_SIZE,-CUBE_SIZE);
  glEnd();


  //cube face2
  glBindTexture(GL_TEXTURE_2D, texture[2]);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBegin(GL_QUADS);
  glTexCoord2f(0.0,0.0); glVertex3f(-CUBE_SIZE,-CUBE_SIZE,CUBE_SIZE);
  glTexCoord2f(1.0,0.0); glVertex3f(CUBE_SIZE,-CUBE_SIZE,CUBE_SIZE);
  glTexCoord2f(1.0,1.0); glVertex3f(CUBE_SIZE,-CUBE_SIZE,-CUBE_SIZE);
  glTexCoord2f(0.0,1.0); glVertex3f(-CUBE_SIZE,-CUBE_SIZE,-CUBE_SIZE);
  glEnd();

  //cube face3
  glBindTexture(GL_TEXTURE_2D, texture[5]);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBegin(GL_QUADS);
  glTexCoord2f(1.0,1.0); glVertex3f(CUBE_SIZE,CUBE_SIZE,-CUBE_SIZE);
  glTexCoord2f(0.0,1.0); glVertex3f(CUBE_SIZE,-CUBE_SIZE,-CUBE_SIZE);
  glTexCoord2f(0.0,0.0); glVertex3f(CUBE_SIZE,-CUBE_SIZE,CUBE_SIZE);
  glTexCoord2f(1.0,0.0); glVertex3f(CUBE_SIZE,CUBE_SIZE,CUBE_SIZE);
  glEnd();


//cube face4
  glBindTexture(GL_TEXTURE_2D, texture[4]);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBegin(GL_QUADS);
  glTexCoord2f(0.0,0.0); glVertex3f(-CUBE_SIZE,-CUBE_SIZE,CUBE_SIZE);
  glTexCoord2f(1.0,0.0); glVertex3f(-CUBE_SIZE,CUBE_SIZE,CUBE_SIZE);
  glTexCoord2f(1.0,1.0); glVertex3f(-CUBE_SIZE,CUBE_SIZE,-CUBE_SIZE);
  glTexCoord2f(0.0,1.0); glVertex3f(-CUBE_SIZE,-CUBE_SIZE,-CUBE_SIZE);
  glEnd();


    glDisable(GL_TEXTURE_2D);

}

void doIdle()
{
  // lookat_control_matrix=new point [4];
  lookat_control_matrix=getControlMatrix(spline_num,point_num); // load the current control matrix
  TNB_handler(lookat_u); // caculate current T,N,B 

  // caculate the current camera position and look_at position and up vector 
  float u=lookat_u;
  float x = (-0.5*u*u*u+u*u-0.5*u)*lookat_control_matrix[0].x+(1.5*u*u*u-2.5*u*u+1)*lookat_control_matrix[1].x+(-1.5*u*u*u+2*u*u+0.5*u)*lookat_control_matrix[2].x+(0.5*u*u*u-0.5*u*u)*lookat_control_matrix[3].x;
  float y = (-0.5*u*u*u+u*u-0.5*u)*lookat_control_matrix[0].y+(1.5*u*u*u-2.5*u*u+1)*lookat_control_matrix[1].y+(-1.5*u*u*u+2*u*u+0.5*u)*lookat_control_matrix[2].y+(0.5*u*u*u-0.5*u*u)*lookat_control_matrix[3].y;
  float z = (-0.5*u*u*u+u*u-0.5*u)*lookat_control_matrix[0].z+(1.5*u*u*u-2.5*u*u+1)*lookat_control_matrix[1].z+(-1.5*u*u*u+2*u*u+0.5*u)*lookat_control_matrix[2].z+(0.5*u*u*u-0.5*u*u)*lookat_control_matrix[3].z;
  eyex=x+0.03*lookat_N.x;
  eyey=y+0.03*lookat_N.y;
  eyez=z+0.03*lookat_N.z;
  centerx=x+lookat_T.x;
  centery=y+lookat_T.y;
  centerz=z+lookat_T.z;
  upx=lookat_N.x;
  upy=lookat_N.y;
  upz=lookat_N.z;


 
  // lookat_u+=0.001;
  lookat_u+=0.06; // speed setting
  if(lookat_u>=1){
    lookat_u=0;
    point_num++;
  }
  if(point_num==g_Splines[spline_num].numControlPoints-3){
    spline_num++;
    point_num=0;
  }
  if(spline_num==g_iNumOfSplines){
    spline_num=0;
    point_num=0;
    lookat_control_matrix=getControlMatrix(0,0);
    lookat_u=0;
    // exit(0);
  }

if(save){
        char resultframe[2048];
            sprintf(resultframe,"%03d.jpg",amoutofimage);
        if(amoutofimage<600){
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


void display(){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  splines();

  sky_cube();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();


  gluLookAt(eyex,eyey,eyez,centerx,centery,centerz,upx,upy,upz);


  glutSwapBuffers();

}

void reshape(int width, int height)
 
{
  // Enable perspective projection
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(60.0, (float)width / height, 0.01, 1000.0);
   glMatrixMode(GL_MODELVIEW);
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

void keyboard(unsigned char c, int x, int y){
    switch(c){

            case 's':
                save=true;
                break;
    }
}




int main (int argc, char ** argv)
{
  if (argc<2)
  {  
  printf ("usage: %s <trackfile>\n", argv[0]);
  exit(0);
  }

  loadSplines(argv[1]);


  lookat_control_matrix=new point [4];
  // lookat_control_matrix=getControlMatrix(spline_num,point_num);
 lookat_control_matrix=getControlMatrix(0,0);


  glutInit(&argc,(char**)argv);
  glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
  
  // Create a window of size 640x480
  glutInitWindowSize(640,480);
  glutInitWindowPosition(200, 100);
  glutCreateWindow("Assignment2 Roller Coaster");
  
  /* tells glut to use a particular display function to redraw */
  glutDisplayFunc(display);

  // Reshape function call
  glutReshapeFunc (reshape);

  
  /* allow the user to quit using the right mouse button menu */
  g_iMenuId = glutCreateMenu(menufunc);
  glutSetMenu(g_iMenuId);
  glutAddMenuEntry("Quit",0);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  
   /* replace with any animate code */
  glutIdleFunc(doIdle);

    
  glutKeyboardFunc(keyboard);

  
  
  /* do initialization */
  myinit();

  glutMainLoop();


  return 0;
}
