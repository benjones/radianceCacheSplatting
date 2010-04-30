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
#include "stopwatch.h"

extern unsigned windWidth;
extern unsigned windHeight;

const float recordFOV = 150.0;
const float defaultFront = 1.0;
const float defaultBack = 30.0;
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

  void splatRecords();

 private:
  void drawAtPoint(float*point, float* direction, float*up, GLuint fbo, 
		   int width, int height, float fovy = 45.0, 
		   float front = 1.0, float back = 30.0);

  void warmupCache(int numRecs);

  void parseScene(std::istream& ins);
  void drawObjects();
  void texMatSetup(unsigned lightNum);
  void viewProjSetup(float *eye, float*eyedir, float*up, float fovy = 45.0, 
		     float aspectRatio = 1.0, float front=defaultFront, 
		     float back= defaultBack);
  GLuint loadShader(std::string filename, GLenum type);
  void loadShadowShader();
  void loadCoordNormalShader();
  void loadSplatShader();
  void loadFinalShader();
  
  std::vector<GLCommand*> model;
  GLUPerspective* projection;//store the commands
  GLULookAt* view;
//to run
  std::vector<Light*> lights;
  std::map<std::string, SceneObject*> sceneObjects;

  HStopWatch timer;

  size_t numLights;
  static const GLenum lightEnums[8]; 
  static const GLenum texUnitEnums[8];
  unsigned shadowMapSize;
  GLuint shadowMapTexture, FBOID, shadowProgram, coordNormalProgram,
    shadowTexUniform, uniformTexUnitBase, uniformNumLights, texUnitBase,
    recordFBOID, splatProgram, splatUniform, splatAttribute, splatHmdUniform,
    splatWorldPosUniform, splatWorldNormUniform, splatAUniform, 
    splatWindSizeUniform, splatBufFBOID, finalProgram, splatDiffuseUniform,
    finalDirect, finalSplat, finalWindSize;
  
  GLuint directFBOID;
  GLuint splatTex;

  GLuint directTex[2];
  GLuint recordTexBase[2];
  GLuint coordTexBase[4];

  const static size_t recordWidth = 64;
  const static size_t recordHeight = 64;

  friend class GLPopMatrix;
  friend class GLPushMatrix;
  friend class GLScale;
  friend class GLTranslate;
  friend class GLRotate;
  
  float* objectCoords;
  float* objectNormals;
  float* splatBuffer;

  float* IMap;
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
