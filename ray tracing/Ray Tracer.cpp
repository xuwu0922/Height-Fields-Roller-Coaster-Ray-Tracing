 // Ray Tracer
// By Xu Wu


#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>
#include <string.h>
#include <string>
using namespace std;
#include <float.h>
#include <math.h>

// bool save=false; //save function is initially closed
// int amoutofimage=0;

#define MAX_TRIANGLES 2000
#define MAX_SPHERES 10
#define MAX_LIGHTS 10

char *filename=0;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2
int mode=MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480

//the field of view of the camera

#define fov 60.0
// #define fov 90.0
#define EPS 10e-8
#define PI 3.14159265

//max height and width of the screen
double Max_Height=tan((double) PI*fov/(2.0*180));
double Max_Width=Max_Height*((double)WIDTH)/((double)HEIGHT);

unsigned char buffer[HEIGHT][WIDTH][3];

struct Vertex
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double normal[3];
  double shininess;
};

typedef struct _Triangle
{
  struct Vertex v[3];
} Triangle;

typedef struct _Sphere
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double shininess;
  double radius;
} Sphere;

typedef struct _Light
{
  double position[3];
  double color[3];
} Light;

Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles=0;
int num_spheres=0;
int num_lights=0;

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);

//point struct, not only store point, also some objects with 3d parameters
struct point {
  double x;
  double y;
  double z;
};


point camera_position;
// camera_position.x=0.0;
// camera_position.y=0.0;
// camera_position.z=0.0;

//vector struct, reprenting vector with two points in space
struct vector {
  double x;
  double y;
  double z;
};

//ray struct
struct ray {
  point src; // source of the ray
  vector dir;// direction of the ray
};



//two points minus, get the vector
vector pointToVector(point s,point t){
  vector result;
  result.x=s.x-t.x;
  result.y=s.y-t.y;
  result.z=s.z-t.z;
  return result;
}

//convert a point to vector(no meaningful,just for usage)
vector pointConvert(point p){
  vector result;
  result.x=p.x;
  result.y=p.y;
  result.z=p.z;
  return result;
}

//convert a vertex.position to point object
point getVertexPos(Vertex v){
	point result;
	result.x= v.position[0];
	result.y= v.position[1];
	result.z= v.position[2];
	return result;
}

// normalize
vector getNormalize(vector s){
  double mag = sqrt(pow(s.x,2)+pow(s.y,2)+pow(s.z,2));
  s.x=s.x/mag;
  s.y=s.y/mag;
  s.z=s.z/mag;
  return s;
}

// distance of two points
double pointDistance(point s,point t){
  double distance = sqrt(pow(s.x-t.x,2)+pow(s.y-t.y,2)+pow(s.z-t.z,2));
  return distance;
}

// dot product
double vectorDotProduct(vector s,vector t){
  double dot_product=s.x*t.x+s.y*t.y+s.z*t.z;
  return dot_product;
}

// cross product
vector vectorCrossProduct(vector s,vector t){
  vector cross_product;
  cross_product.x=s.y*t.z-s.z*t.y;
  cross_product.y=s.z*t.x-s.x*t.z;
  cross_product.z=s.x*t.y-s.y*t.x;
  return cross_product;
}

// caculate reflected vector
vector getReflectedVector(vector l, vector n){
  vector r;
  double l_dot_n=vectorDotProduct(l,n);
  r.x=2*l_dot_n*n.x-l.x;
  r.y=2*l_dot_n*n.y-l.y;
  r.z=2*l_dot_n*n.z-l.z;
  return r;
}

// given ray and distance, caculate the destination point
point getP(ray ra,double t){
  point p;
  p.x=ra.src.x+ra.dir.x*t;
  p.y=ra.src.y+ra.dir.y*t;
  p.z=ra.src.z+ra.dir.z*t;
  return p;
}

// point* getTrianglePoints(Triangle tri){
//   point* triangle_points;
//   triangle_points=new point [3];
//   for(int i=0;i<3;i++){
//     triangle_points[i].x=tri.v[i].position[0];
//     triangle_points[i].y=tri.v[i].position[1];
//     triangle_points[i].z=tri.v[i].position[2];
//   }
//   return triangle_points;

// }

// triangle intersection and check if intersect the inner of triangle
double triangleIntersect_and_CheckInOut(ray Ray,Triangle triangle,point &baryCoord){
	point A, B, C;
	vector AB, AC, n;
	double d=0;
	double t,a1,a2,a3,a;


	A=getVertexPos(triangle.v[0]);
	B=getVertexPos(triangle.v[1]);
	C=getVertexPos(triangle.v[2]);

  	AB=pointToVector(B,A);
  	AC=pointToVector(C,A);
  	n=vectorCrossProduct(AB,AC);
  	n=getNormalize(n);

  	if(vectorDotProduct(Ray.dir,n)==0){
  		return -1.0;
  	}
  	else{
  		// d from R(t)=p0+dt
  		d=vectorDotProduct(n,pointConvert(A));
  		t=(d-vectorDotProduct(n,pointConvert(Ray.src)))/vectorDotProduct(n,Ray.dir);
  		if(t<EPS){
  			return -1.0;
  		}
  	}

    //binary coordinate caculation
  	point Q= getP(Ray,t);
  	a1=vectorDotProduct(n,vectorCrossProduct(pointToVector(B,A),pointToVector(Q,A)));
  	a2=vectorDotProduct(n,vectorCrossProduct(pointToVector(C,B),pointToVector(Q,B)));
  	a3=vectorDotProduct(n,vectorCrossProduct(pointToVector(A,C),pointToVector(Q,C)));

  	if(a1>=0&&a2>=0&&a3>=0){
      a=vectorDotProduct(n,vectorCrossProduct(pointToVector(B,A),pointToVector(C,A)));
      baryCoord.x=a2/a;
      baryCoord.y=a3/a;
      baryCoord.z=a1/a;
      return t;
	  }
	  return -1.0;

}

//sphere intersection
double sphereIntersect(ray Ray,Sphere sphere, point &q){
	 double b,c,t1,t2,delta,t;

   c=pow((Ray.src.x-sphere.position[0]),2)
              +pow((Ray.src.y-sphere.position[1]),2)
              +pow((Ray.src.z-sphere.position[2]),2)
              -pow(sphere.radius,2);


	 b=2.0*(Ray.dir.x*(Ray.src.x-sphere.position[0])
              +Ray.dir.y*(Ray.src.y-sphere.position[1])
              +Ray.dir.z*(Ray.src.z-sphere.position[2]));


    delta=pow(b,2.0)-4.0*c;
    if(delta>=0.0){
      t1=(sqrt(delta)-b)/2;
      t2=(-sqrt(delta)-b)/2;
      if(t1<=0&&t2<=0){
        return -1.0;
      }
      if(t1>0&&t2>0){
        t=min(t1,t2);
        if(t<EPS){
          return -1.0;
        }
      }
      else{
        if(t1>t2){
          t=t1;
        }
        if(t2>t1){
          t=t2;
        }
        if(t<EPS){
          return -1.0; 
        }
      }
    	q=getP(Ray,t); // get the intersection point
    	return t;
    }
    else{
      return -1.0;
    }
}

// caculate the sphere normal
vector getSphereNormal(point q,Sphere sphere){
    vector n;
    n.x=(q.x-sphere.position[0])/sphere.radius;
    n.y=(q.y-sphere.position[1])/sphere.radius;
    n.z=(q.z-sphere.position[2])/sphere.radius;
    n=getNormalize(n);
    return n;
}


// check if the ray directs to the light
double checkDirectToLight(ray Ray,Light light){
	double d,a,b,c;
  	point light_position;
  	light_position.x=light.position[0];
  	light_position.y=light.position[1];
  	light_position.z=light.position[2];
  	d=pointDistance(Ray.src,light_position);
 	if(d<EPS){
    	return -1.0;
  	}
  	a=(light_position.x-Ray.src.x)/Ray.dir.x;
  	b=(light_position.y-Ray.src.y)/Ray.dir.y;
  	c=(light_position.z-Ray.src.z)/Ray.dir.z;
    if(a==d&&b==d&&c==d){
      return d;
    }
    return -1.0;
}


// sphere phong shading
void caculateSphereShading(vector *color_params,double sh, point intersect_point, vector N, vector &phongshading, vector V){
	for(int i=0;i<num_lights;i++){

    // start to shot each shadow rays
		  ray shotray;
		  bool shadowblocked=false;
		  shotray.src=intersect_point;
		  vector direction;
		  direction.x=lights[i].position[0]-intersect_point.x;
    	direction.y=lights[i].position[1]-intersect_point.y;
    	direction.z=lights[i].position[2]-intersect_point.z;
    	direction=getNormalize(direction);
    	shotray.dir=direction;

    	point light_position;
    	light_position.x=lights[i].position[0];
    	light_position.y=lights[i].position[1];
    	light_position.z=lights[i].position[2];
    	double d_light=pointDistance(shotray.src,light_position);

      //check if shadow blocked
    	for(int j=0;j<num_spheres;j++){
      		double d_sphere;
      		vector sphere_normal;
          point q;
      		d_sphere=sphereIntersect(shotray,spheres[j],q);
      		if(d_sphere!=-1.0){
        		if(d_sphere-d_light<EPS){
          			shadowblocked=true;
        		}
      		}
    	}

      // if not blocked, caculate the phong shading
    	if(!shadowblocked){
    		double L_dot_N=vectorDotProduct(direction,N);
     		vector R=getNormalize(getReflectedVector(direction,N));
      		double R_dot_V=vectorDotProduct(R,V);
      		if(L_dot_N<0){
        		L_dot_N=0.0;
      		}
      		if(R_dot_V<0){
        		R_dot_V=0.0;
      		}
      		vector kd,ks;
      		kd=color_params[0];
      		ks=color_params[1];
      		phongshading.x += lights[i].color[0] * (kd.x * L_dot_N + ks.x * pow(R_dot_V, sh));
      		phongshading.y += lights[i].color[1] * (kd.y * L_dot_N + ks.y * pow(R_dot_V, sh));
      		phongshading.z += lights[i].color[2] * (kd.z * L_dot_N + ks.z * pow(R_dot_V, sh));
    	}

	}
}

// triangle phong shading
void caculateTriangleShading(vector *color_params,double sh, point intersect_point, vector N, vector &phongshading, vector V){
	for(int i=0;i<num_lights;i++){

    // start to shot each shadow rays
		ray shotray;
		bool shadowblocked=false;
		shotray.src=intersect_point;
		vector direction;
		direction.x=lights[i].position[0]-intersect_point.x;
    	direction.y=lights[i].position[1]-intersect_point.y;
    	direction.z=lights[i].position[2]-intersect_point.z;
    	direction=getNormalize(direction);
    	shotray.dir=direction;

    	point light_position;
    	light_position.x=lights[i].position[0];
    	light_position.y=lights[i].position[1];
    	light_position.z=lights[i].position[2];
    	double d_light=pointDistance(shotray.src,light_position);

      //check if shadow blocked
    	for(int j=0;j<num_triangles;j++){
      		double d_triangle;
      		point baryCoord_triangle;
      		d_triangle=triangleIntersect_and_CheckInOut(shotray,triangles[j], baryCoord_triangle);
      		if(d_triangle!=-1.0){
        		if(d_triangle-d_light<EPS){
          			shadowblocked=true;
        		}
      		}
    	}

      // if not blocked, caculate the phong shading
    	if(!shadowblocked){
    		double L_dot_N=vectorDotProduct(direction,N);
     		vector R=getNormalize(getReflectedVector(direction,N));
      		double R_dot_V=vectorDotProduct(R,V);
      		if(L_dot_N<0){
        		L_dot_N=0.0;
      		}
      		if(R_dot_V<0){
        		R_dot_V=0.0;
      		}
      		vector kd,ks;
      		kd=color_params[0];
      		ks=color_params[1];
      		phongshading.x += lights[i].color[0] * (kd.x * L_dot_N + ks.x * pow(R_dot_V, sh));
      		phongshading.y += lights[i].color[1] * (kd.y * L_dot_N + ks.y * pow(R_dot_V, sh));
      		phongshading.z += lights[i].color[2] * (kd.z * L_dot_N + ks.z * pow(R_dot_V, sh));
    	}

	}
}

// ray tracing process
void RayTracing(ray ra, vector &color) {
  double distance_mark=DBL_MAX;
  bool intersected = false;
// start to check if shot to light directly
  for(int i=0;i<num_lights;i++){
    double distance_to_light;
    distance_to_light=checkDirectToLight(ra,lights[i]);
    if(distance_to_light!=-1.0){
      if(distance_to_light<distance_mark){
        color.x=lights[i].color[0]*255.0;
        color.y=lights[i].color[1]*255.0;
        color.z=lights[i].color[2]*255.0;
        intersected=true;
        distance_mark=distance_to_light;
      }
    }
  }
  
//check if inserct some sphere, track the nearest one
  for(int j=0;j<num_spheres;j++){
    double distance_to_sphere;
    vector sphere_normal;
    point intersect_point;
    distance_to_sphere=sphereIntersect(ra,spheres[j],intersect_point);
    if(distance_to_sphere!=-1.0){
      if(distance_to_sphere<distance_mark){

        //start caculate the illumination 
        vector phongshading;
        phongshading.x=0.0;
        phongshading.y=0.0;
        phongshading.z=0.0;
        vector* color_params;
        color_params=new vector [2];
        color_params[0].x=spheres[j].color_diffuse[0];
        color_params[0].y=spheres[j].color_diffuse[1];
        color_params[0].z=spheres[j].color_diffuse[2];
        color_params[1].x=spheres[j].color_specular[0];
        color_params[1].y=spheres[j].color_specular[1];
        color_params[1].z=spheres[j].color_specular[2];
        double sh=spheres[j].shininess;
        vector V;
        V.x=-ra.dir.x;
        V.y=-ra.dir.y;
        V.z=-ra.dir.z;
        V=getNormalize(V);
        sphere_normal=getSphereNormal(intersect_point,spheres[j]);
        caculateSphereShading(color_params,sh,intersect_point,sphere_normal,phongshading,V);
        distance_mark=distance_to_sphere;
        intersected=true;
        color.x=phongshading.x*255.0;
        color.y=phongshading.y*255.0;
        color.z=phongshading.z*255.0;

        }


      }
    }

//check if inserct some triangle, track the nearest one
    for(int k=0;k<num_triangles;k++){
      double distance_to_triangle;
      vector triangle_normal;
      point baryCoord;
      distance_to_triangle=triangleIntersect_and_CheckInOut(ra, triangles[k],baryCoord);
      if(distance_to_triangle!=-1.0){
        if(distance_to_triangle<distance_mark){ 

          //start caculate the illumination          
          vector phongshading;
          phongshading.x=0.0;
          phongshading.y=0.0;
          phongshading.z=0.0;

          vector* color_params;
          color_params=new vector [2];   

          // interpolate diffuse, specular and shininess coefficients
          color_params[0].x= triangles[k].v[0].color_diffuse[0] * baryCoord.x + 
                          triangles[k].v[1].color_diffuse[0] * baryCoord.y + 
                          triangles[k].v[2].color_diffuse[0] * baryCoord.z;
          color_params[0].y= triangles[k].v[0].color_diffuse[1] * baryCoord.x + 
                          triangles[k].v[1].color_diffuse[1] * baryCoord.y + 
                          triangles[k].v[2].color_diffuse[1] * baryCoord.z;
          color_params[0].z= triangles[k].v[0].color_diffuse[2] * baryCoord.x + 
                          triangles[k].v[1].color_diffuse[2] * baryCoord.y + 
                          triangles[k].v[2].color_diffuse[2] * baryCoord.z;

          color_params[1].x= triangles[k].v[0].color_specular[0] * baryCoord.x + 
                          triangles[k].v[1].color_specular[0] * baryCoord.y + 
                          triangles[k].v[2].color_specular[0] * baryCoord.z;
          color_params[1].y= triangles[k].v[0].color_specular[1] * baryCoord.x + 
                          triangles[k].v[1].color_specular[1] * baryCoord.y + 
                          triangles[k].v[2].color_specular[1] * baryCoord.z;
          color_params[1].z= triangles[k].v[0].color_specular[2] * baryCoord.x + 
                          triangles[k].v[1].color_specular[2] * baryCoord.y + 
                          triangles[k].v[2].color_specular[2] * baryCoord.z;

          double sh = triangles[k].v[0].shininess * baryCoord.x + 
                    triangles[k].v[0].shininess * baryCoord.y + 
                    triangles[k].v[0].shininess * baryCoord.z;

          vector V;
          V.x=-ra.dir.x;
          V.y=-ra.dir.y;
          V.z=-ra.dir.z;
          V=getNormalize(V);
          point intersect_point=getP(ra,distance_to_triangle);

          // interpolate the normal
          triangle_normal.x = triangles[k].v[0].normal[0] * baryCoord.x + 
                    triangles[k].v[1].normal[0] * baryCoord.y + 
                    triangles[k].v[2].normal[0] * baryCoord.z;
          triangle_normal.y = triangles[k].v[0].normal[1] * baryCoord.x + 
                    triangles[k].v[1].normal[1] * baryCoord.y + 
                    triangles[k].v[2].normal[1] * baryCoord.z;
          triangle_normal.z = triangles[k].v[0].normal[2] * baryCoord.x + 
                    triangles[k].v[1].normal[2] * baryCoord.y + 
                    triangles[k].v[2].normal[2] * baryCoord.z;
          caculateTriangleShading(color_params,sh,intersect_point,triangle_normal,phongshading,V);
          distance_mark=distance_to_triangle;
          intersected=true;
          color.x=phongshading.x*255.0;
          color.y=phongshading.y*255.0;
          color.z=phongshading.z*255.0;
        }
   
      }
    }

    if(intersected){
      //add the global ambient color once
      color.x += ambient_light[0] * 255;
      color.y += ambient_light[1] * 255;
      color.z += ambient_light[2] * 255;
    }
    else{
      color.x=255.0;
      color.y=255.0;
      color.z=255.0;
    }
    if(color.x>255.0){
      color.x=255.0;
    }
    if(color.y>255.0){
      color.y=255.0;
    }
    if(color.z>255.0){
      color.z=255.0;
    }
    if(color.x<0.0){
      color.x=0.0;
    }
    if(color.y<0.0){
      color.y=0.0;
    }
    if(color.z<0.0){
      color.z=0.0;
    }
  }

// set screen coordinate to world coordinate
point setToWorldCoord(int x, int y){
  point p;
  p.x=-Max_Width+(((double)x/(double)WIDTH)*Max_Width*2.0);
  p.y=-Max_Height+(((double)y/(double)HEIGHT)*Max_Height*2.0);
  p.z=-1.0; 
  return p;
}

// get color for a pixel point
vector getFinalColor(point current){
  ray ra;
  camera_position.x=0.0;
  camera_position.y=0.0;
  camera_position.z=0.0;
  ra.src=camera_position;
  ra.dir=getNormalize(pointConvert(current));
  vector final_color;
  final_color.x=0.0;
  final_color.y=0.0;
  final_color.z=0.0;
  RayTracing(ra,final_color);
  return final_color;
}

//MODIFY THIS FUNCTION
void draw_scene()
{
  unsigned int x,y;
  //simple output
  for(x=0; x<WIDTH; x++)
  {
    glPointSize(2.0);  
    glBegin(GL_POINTS);
    for(y=0;y < HEIGHT;y++)
    {
      point current;
      current=setToWorldCoord(x,y);
      vector final_color=getFinalColor(current);

      plot_pixel(x,y,final_color.x,final_color.y,final_color.z);
    }
    glEnd();
    glFlush();
  }
  printf("Done!\n"); fflush(stdout);
}

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b)
{
  glColor3f(((double)r)/256.f,((double)g)/256.f,((double)b)/256.f);
  glVertex2i(x,y);
}

void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b)
{
  buffer[HEIGHT-y-1][x][0]=r;
  buffer[HEIGHT-y-1][x][1]=g;
  buffer[HEIGHT-y-1][x][2]=b;
}

void plot_pixel(int x,int y,unsigned char r,unsigned char g, unsigned char b)
{
  plot_pixel_display(x,y,r,g,b);
  if(mode == MODE_JPEG)
      plot_pixel_jpeg(x,y,r,g,b);
}

void save_jpg()
{
  Pic *in = NULL;

  in = pic_alloc(640, 480, 3, NULL);
  printf("Saving JPEG file: %s\n", filename);

  memcpy(in->pix,buffer,3*WIDTH*HEIGHT);
  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);      

}

void parse_check(char *expected,char *found)
{
  if(strcasecmp(expected,found))
    {
      char error[100];
      printf("Expected '%s ' found '%s '\n",expected,found);
      printf("Parse error, abnormal abortion\n");
      exit(0);
    }
}

void parse_doubles(FILE*file, char *check, double p[3])
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check(check,str);
  fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
  printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE*file,double *r)
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check("rad:",str);
  fscanf(file,"%lf",r);
  printf("rad: %f\n",*r);
}

void parse_shi(FILE*file,double *shi)
{
  char s[100];
  fscanf(file,"%s",s);
  parse_check("shi:",s);
  fscanf(file,"%lf",shi);
  printf("shi: %f\n",*shi);
}

int loadScene(char *argv)
{
  FILE *file = fopen(argv,"r");
  int number_of_objects;
  char type[50];
  int i;
  Triangle t;
  Sphere s;
  Light l;
  fscanf(file,"%i",&number_of_objects);

  printf("number of objects: %i\n",number_of_objects);
  char str[200];

  parse_doubles(file,"amb:",ambient_light);

  for(i=0;i < number_of_objects;i++)
    {
      fscanf(file,"%s\n",type);
      printf("%s\n",type);
      if(strcasecmp(type,"triangle")==0)
  {

    printf("found triangle\n");
    int j;

    for(j=0;j < 3;j++)
      {
        parse_doubles(file,"pos:",t.v[j].position);
        parse_doubles(file,"nor:",t.v[j].normal);
        parse_doubles(file,"dif:",t.v[j].color_diffuse);
        parse_doubles(file,"spe:",t.v[j].color_specular);
        parse_shi(file,&t.v[j].shininess);
      }

    if(num_triangles == MAX_TRIANGLES)
      {
        printf("too many triangles, you should increase MAX_TRIANGLES!\n");
        exit(0);
      }
    triangles[num_triangles++] = t;
  }
      else if(strcasecmp(type,"sphere")==0)
  {
    printf("found sphere\n");

    parse_doubles(file,"pos:",s.position);
    parse_rad(file,&s.radius);
    parse_doubles(file,"dif:",s.color_diffuse);
    parse_doubles(file,"spe:",s.color_specular);
    parse_shi(file,&s.shininess);

    if(num_spheres == MAX_SPHERES)
      {
        printf("too many spheres, you should increase MAX_SPHERES!\n");
        exit(0);
      }
    spheres[num_spheres++] = s;
  }
      else if(strcasecmp(type,"light")==0)
  {
    printf("found light\n");
    parse_doubles(file,"pos:",l.position);
    parse_doubles(file,"col:",l.color);

    if(num_lights == MAX_LIGHTS)
      {
        printf("too many lights, you should increase MAX_LIGHTS!\n");
        exit(0);
      }
    lights[num_lights++] = l;
  }
      else
  {
    printf("unknown type in scene description:\n%s\n",type);
    exit(0);
  }
    }
  return 0;
}

void display()
{

}


void init()
{
  glMatrixMode(GL_PROJECTION);
  glOrtho(0,WIDTH,0,HEIGHT,1,-1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);

}

void idle()
{
  //hack to make it only draw once
  static int once=0;
  if(!once)
  {
    draw_scene();
    if(mode == MODE_JPEG)
    save_jpg();
  }
  once=1;

}

int main (int argc, char ** argv)
{
  if (argc<2 || argc > 3)
  {  
    printf ("usage: %s <scenefile> [jpegname]\n", argv[0]);
    exit(0);
  }
  if(argc == 3)
    {
      mode = MODE_JPEG;
      filename = argv[2];
    }
  else if(argc == 2)
    mode = MODE_DISPLAY;

  glutInit(&argc,argv);
  loadScene(argv[1]);

  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(WIDTH,HEIGHT);
  int window = glutCreateWindow("Ray Tracer");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  // glutKeyboardFunc(keyboard);
  init();
  glutMainLoop();
}


