#ifndef _GL_COMMAND_H
#define _GL_COMMAND_H

#include <GL/glew.h>
//#include <GL/glu.h>

#include <iostream>
#include <map>
#include <string>
#include "SceneObject.h"
#include "Light.h"

//way to store matrix push, pop, translate, rotate and scale commands
//from scene file


class Scene;

class GLCommand
{
 public:
  virtual void execute()=0;
};

//read in an object and draw it
class GLObject : public GLCommand
{
 public:
  GLObject(std::map<const std::string, SceneObject*>::iterator _it)
    :it(_it)
  {}
  virtual void execute()
  {
    //std::cout << "object execute" << std::endl;
    //std::cin.get();
    it->second->drawTriangles();
    //std::cout << "drew object" << std::endl;
  }
 private:
  std::map<const std::string, SceneObject*>::iterator it;
};

/*class GLLight : public GLCommand
{
 public:
  GLLight(Light* _light)
    : light(_light)
  {}
  virtual void execute()
  {
    light->execute();
  }
 private:
  Light* light;
  };*/

class GLBRDF : public GLCommand
{

 public:
  GLBRDF(GLfloat r,GLfloat g,GLfloat b,GLfloat a)
    {
      rgba[0] = r;
      rgba[1] = g;
      rgba[2] = b;
      rgba[3] = a;
    }
  virtual void execute()
  {
    glMaterialfv(GL_FRONT, GL_DIFFUSE, rgba);
  }
 private:
  GLfloat rgba[4];
};

class GLULookAt : public GLCommand
{
 public:
  GLULookAt(GLdouble _ix, GLdouble _iy, GLdouble _iz, 
	    GLdouble _cx, GLdouble _cy, GLdouble _cz, 
	    GLdouble _ux, GLdouble _uy, GLdouble _uz)
    :ix(_ix), iy(_iy), iz(_iz), cx(_cx), cy(_cy), cz(_cz), 
    ux(_ux), uy(_uy), uz(_uz)
  {}
  virtual void execute()
  {
    gluLookAt(ix, iy, iz, cx, cy, cz, ux, uy, uz);
  }
  void getEye(float eye[])
  { eye[0] = ix; eye[1] = iy; eye[2] = iz;}
  void getCenter(float cen[])
  {cen[0] = cx; cen[1] = cy; cen[2] = cz;}
  void getUp(float up[])
  {up[0] = ux; up[1] = uy; up[2] = uz;}
 private:
  GLdouble ix, iy, iz, cx, cy, cz, ux, uy, uz;
};

class GLUPerspective : public GLCommand
{
 public:
  GLUPerspective(GLdouble _fovy, GLdouble _aspect, 
		 GLdouble _near, GLdouble _far)
    :fovy(_fovy), aspect(_aspect), near(_near), far(_far)
    {}
  virtual void execute()
  {
    gluPerspective(fovy, aspect, near, far);
  }
 private:
  GLdouble fovy, aspect, near, far;
};


class GLRotate: public GLCommand
{
 public:
 GLRotate(GLfloat _angle, GLfloat _x, GLfloat _y, GLfloat _z, Scene* _s)
   :angle(_angle), x(_x), y(_y), z(_z), s(_s)
    {}
  virtual void execute()
  {
    glRotatef(angle, x, y, z);
    for(unsigned light = 0; light < s->numLights; ++light)
      {
	glMatrixMode(GL_TEXTURE);
	glActiveTexture(Scene::texUnitEnums[light]);
	glRotatef(angle, x, y, z);
      }
    glMatrixMode(GL_MODELVIEW);
  }
 private:
  GLfloat angle, x, y, z;
  Scene* s;
};

class GLTranslate: public GLCommand
{
 public:
 GLTranslate(GLfloat _x, GLfloat _y, GLfloat _z, Scene *_s)
   :x(_x), y(_y), z(_z), s(_s)
  {}
  virtual void execute()
  {
    glTranslatef(x,y,z);
    for(unsigned light = 0; light < s->numLights; ++light)
      {
	glMatrixMode(GL_TEXTURE);
	glActiveTexture(Scene::texUnitEnums[light]);
	glTranslatef(x,y,z);
      }
	glMatrixMode(GL_MODELVIEW);
  }
 private:
  GLfloat x,y,z;
  Scene* s;
};

class GLScale : public GLCommand
{
 public:
 GLScale(GLfloat _x, GLfloat _y, GLfloat _z, Scene* _s)
   :x(_x), y(_y), z(_z), s(_s)
  {}
  virtual void execute()
  {
    glScalef(x,y,z);
    for(unsigned light = 0; light < s->numLights; ++light)
      {
	glMatrixMode(GL_TEXTURE);
	glActiveTexture(Scene::texUnitEnums[light]);
	glScalef(x,y,z);
      }
    glMatrixMode(GL_MODELVIEW);
  }
 private:
  GLfloat x,y,z;
  Scene* s;
};

class GLPushMatrix : public GLCommand
{
 public:
  GLPushMatrix(Scene* _s)
    :s(_s) {}
  virtual void execute()
  {
    glPushMatrix();
    
    
    for(unsigned light = 0; light < s->numLights; ++light)
      {
	glActiveTexture(Scene::texUnitEnums[light]);
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
      }
    glMatrixMode(GL_MODELVIEW);
  }
 private:
  Scene* s;
};
class GLPopMatrix : public GLCommand
{
 public:
  GLPopMatrix(Scene* _s)
    :s(_s) {}
  virtual void execute()
  {
    glPopMatrix();
    for(unsigned light = 0; light < s->numLights; ++light)
      {
	glActiveTexture(Scene::texUnitEnums[light]);
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
      }
    glMatrixMode(GL_MODELVIEW);
  }
 private:
  Scene* s;
};


#endif 
