#ifndef _SCENE_H
#define _SCENE_H

//scene format
//lines starting with token and add'l parameters
//described below.  Lines include
//Rotate, Translate, Scale, PushMatrix, PopMatrix, Camera and
//Perspective transforms




#include <vector>
#include <istream>
#include <string>
#include <map>
#include "SceneObject.h"
#include <GL/glew.h>
#include "Light.h"

extern unsigned windWidth;
extern unsigned windHeight;

class GLCommand;
class GLUPerspective;
class GLULookAt;

class Scene
{
 public:
  Scene(std::istream& ins);
  ~Scene();
  void display();

  void directIllumination();
  void noShadows();

  void readCoordNormals();
  void generateRecord(float* pos, float* normal);

 private:
  void drawAtPoint(float*point, float* direction, float*up, GLuint fbo, 
		   int width, int height);

  void warmupCache(int numRecs);

  void parseScene(std::istream& ins);
  void drawObjects();
  void texMatSetup(unsigned lightNum);
  void viewProjSetup(float *eye, float*eyedir, float*up, float fovy = 45.0, float aspectRatio = 1.0);
  GLuint loadShader(std::string filename, GLenum type);
  void loadShadowShader();
  void loadCoordNormalShader();
  std::vector<GLCommand*> model;
  GLUPerspective* projection;//store the commands
  GLULookAt* view;
//to run
  std::vector<Light*> lights;
  std::map<std::string, SceneObject*> sceneObjects;

  size_t numLights;
  static const GLenum lightEnums[8]; 
  static const GLenum texUnitEnums[8];
  unsigned shadowMapSize;
  GLuint shadowMapTexture, FBOID, shadowProgram, coordNormalProgram,
    shadowTexUniform, uniformTexUnitBase, uniformNumLights, texUnitBase,
    recordFBOID;
  
  GLuint recordTexBase[2];
  GLuint coordTexBase[3];

  const static size_t recordWidth = 512;
  const static size_t recordHeight = 512;

  friend class GLPopMatrix;
  friend class GLPushMatrix;
  friend class GLScale;
  friend class GLTranslate;
  friend class GLRotate;
  
  float* objectCoords;
  float* objectNormals;

  struct irradianceRecord {
    float pos[3];
    float norm[3];
    float transGrad[3];
    float rotGrad[3];
    float irradiance[3];
    float hmd;
  };

  std::vector<irradianceRecord> records;

};

#include "GLCommand.h"

#endif //_SCENE_H
