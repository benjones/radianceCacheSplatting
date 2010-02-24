#ifndef _LIGHT_H
#define _LIGHT_H

#include <GL/glew.h>
#include <istream>

class Light
{
 public:
  Light(std::istream& ins, GLenum _lightNum);
  void execute();//actually call light commands

 private:
  void parseLight(std::istream& ins);
  
  float *ambient;
  float *diffuse;
  float * specular;
  float *position;
  float * spotDirection;
  float spotExponent, spotCutoff, constantAttenuation,
    linearAttenuation, quadraticAttenuation;
  bool spotExpOn, spotCutOn, constAttenOn, linAttenOn, quadAttenOn;
  GLenum lightNum;

};

#endif
