// SimpleFountain.cpp - An example of the Particle System API with CUDA in OpenGL.
//
// Copyright 2013 by Lee Flier

#include "Particle/pAPI.h"
using namespace PAPI;

// OpenGL Graphics includes
#include <GL/glew.h>
#if defined (_WIN32)
#include <GL/wglew.h>
#endif
#if defined(__APPLE__) || defined(__MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

ParticleContext_t P;//This is a complete instance of the Particle API. All API state is stored in the context.

// A fountain（喷泉） spraying up in the middle of the screen
void ComputeParticles()
{
  // Set up the state.
  //PDCylinder (const pVec &e0, const pVec &e1, const float OuterRadius, const float InnerRadius=0.0f)注意采用的坐标轴
  P.Velocity(PDCylinder(pVec(0.0f, -0.01f, 0.25f), pVec(0.0f, -0.01f, 0.27f), 0.021f, 0.019f));
  //P.Velocity(PDCylinder(pVec(0.0f, 0.0f, 0.25f), pVec(0.0f, 0.0f, 0.27f), 0.021f, 0.019f));
  //指定要创建的粒子的初始速度矢量域。
  //PDLine (const pVec &e0, const pVec &e1)
  P.Color(PDLine(pVec(0.8f, 0.9f, 1.0f), pVec(1.0f, 1.0f, 1.0f)));//Particle colors are between light blue and white.
  //指定要创建的粒子的颜色域。For example,the PDLine(pVec(1, 0, 0), pVec(1, 1, 0)) will choose points on a line between red and yellow. 

  // Generate particles along a very small line in the nozzle(喷嘴).
  P.Source(1000, PDLine(pVec(0, 0, 0), pVec(0, 0, 0.4f)));
  //P.Source(100, PDLine(pVec(0, 0, 0.401f), pVec(0, 0, 0.405f)));
  //void  Source (const float particle_rate, const pDomain &dom) 
  //Add particles with positions in the specified domain.the average number added per unit time is particle_rate. 

  // Gravity.
  P.Gravity(pVec(0, 0, -0.01f));
  //The gravity acceleration vector is simply added to the velocity vector of each particle at each time step. 

  // Bounce particles off a disc of radius 5.
  //P.Bounce(-0.05f, 0.35f, 0, PDDisc(pVec(0, 0, 0), pVec(0, 0,1), 5, 0));
  P.Bounce(0.0f, 0.35f, 0, PDDisc(pVec(0, 0, 0), pVec(0, 0,1), 5, 0));
  //void  Bounce (float friction, const float resilience(弹性), const float cutoff, const pDomain &dom) ;PDDisc (const pVec &Center, const pVec Normal, const float OuterRadius, const float InnerRadius=0.0f)
  //Bounce particles off an object defined by a domain. 

  // Kill particles below Z=-3.
  P.Sink(false, PDPlane(pVec(0,0,-3), pVec(0,0,1)));
  //==P.Sink(true, PDPlane(pVec(0,0,-3), pVec(0,0,-1)));//PDPlane (const pVec &p0, const pVec &Normal)
  //void  Sink (const bool kill_inside, const pDomain &dom) 
  //If kill_inside is true, deletes all particles inside the given domain. If kill_inside is false, deletes all particles outside the given domain. 

  // Move particles to their new positions.
  P.Move(true, false);
  //void  Move (const bool move_velocity=true, const bool move_rotational_velocity=true) 
  //Apply the particles' velocities to their positions, and age the particles. 
}

// Draw as points using vertex arrays
// To draw as textured point sprites just call
// glEnable(GL_POINT_SPRITE_ARB) before calling this function.

void DrawGroupAsLines()
{
  int cnt = (int)P.GetGroupCount();
  if(cnt < 1) return;

  float *ptr;//the returned pointer to the particle data 
  size_t flstride, pos3Ofs, posB3Ofs, size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs, up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs;
  ;

  cnt = (int)P.GetParticlePointer(ptr, flstride, pos3Ofs, posB3Ofs,
  	size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs,
  	up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs);//Return a pointer to particle data stored in API memory.
  if(cnt < 1) return;

  glBegin(GL_LINES);
  for(int i = 0; i < cnt; i++) 
  {
  	// Warning: this depends on alpha following color in the Particle struct.
  	glColor4fv((GLfloat *)ptr + flstride*i + color3Ofs);//set the current color
  	glVertex3fv((GLfloat *)ptr + flstride*i + pos3Ofs);//specify a vertex

  	// Make a tail with the velocity vector's direction and length.
  	pVec tail = (*(pVec *)(ptr + flstride*i + pos3Ofs)) - (*(pVec *)(ptr + flstride*i + vel3Ofs));
  	glVertex3fv((GLfloat *)&tail);
  }

  glEnd();
}

void Draw()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set up the view.
  glLoadIdentity();//重置当前指定的矩阵为单位矩阵
  gluLookAt(0, -8, 3, 0, 0, 0, 0, 0, 1);//视图变换（世界坐标->观察坐标）；关于转换前的坐标系和转换后的坐标系要注意
  //glTranslatef(0,0,-10);

  // Draw the ground.
  glBegin(GL_QUADS);
  glColor3ub(105, 0, 0);
  glVertex3f(-3.5,-3.5,0);
  glColor3ub(0, 105, 0);
  glVertex3f(-3.5,3.5,0);
  glColor3ub(0, 0, 105);
  glVertex3f(3.5,3.5,0);
  glColor3ub(105, 105, 0);
  glVertex3f(3.5,-3.5,0);
  glEnd();

  // Do what the particles do.
  ComputeParticles();

  // Draw the particles.
  DrawGroupAsLines();

  glutSwapBuffers();//交换前后景缓冲区指针（开启双缓冲，防止闪屏现象）
}

void Reshape(int w, int h)
{
  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);//当前矩阵设置为投影矩阵
  glLoadIdentity();
  gluPerspective(45, w / double(h), 1, 100);
  //void glPerspective(GLdouble fov(视野范围，单位为度，该值小的话（非0），显示的物体会显得大，甚至进入物体内部，因为其在视野内占据的比例大), GLdouble aspect(视景体的宽高比), GLdouble near, GLdouble far)
  glMatrixMode(GL_MODELVIEW);//当前矩阵设置为模型视图矩阵
}

int main(int argc, char **argv)
{
  // Make a particle group 
  int particle_handle = P.GenParticleGroups(1, 100000);//Create particle groups, each with a maximum of max_particles, and returns the particle group number of the first one. 
  //int GenParticleGroups(const int p_group_count = 1, const size_t  max_particles = 0 ) 

  P.CurrentGroup(particle_handle);

  // Initialize GLUT(GLUT代表OpenGL应用工具包).
  glutInit(&argc, argv);

  // Make a normal 3D window.
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);//RGB颜色模式、窗口深度缓存、双缓存窗口
  glutInitWindowSize(800, 600);
  glutCreateWindow("粒子系统 Demo1");

  glutDisplayFunc(Draw);
  glutIdleFunc(Draw);
  glutReshapeFunc(Reshape);

  // We want depth buffering, etc.
  glEnable(GL_DEPTH_TEST);//glEnable 是直接维护流水线处理相关的状态的，glEnableClientState 维护的则是进入流水线前的状态。
  //glDepthFunc(GL_LESS);//指定深度比较函数，这里为如果输入的深度值小于参考值，则通过 

  try {
  	glutMainLoop();
  }
  catch (PError_t &Er) {
  	std::cerr << "Particle API exception: " << Er.ErrMsg << std::endl;
  	throw Er;
  }//先执行try语句，如果成功，程序结束。如果不成功执行catch语句，捕捉异常，并且输出。

  return 0;
}


