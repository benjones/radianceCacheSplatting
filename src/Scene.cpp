#include "Scene.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Helpers.h"
#include <GL/glut.h>
#include "Light.h"

const GLenum Scene::lightEnums[8] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, 
				     GL_LIGHT3, GL_LIGHT4, GL_LIGHT5, 
				     GL_LIGHT6, GL_LIGHT7};

Scene::Scene(std::istream& ins)
  :projection(NULL), view(NULL), numLights(0), shadowMapSize(windWidth*2), 
   shadowTexEnum(GL_TEXTURE7)
{
  parseScene(ins);

  //create shadow map texture
  glGenTextures(1, &shadowMapTexture);
  glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapSize, 
	       shadowMapSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
  
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenFramebuffersEXT(1, &FBOID);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBOID);

  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, 
			    GL_TEXTURE_2D, shadowMapTexture, 0);

  GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    std::cerr << "Framebuffer complete fail" << std::endl;

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);//window buffer

  loadShadowShader();

  //  std::cout << "constructor completed" << std::endl;
}

Scene::~Scene()
{
  glDeleteTextures(1, &shadowMapTexture);
  glDeleteFramebuffersEXT(1, &FBOID);
  for(std::vector<GLCommand*>::iterator i = model.begin();
      i != model.end(); ++i)
    delete (*i);
  model.clear();
  delete projection;
  delete view;
  for(std::vector<Light*>::iterator i = lights.begin();
      i != lights.end(); ++i)
    delete (*i);
  lights.clear();
  for(std::map<std::string, SceneObject*>::iterator i = sceneObjects.begin();
      i != sceneObjects.end(); ++i)
    delete (*i).second;
  sceneObjects.clear();
}

void Scene::parseScene(std::istream& ins)
{
  std::string curLine;
  std::vector<std::string> tokens;
  while(std::getline(ins, curLine))
    {
      //strip comments
      curLine = curLine.substr(0, curLine.find_first_of('#'));
      Helpers::tokenize(curLine, tokens);
      if(!tokens.size())
	continue;
      
      //cam same format as gluLookAt
      //cam eye_x eye_y eye_z center_x center_y center_z up_x up_y up_z
      if(tokens[0] == "cam")
	{
	  if(tokens.size() != 10 || view)
	    {
	      std::cerr << "Ignoring malformed/repeated camera line: " <<
		curLine << std::endl;
	      tokens.clear();
	      continue;
	    }
	  float ix, iy, iz, cx, cy, cz, ux, uy, uz;
	  ix = Helpers::str2float(tokens[1]);
	  iy = Helpers::str2float(tokens[2]);
	  iz = Helpers::str2float(tokens[3]);
	  cx = Helpers::str2float(tokens[4]);
	  cy = Helpers::str2float(tokens[5]);
	  cz = Helpers::str2float(tokens[6]);
	  ux = Helpers::str2float(tokens[7]);
	  uy = Helpers::str2float(tokens[8]);
	  uz = Helpers::str2float(tokens[9]);
	  view = new GLULookAt(ix,iy,iz,cx,cy,cz,ux,uy,uz);
	}
      else if(tokens[0] == "rotate")
	{
	  if(tokens.size() != 5)
	    {
	      std::cerr << "Ignoring malformed rotation line: " <<
		curLine << std::endl;
	      tokens.clear();
	      continue;
	    }
	  float angle, x, y, z;
	  angle = Helpers::str2float(tokens[1]);
	  x = Helpers::str2float(tokens[2]);
	  y = Helpers::str2float(tokens[3]);
	  z = Helpers::str2float(tokens[4]);
	  model.push_back(new GLRotate(angle, x, y, z, shadowTexEnum));

	}
      else if(tokens[0] == "trans")
	{
	  if(tokens.size() != 4)
	    {
	      std::cerr << "Ignoring malformed translate line: " <<
		curLine << std::endl;
		tokens.clear();
	      continue;
	    }
	  float x,y,z;
	  x = Helpers::str2float(tokens[1]);
	  y = Helpers::str2float(tokens[2]);
	  z = Helpers::str2float(tokens[3]);
	  model.push_back(new GLTranslate(x,y,z, shadowTexEnum));
	}
      else if(tokens[0] == "scale")
	{
	  if(tokens.size() != 4)
	    {
	      std::cerr << "Ignoring malformed scale line: " << 
		curLine << std::endl;
	      tokens.clear();
	      continue;
	    }
	  float x,y,z;
	  x = Helpers::str2float(tokens[1]);
	  y = Helpers::str2float(tokens[2]);
	  z = Helpers::str2float(tokens[3]);
	  model.push_back(new GLScale(x,y,z, shadowTexEnum));
	}
      else if(tokens[0] == "push")
	{
	  if(tokens.size() != 1)
	    {
	      std::cerr << "ignoring malformed push line: " <<
		curLine << std::endl;
	      tokens.clear();
	      continue;
	    }
	  model.push_back(new GLPushMatrix(shadowTexEnum));
	}
      else if(tokens[0] == "pop")
	{
	  if(tokens.size() != 1)
	    {

	      std::cerr << "ignoring malformed pop line: " << 
		curLine << std::endl;
	      tokens.clear();
	      continue;
	    }
	  model.push_back(new GLPopMatrix(shadowTexEnum));
	}
      else if(tokens[0] == "proj")
	{
	  if(tokens.size() != 5 || projection)
	    {
	      std::cerr << "ignoring malformed/repeated projection line" <<
		curLine << std::endl;
	      tokens.clear();
	      continue;
	    }
	  float fovy, aspect, near, far;
	  fovy = Helpers::str2float(tokens[1]);
	  aspect = Helpers::str2float(tokens[2]);
	  near = Helpers::str2float(tokens[3]);
	  far = Helpers::str2float(tokens[4]);
	  projection = new GLUPerspective(fovy, aspect, near, far);
	}
      else if(tokens[0] == "obj")
	{
	  if(tokens.size() != 2)
	    {
	      std::cerr << "Ignoring malformed object line: " <<
		curLine << std::endl;
	      tokens.clear();
	      continue;
	    }
	  std::map<std::string, SceneObject*>::iterator it = 
	    sceneObjects.find(tokens[1]);
	  if(it == sceneObjects.end())
	    {
	      //read in the file and create the object
	      std::ifstream istr;
	      istr.open(tokens[1].c_str());
	      SceneObject* sop = new SceneObject(istr);
	      //std::cout << "scene object created" << std::endl;
	      //std::cin.get();
	      std::pair<std::map
		<std::string, SceneObject*>::iterator,bool> ret;
	      ret = sceneObjects.insert(std::pair<std::string, SceneObject*>
					(tokens[1], sop));
	      
	      model.push_back(new GLObject(ret.first));
	      //std::cout << "model pushed" << std::endl;
	      //std::cin.get();
	    } 
	  else
	    {
	      model.push_back(new GLObject(it));
	      //its already read, don't do anything
	    }
	  
	}
      else if(tokens[0] == "light")
	{
	  if(tokens.size() != 2)
	    {
	      std::cerr << "Ignoring malformed light line" << 
		curLine <<std::endl;
	      tokens.clear();
	      continue;
	    }
	  else 
	    {
	      std::ifstream ltStr;
	      ltStr.open(tokens[1].c_str());
	      Light * light = new Light(ltStr, lightEnums[numLights]);
	      lights.push_back(light);
	      numLights++;
	    }
	}

      else if(tokens[0] == "brdf")
	{
	  if(tokens.size() < 3)
	    {
	      std::cerr << "ignoring malformed brdf line" <<
		curLine << std::endl;
	      tokens.clear();
	      continue;
	    }
	  if(tokens[1] == "diffuse")
	    {
	      if( tokens.size() != 6)
		{
		  std::cerr << "ignoring malformed diffuse line" <<
		    curLine << std::endl;
		}
	      GLfloat r,g,b,a;
	      r = Helpers::str2float(tokens[2]);
	      g = Helpers::str2float(tokens[3]);
	      b = Helpers::str2float(tokens[4]);
	      a = Helpers::str2float(tokens[5]);
	      model.push_back(new GLBRDF(r,g,b,a));
	    }
	}
      else
	{
	  std::cerr << "ignoring malformed line" <<curLine <<std::endl;
	}

      tokens.clear();
    }
  std::cout << "Scene read" << std::endl;
  //std::cin.get();
}



void Scene::directIllumination()
{
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBOID);
  glUseProgram(0);
  glViewport(0,0,shadowMapSize, shadowMapSize);
  glClear(GL_DEPTH_BUFFER_BIT);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

  float* lightPos;
  float lightLookAt[3];
  lightPos = lights[0]->getPosition();
  lights[0]->lookAt(lightLookAt);

  std::cerr << "light pos: " << lightPos[0] << ' ' << lightPos[1] <<
    ' ' << lightPos[2] << std::endl;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, windWidth/windHeight, 1, 10000);//TODO: use spot width

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(lightPos[0], lightPos[1], lightPos[2], lightLookAt[0], 
	    lightLookAt[1], lightLookAt[2], 0, 1, 0);
  //TODO: Y is always up
       
  glCullFace(GL_FRONT);

  drawObjects();
  
  texMatSetup();//load bias and projection/

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);//render to window
  glViewport(0,0,windWidth, windHeight);

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(shadowProgram);
  glUniform1i(shadowTexUniform, 7);
  glActiveTexture(shadowTexEnum);
  glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
  
  float eye[3];
  float eyedir[3];
  
  view->getEye(eye);
  view->getCenter(eyedir);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, windWidth/windHeight, 1, 10000);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(eye[0], eye[1], eye[2], eyedir[0], eyedir[1], eyedir[2], 
	    0, 1, 0);

  glCullFace(GL_BACK);
  drawObjects();

  glutSwapBuffers();

  glIsShader(999);//debug
}

void Scene::texMatSetup()
{
  float modelView[16];
  float proj[16];
  const GLfloat biasMat[] = {.5, 0, 0, 0,
			   0,.5,0,0,
			   0,0,.5,0,
			   .5,.5,.5,1};

  glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
  glGetFloatv(GL_PROJECTION_MATRIX, proj);

  glMatrixMode(GL_TEXTURE);
  glActiveTexture(shadowTexEnum);
  glLoadIdentity();
  glLoadMatrixf(biasMat);
  glMultMatrixf(proj);
  glMultMatrixf(modelView);
  glMatrixMode(GL_MODELVIEW);


}


GLuint Scene::loadShader(char* filename, GLenum type)
{
  GLuint handle = glCreateShader(type);
  std::ifstream infile;
  infile.open(filename);
  std::ostringstream strmobj;
  strmobj << infile.rdbuf();
  std::string filestr1 = strmobj.str();
  const char* filestr = filestr1.c_str();
  glShaderSource(handle, 1, &filestr, NULL);
  
  glCompileShader(handle);

  GLint result;
  glGetShaderiv(handle, GL_COMPILE_STATUS, &result);
  if(! result)
    {
      GLint logLength;
      std::cerr << "compile failed of : " << filename << std::endl;
      glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLength);
      char * log = new char[logLength];
      GLint actualLength;
      glGetShaderInfoLog(handle, logLength, &actualLength, log);
      std::cerr << "Error: " << log << std::endl;

      GLint shaderLen;
      glGetShaderiv(handle, GL_SHADER_SOURCE_LENGTH, &shaderLen);
      std::cerr << "Shader length: " << shaderLen << std::endl;

      delete [] log;
    }
  return handle;
  
}

void Scene::loadShadowShader()
{
  GLuint vShaderHandle, fShaderHandle;
  vShaderHandle = loadShader("shadowVertexShader.c", GL_VERTEX_SHADER);
  fShaderHandle = loadShader("shadowFragmentShader.c", GL_FRAGMENT_SHADER);

  shadowProgram = glCreateProgram();
  glAttachShader(shadowProgram, vShaderHandle);
  glAttachShader(shadowProgram, fShaderHandle);
  glLinkProgram(shadowProgram);

  shadowTexUniform = glGetUniformLocation(shadowProgram, "ShadowMap");

}
/* failed shadowing with no shaders
void Scene::directIllumination()
{
  //std::cout << "cleared" << std::endl;
  //std::cin.get();

  //shadowing for each light, do shadow mapping and store in the accum
  //buffer

  float camProjMatrix[16];
  float camViewMatrix[16];
  float lightProjMatrix[16];
  float lightViewMatrix[16];

  //get camera view/projection matrices
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  projection->execute();
  glGetFloatv(GL_MODELVIEW_MATRIX, camProjMatrix);

  glLoadIdentity();
  view->execute();
  glGetFloatv(GL_MODELVIEW_MATRIX, camViewMatrix);

  glPopMatrix();

  for(size_t light = 0; light < numLights; ++light)
    {
      glMatrixMode(GL_MODELVIEW);//use modelview for all
      //get the view/projection matrices
      glPushMatrix();
      glLoadIdentity();
      float spotCutoff = lights[light]->getCutoff();

      std::cout << "spot cutoff: " << spotCutoff << "Using: " <<
	std::min((spotCutoff *2 + 10), 90.0f) << std::endl; 
      gluPerspective(std::min((spotCutoff *2 + 10), 90.0f), 1, 1, 50);

      glGetFloatv(GL_MODELVIEW_MATRIX, lightProjMatrix);
      
      glLoadIdentity();
      float * pos = lights[light]->getPosition();
      float * dir = lights[light]->getPosition();
      //TODO DON'T ALWAYS ASSUME UP IS 0 1 0
      gluLookAt(pos[0], pos[1], pos[2],
		pos[0] + dir[0], pos[1] + dir[1], pos[2] + dir[2],
		0, 1, 0); 
      glGetFloatv(GL_MODELVIEW_MATRIX, lightViewMatrix);

      glPopMatrix();
      
      //render from light's POV:
      glViewport(0,0, shadowMapSize, shadowMapSize);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(lightProjMatrix);
      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixf(lightViewMatrix);

      glCullFace(GL_FRONT);//assuming all scene objects are closed
      glShadeModel(GL_FLAT);
      glColorMask(0,0,0,0);//small performance boosts

      glDisable(GL_LIGHTING);
      glEnable(GL_DEPTH_TEST);
      //draw geometry
      glPushMatrix();
      for(std::vector<GLCommand*>::iterator i = model.begin();
	  i != model.end(); ++i)
	{
	  (*i)->execute();
	}
      glPopMatrix();
      glBindTexture(GL_TEXTURE_2D, shadowMapTexture);



      glCopyTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT,0,0, 
			  shadowMapSize, shadowMapSize,0);

v      glViewport(0, 0, windWidth, windHeight);

      glCullFace(GL_BACK);
      glShadeModel(GL_SMOOTH);
      glColorMask(1,1,1,1);
      
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      lights[light]->execute();//enable light and set its params
      glPopMatrix();
      glClear(GL_DEPTH_BUFFER_BIT);
      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(camProjMatrix);

      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixf(camViewMatrix);

      
      //set up bias matrix
      float bias[16];
      glPushMatrix();
      glLoadIdentity();
      glTranslatef(0.5,0.5,0);
      glScalef(0.5,0.5,1.0);
      glMultMatrixf(lightProjMatrix);
      glMultMatrixf(lightViewMatrix);
      glGetFloatv(GL_MODELVIEW_MATRIX, bias);
      for(int i = 0; i < 16; ++i)
	std::cout << bias[i] << '\t';
      std::cout << std::endl;
      //transpose it, since we need rows
      glLoadIdentity();
      glMultTransposeMatrixf(bias);
      glGetFloatv(GL_MODELVIEW_MATRIX, bias);
      glPopMatrix();
      for(int i = 0; i < 16; ++i)
	std::cout << bias[i] << '\t';
      std::cout << std::endl;
      //set up tex coord generation
      
      glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
      glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
      glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
      glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

      glTexGenfv(GL_S, GL_EYE_PLANE, &bias[0]);
      glTexGenfv(GL_T, GL_EYE_PLANE, &bias[4]);
      glTexGenfv(GL_R, GL_EYE_PLANE, &bias[8]);
      glTexGenfv(GL_Q, GL_EYE_PLANE, &bias[12]);

      glEnable(GL_TEXTURE_GEN_S);
      glEnable(GL_TEXTURE_GEN_T);
      glEnable(GL_TEXTURE_GEN_R);
      glEnable(GL_TEXTURE_GEN_Q);

      //bind and enable shadow map texture
      glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
      glEnable(GL_TEXTURE_2D);
      
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, 
		      GL_COMPARE_R_TO_TEXTURE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, 
		      GL_LEQUAL);
      glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, 
		      GL_LUMINANCE);
      //alpha fail fragments in shadow
      glAlphaFunc(GL_GEQUAL, .99f);
      glEnable(GL_ALPHA_TEST);

      //draw geometry again
      glPushMatrix();
      for(std::vector<GLCommand*>::iterator i = model.begin();
	  i != model.end(); ++i)
	(*i)->execute();
      glPopMatrix();

      //restore state
      glDisable(GL_ALPHA_TEST);
      glDisable(lightEnums[light]);
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_TEXTURE_GEN_S);
      glDisable(GL_TEXTURE_GEN_T);
      glDisable(GL_TEXTURE_GEN_R);
      glDisable(GL_TEXTURE_GEN_Q);


      //accumulate each light
      if(light == 0)
	{
	  //load accum buffer
	  glAccum(GL_LOAD, 1.0/numLights);
	}
      else{
	glAccum(GL_ACCUM, 1.0/numLights);
      }
      
    }
  //read out accumulator
  glAccum(GL_RETURN, 1.0);

  //glMatrixMode(GL_PROJECTION);
  //glLoadIdentity();
  BEGIN COMMENT
  for(std::vector<GLCommand*>::iterator i = projection.begin();
      i != projection.end(); ++i)
    {
      (*i)->execute();
    }
  END COMMENT
  //std::cout << "Projection finished" << std::endl;
  //std::cin.get();

  
  //std::cin.get();
  //for(std::vector<GLCommand*>::iterator i = model.begin();
  //   i != model.end(); ++i)
  // {
  //   (*i)->execute();
      
  // }

  //std::cout << "model done" << std::endl;
  //std::cin.get();
  glFlush();
  glutSwapBuffers();
  glIsShader(999);
}*/


void Scene::drawObjects()
{

  for(std::vector<GLCommand*>::iterator i = model.begin();
      i != model.end(); ++i)
    (*i)->execute();


}

void Scene::noShadows()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  projection->execute();
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  view->execute();
  for(std::vector<Light*>::iterator l = lights.begin();
      l != lights.end(); ++l)
    (*l)->execute();
  drawObjects();

  glFlush();
  glutSwapBuffers();
}
