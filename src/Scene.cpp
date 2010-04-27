#include "Scene.h"

#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Helpers.h"
#include <GL/glut.h>
#include "Light.h"
#include "GLCommand.h"

const float closePlane = 2.0;

const GLenum Scene::lightEnums[8] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, 
				     GL_LIGHT3, GL_LIGHT4, GL_LIGHT5, 
				     GL_LIGHT6, GL_LIGHT7};

const GLenum Scene::texUnitEnums[8] = {GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2,
				       GL_TEXTURE3, GL_TEXTURE4, 
				       GL_TEXTURE5, GL_TEXTURE6,
				       GL_TEXTURE7};


/*const GLenum Scene::texUnitEnums[8] = {GL_TEXTURE7, GL_TEXTURE8, GL_TEXTURE9,
				       GL_TEXTURE10, GL_TEXTURE11, 
				       GL_TEXTURE12, GL_TEXTURE13,
				       GL_TEXTURE14};*/



Scene::Scene(std::istream& ins)
  :projection(NULL), view(NULL), numLights(0), shadowMapSize(windWidth*2), 
   texUnitBase(0)
{
  parseScene(ins);

  //create shadow map texture
  glGenTextures(1, &shadowMapTexture);
  glGenFramebuffersEXT(1, &FBOID);

  glGenFramebuffersEXT(1, &recordFBOID);
  glGenTextures(2, recordTexBase);

  glActiveTexture(texUnitEnums[0]);
  glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMapTexture);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, 
		  GL_COMPARE_R_TO_TEXTURE);
  std::cout << "numlights: " << numLights << std::endl;
  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, shadowMapSize, 
	       shadowMapSize, numLights, 0, GL_DEPTH_COMPONENT, 
	       GL_UNSIGNED_BYTE,NULL);
  
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBOID);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  

  //set up "color" attachment, alpha channel with be depth
  glBindTexture(GL_TEXTURE_2D, recordTexBase[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, recordWidth, recordHeight,
	       0, GL_RGBA, GL_FLOAT, NULL);
  
  //set up depth attachment
  glBindTexture(GL_TEXTURE_2D, recordTexBase[1]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, recordWidth, 
	       recordHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);


  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);//window buffer

  loadShadowShader();

  //  std::cout << "constructor completed" << std::endl;
}

Scene::~Scene()
{
  glDeleteTextures(1, &shadowMapTexture);
  glDeleteFramebuffersEXT(1, &FBOID);
  glDeleteTextures(3, coordTexBase);
  glDeleteFramebuffersEXT(1, &recordFBOID);
  glDeleteTextures(2, recordTexBase);
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

  delete [] objectCoords;
  delete [] objectNormals;
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
	  model.push_back(new GLRotate(angle, x, y, z, this));

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
	  model.push_back(new GLTranslate(x,y,z, this));
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
	  model.push_back(new GLScale(x,y,z, this));
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
	  model.push_back(new GLPushMatrix(this));
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
	  model.push_back(new GLPopMatrix(this));
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

  float eye[3];
  float eyedir[3];
  float up[3] = {0.0, 1.0, 0.0};
  view->getEye(eye);
  view->getCenter(eyedir);

  drawAtPoint(eye, eyedir, up, 0, windWidth, windHeight);

  glutSwapBuffers();

  Helpers::getGLErrors("End of directIllumination");

  glIsShader(999);//debug
}

void Scene::generateRecord(float *pos, float* normal)
{
  float eps = .00001;
  float up[3];
  if((fabs(normal[0]) <= eps) && (fabs(normal[1] -1) <= eps) &&
     (fabs(normal[2]) <= eps))
    {
      up[0] = 1;
      up[1] = 0;
      up[2] = 0;
    }
  else
    {
      up[0] = 0;
      up[1] = 1;
      up[2] = 0;
    }
  
  float lookat[3];
  lookat[0] = pos[0] + normal[0];
  lookat[1] = pos[1] + normal[1];
  lookat[2] = pos[2] + normal[2];

  //bind my fbo
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, recordFBOID);
  
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			    GL_TEXTURE_2D, recordTexBase[0], 0);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT,
			    GL_TEXTURE_2D, recordTexBase[1], 0);

  drawAtPoint(pos, lookat, up, recordFBOID, recordWidth, recordHeight);

  glIsShader(999);


  glBindTexture(GL_TEXTURE_2D, recordTexBase[0]);
  float * IMap = new float[4*recordWidth*recordHeight];
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, IMap);

  float r = 0.0;
  float g = 0.0;
  float b = 0.0;

  //WARNING TODO FIXME 45.0 ISNT ALWAYS THE ANGLE!!!!
  float planeDim = 2*closePlane/cos(45.0*M_PI/360.0);//width/height of
						     //the plane

  float dA = planeDim*planeDim/(recordWidth*recordHeight);
  float xoffset = recordWidth/2.0;
  float yoffset = recordWidth/2.0;
  float f_h; //eqn 2 from LC04
  irradianceRecord rec;

  float hmdsum = 0.0;
  
  float dist;
  for(size_t y = 0; y < recordHeight; ++y)
    {
        for(size_t x = 0; x < recordWidth; ++x)
	  {
	     f_h = dA/(M_PI*pow(pow(x - xoffset, 2) + 
			      pow(y - yoffset, 2) +1,2));
	     r += f_h*IMap[(4*(y*recordWidth + x))];
	     g += f_h*IMap[(4*(y*recordWidth + x)) + 1];
	     b += f_h*IMap[(4*(y*recordWidth + x)) + 2];

	     dist = IMap[(4*(y*recordWidth + x)) + 3];
	     if(fabs(dist) < .0001) {}
	     else
	       hmdsum += 1.0/dist;

	  }
    }
  float hmd = (recordWidth*recordWidth)/hmdsum;
  std::cout << "sum: " << r << ' ' << g << ' ' << b << std::endl;
  std::cout << "hmd: " << hmd << std::endl;
  rec.pos[0] = pos[0];
  rec.pos[1] = pos[1];
  rec.pos[2] = pos[2];
  rec.norm[0] = normal[0];
  rec.norm[1] = normal[1];
  rec.norm[2] = normal[2];
  rec.transGrad[0] = 0;
  rec.transGrad[1] = 0;
  rec.transGrad[2] = 0;
  rec.rotGrad[0] = 0;
  rec.rotGrad[1] = 0;
  rec.rotGrad[2] = 0;

  rec.irradiance[0] = r;
  rec.irradiance[1] = g;
  rec.irradiance[2] = b;
  
  rec.hmd = hmd;

  records.push_back(rec);
}


void Scene::drawAtPoint(float*point, float* direction,
			    float*up, GLuint fbo, int width, int height)
{


  glUseProgram(0);
  glViewport(0,0,shadowMapSize, shadowMapSize);
  
  for(size_t i = 0; i < lights.size(); ++i)
    lights[i]->execute();

  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glCullFace(GL_FRONT);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBOID);

  Helpers::getGLErrors("After bindFramebufferEXT");
  for (unsigned light = 0; light < numLights; ++ light)
    {
      std::cout << "Light: " << light << std::endl;
      glFramebufferTextureLayerARB(GL_FRAMEBUFFER_EXT, 
				   GL_DEPTH_ATTACHMENT_EXT,
				   shadowMapTexture, 0, light);
      
      Helpers::getGLErrors("After textureLayer call");

      glClear(GL_DEPTH_BUFFER_BIT);

      float* lightPos;
      float lightLookAt[3];
      lightPos = lights[light]->getPosition();
      lights[light]->lookAt(lightLookAt);

      std::cerr << "light pos: " << lightPos[0] << ' ' << lightPos[1] <<
	' ' << lightPos[2] << std::endl << "Look at" << lightLookAt[0] << 
	lightLookAt[1] << lightLookAt[2] << std::endl;

      float cutoff = lights[light]->getCutoff();
      std::cout << "light cutoff: " << cutoff << std::endl;
      
      viewProjSetup(lightPos, lightLookAt, up, cutoff);

      Helpers::getGLErrors("After first viewProjSetup");

      

      drawObjects();
  
      Helpers::getGLErrors("After drawObjects in light pass");
      viewProjSetup(lightPos, lightLookAt, up, cutoff);

      texMatSetup(light);//load bias and projection/
    }

  Helpers::getGLErrors("After texture renders");
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);//render to window
  glViewport(0,0,width, height);

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(shadowProgram);
  glUniform1i(shadowTexUniform, texUnitBase);
  glUniform1i(uniformTexUnitBase, texUnitBase);
  glUniform1i(uniformNumLights, numLights);

  /*for(unsigned light = 0; light < numLights; ++light)
    {
      glActiveTexture(texUnitEnums[light]);
      glBindTexture(GL_TEXTURE_2D, shadowMapTextureBase + light);
      }*/
  viewProjSetup(point, direction, up);

  glCullFace(GL_BACK);
  drawObjects();
  glFlush();

}





void Scene::viewProjSetup(float *eye, float*eyedir, float* up, float fovy,
			  float aspectRatio)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(fovy, aspectRatio, closePlane, 30);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(eye[0], eye[1], eye[2], eyedir[0], eyedir[1], eyedir[2], 
	    up[0], up[1], up[2]);

}

void Scene::texMatSetup(unsigned lightNum)
{
  float modelView[16];
  float proj[16];
  const GLfloat biasMat[16] = {0.5, 0.0, 0.0, 0.0,
			       0.0, 0.5, 0.0, 0.0,
			       0.0, 0.0, 0.5, 0.0,
			       0.5, 0.5, 0.5, 1.0};

  glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
  glGetFloatv(GL_PROJECTION_MATRIX, proj);

  glMatrixMode(GL_TEXTURE);
  glActiveTexture(texUnitEnums[lightNum]);
  glLoadIdentity();
  glLoadMatrixf(biasMat);
  glMultMatrixf(proj);
  glMultMatrixf(modelView);
  glMatrixMode(GL_MODELVIEW);

}


GLuint Scene::loadShader(std::string filename, GLenum type)
{
  GLuint handle = glCreateShader(type);
  std::ifstream infile;
  infile.open(filename.c_str());
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


  GLint result;
  glGetProgramiv(shadowProgram, GL_LINK_STATUS, &result);
  if(! result)
    {
      GLint logLength;
      std::cerr << "link failed : " << std::endl;
      glGetProgramiv(shadowProgram, GL_INFO_LOG_LENGTH, &logLength);
      char * log = new char[logLength];
      GLint actualLength;
      glGetProgramInfoLog(shadowProgram, logLength, &actualLength, log);
      std::cerr << "Error: " << log << std::endl;

      /*GLint shaderLen;
      glGetProgramiv(handle, GL_SHADER_SOURCE_LENGTH, &shaderLen);
      std::cerr << "Shader length: " << shaderLen << std::endl;
      */
      delete [] log;
    }



  shadowTexUniform = glGetUniformLocation(shadowProgram, "ShadowMap");
  uniformTexUnitBase = glGetUniformLocation(shadowProgram, "texUnitBase");
  uniformNumLights = glGetUniformLocation(shadowProgram, "numLights");
  
  Helpers::getGLErrors("end of loadShadowShader");

}



void Scene::drawObjects()
{
  //glPushMatrix();
  for(std::vector<GLCommand*>::iterator i = model.begin();
      i != model.end(); ++i)
    (*i)->execute();
  //glPopMatrix();

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
  //  glDisable(GL_LIGHTING);
  //  glColor3f(1, 1, 1);
  drawObjects();
  
  //float* pos = lights[0]->getPosition();
  float rgba[4] = {1,1,1,1};
  glPointSize(10);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, rgba);
  glBegin(GL_POINTS);
  glVertex3f(-5, 5, 5);
  glEnd();
  glFlush();
  glutSwapBuffers();


}


void Scene::readCoordNormals()
{
  //render scene to texture and read it out

  loadCoordNormalShader();

  GLuint fboid;
  glGenFramebuffersEXT(1, &fboid);
  glGenTextures(3, coordTexBase);
  glBindTexture(GL_TEXTURE_2D, coordTexBase[0]);
  

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);


  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, windWidth, windHeight,
	       0, GL_RGBA, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, coordTexBase[1]);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, windWidth, windHeight,
	       0, GL_RGBA, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, coordTexBase[2]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windWidth, windHeight,
	       0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);


  Helpers::getGLErrors("After texImage calls");
  
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboid);

  Helpers::getGLErrors("after bindFramebuffer");
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			 GL_TEXTURE_2D, coordTexBase[0], 0);
  Helpers::getGLErrors("After first framebufferTex2D");
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT,
			 GL_TEXTURE_2D, coordTexBase[1], 0);
  Helpers::getGLErrors("After 2nd framebufferTex2D");

  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
			    GL_TEXTURE_2D, coordTexBase[2], 0);

  glViewport(0,0, windWidth, windHeight);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);

  Helpers::getGLErrors("After cullFace, before drawBufs");
  glReadBuffer(GL_NONE);
  const GLenum drawbufs[2] = {GL_COLOR_ATTACHMENT0_EXT, 
			      GL_COLOR_ATTACHMENT1_EXT};
  glDrawBuffers(2, drawbufs);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  float eye[3];
  float eyedir[3];
  float up[3] = {0, 1, 0};
  view->getEye(eye);
  view->getCenter(eyedir);
  viewProjSetup(eye, eyedir, up);
  
  glUseProgram(coordNormalProgram);

  drawObjects();

  glFlush();

  glIsShader(999);//debug

  //read values 
  glBindTexture(GL_TEXTURE_2D, coordTexBase[0]);
  objectCoords = new float[3*windWidth*windHeight];
  objectNormals = new float[3*windWidth*windHeight];
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, objectCoords);
  
  glBindTexture(GL_TEXTURE_2D, coordTexBase[1]);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, objectNormals);

  /*for(size_t i = 0; i < 3*windWidth*windHeight; ++i)
    std::cout << objectCoords[i];
    std::cout << std::endl;*/


  glDeleteFramebuffersEXT(1, &fboid);

  Helpers::getGLErrors("End of coordNormalSetup");

}


void Scene::loadCoordNormalShader()
{
  GLuint vShaderHandle, fShaderHandle;
  vShaderHandle = loadShader("coordNormalVertexShader.c", GL_VERTEX_SHADER);
  fShaderHandle = loadShader("coordNormalFragmentShader.c", 
			     GL_FRAGMENT_SHADER);

  coordNormalProgram = glCreateProgram();
  glAttachShader(coordNormalProgram, vShaderHandle);
  glAttachShader(coordNormalProgram, fShaderHandle);
  glLinkProgram(coordNormalProgram);


  GLint result;
  glGetProgramiv(coordNormalProgram, GL_LINK_STATUS, &result);
  if(! result)
    {
      GLint logLength;
      std::cerr << "link failed coordNormal shader: " << std::endl;
      glGetProgramiv(coordNormalProgram, GL_INFO_LOG_LENGTH, &logLength);
      char * log = new char[logLength];
      GLint actualLength;
      glGetProgramInfoLog(coordNormalProgram, logLength, &actualLength, log);
      std::cerr << "Error: " << log << std::endl;

      /*GLint shaderLen;
      glGetProgramiv(handle, GL_SHADER_SOURCE_LENGTH, &shaderLen);
      std::cerr << "Shader length: " << shaderLen << std::endl;
      */
      delete [] log;
    }

  Helpers::getGLErrors("end of loadCoordNormalShader");

}
