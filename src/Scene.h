#ifndef _SCENE_H
#define _SCENE_H

//scene format
//lines starting with token and add'l parameters
//described below.  Lines include
//Rotate, Translate, Scale, PushMatrix, PopMatrix, Camera and
//Perspective transforms



#include "GLCommand.h"
#include <vector>
#include <istream>
#include <string>
#include <map>
#include "SceneObject.h"
#include <GL/glew.h>

extern unsigned windWidth;
extern unsigned windHeight;

class Scene
{
 public:
  Scene(std::istream& ins);
  ~Scene();
  void directIllumination();
  void noShadows();
 private:
  void parseScene(std::istream& ins);
  void drawObjects();
  void texMatSetup();
  void viewProjSetup(float *eye, float*eyedir);
  GLuint loadShader(std::string filename, GLenum type);
  void loadShadowShader();
  std::vector<GLCommand*> model;
  GLUPerspective* projection;//store the commands
  GLULookAt* view;
//to run
  std::vector<Light*> lights;
  std::map<std::string, SceneObject*> sceneObjects;

  size_t numLights;
  static const GLenum lightEnums[8]; 

  GLuint shadowMapTexture, FBOID, shadowProgram, shadowTexUniform;
  unsigned shadowMapSize;
  const GLenum shadowTexEnum;
};
#endif //_SCENE_H
