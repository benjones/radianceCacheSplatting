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

class GLLight : public GLCommand
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
  GLRotate(GLfloat _angle, GLfloat _x, GLfloat _y, GLfloat _z)
    :angle(_angle), x(_x), y(_y), z(_z)
    {}
  virtual void execute()
  {
    glRotatef(angle, x, y, z);
  }
 private:
  GLfloat angle, x, y, z;
};

class GLTranslate: public GLCommand
{
 public:
 GLTranslate(GLfloat _x, GLfloat _y, GLfloat _z)
   :x(_x), y(_y), z(_z)
  {}
  virtual void execute()
  {
    glTranslatef(x,y,z);
  }
 private:
  GLfloat x,y,z;
};

class GLScale : public GLCommand
{
 public:
  GLScale(GLfloat _x, GLfloat _y, GLfloat _z)
    :x(_x), y(_y), z(_z)
  {}
  virtual void execute()
  {
    glScalef(x,y,z);
  }
 private:
  GLfloat x,y,z;
};

class GLPushMatrix : public GLCommand
{
 public:
  virtual void execute()
  {
    glPushMatrix();
  }
};
class GLPopMatrix : public GLCommand
{
 public:
  virtual void execute()
  {
    glPopMatrix();
  }
};

#endif 
