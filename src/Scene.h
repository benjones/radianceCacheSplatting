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

class Scene
{
 public:
  Scene(std::istream& ins);
  void directIllumination();
 private:
  void parseScene(std::istream& ins);

  std::vector<GLCommand*> model, view, projection;//store the commands
						  //to run
  std::map<std::string, SceneObject*> sceneObjects;

  size_t numLights;
  static const GLenum lights[8]; 
};
#endif //_SCENE_H
