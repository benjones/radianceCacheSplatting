#include "Scene.h"

#include <cstdlib>
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

#include "Matrix.h"

const float closePlane = 2.0;

const float recordFront = .1;
const float recordBack = 30;

const float a = 1.0;


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

  srand(0);//TODO SEED WITH TIME
  parseScene(ins);

  //create shadow map texture
  glGenTextures(1, &shadowMapTexture);
  glGenFramebuffersEXT(1, &FBOID);

  glGenFramebuffersEXT(1, &recordFBOID);
  glGenTextures(2, recordTexBase);

  glGenFramebuffersEXT(1, &directFBOID);
  glGenTextures(2, directTex);

  glBindTexture(GL_TEXTURE_2D, directTex[0]);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windWidth, windHeight,
	       0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  glBindTexture(GL_TEXTURE_2D, directTex[1]);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windWidth, windHeight,
	       0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

  
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, directFBOID);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			    GL_TEXTURE_2D, directTex[0], 0);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
			    GL_TEXTURE_2D, directTex[1], 0);


  glGenFramebuffersEXT(1, &splatBufFBOID);
  glGenTextures(1, &splatTex);

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

  
  //set up splat buffer
  glBindTexture(GL_TEXTURE_2D, splatTex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, windWidth, windHeight,
	       0, GL_RGBA, GL_FLOAT, NULL);
  //it doesn't need a depth attachment



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
  loadSplatShader();
  loadFinalShader();
  readCoordNormals();
  //  std::cout << "constructor completed" << std::endl;

  IMap = new float[4*recordWidth*recordHeight];
  splatBuffer = new float[4*windWidth*windHeight];
  warmupCache(1000);//generate starting records

}

Scene::~Scene()
{
  glDeleteTextures(1, &shadowMapTexture);
  glDeleteFramebuffersEXT(1, &FBOID);
  glDeleteTextures(4, coordTexBase);
  glDeleteFramebuffersEXT(1, &recordFBOID);
  glDeleteTextures(2, recordTexBase);
  glDeleteFramebuffersEXT(1, &directFBOID);
  glDeleteTextures(2, directTex);
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
  delete [] splatBuffer;
  delete [] IMap;
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


void Scene::display()
{

  directIllumination();
  /*
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glBlendColor(.75, .75, .75, .75);
  glBlendFunc(GL_ONE, GL_CONSTANT_ALPHA);*/
  splatRecords();


  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glViewport(0, 0, windWidth, windHeight);
  float ws[2] = {windWidth, windHeight};
  glUseProgram(finalProgram);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, splatTex);
  glUniform1i(finalSplat, 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, directTex[0]);
  glUniform1i(finalDirect, 1);

  glUniform2fv(finalWindSize, 1, ws);

  glDisable(GL_BLEND);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glActiveTexture(GL_TEXTURE0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  
  glDisable(GL_CULL_FACE);
  glBegin(GL_QUADS);
  glTexCoord2f(0.0,0.0);
  glVertex2f(-1.0, -1.0);
  glTexCoord2f(1.0, 0.0);
  glVertex2f(1.0, -1.0);

  glTexCoord2f(1.0, 1.0);
  glVertex2f(1.0, 1.0);
  glTexCoord2f(0.0, 1.0);
  glVertex2f(-1.0, 1.0);

  glEnd();

  /* glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
  glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);//splatBufFBOID);
  
  glBlitFramebufferEXT(0,0, windWidth, windHeight,
		       0,0, windWidth, windHeight,
		       GL_COLOR_BUFFER_BIT, GL_NEAREST);*/

  glFlush();


  glutSwapBuffers();


}

void Scene::splatRecords()
{
  /*irradianceRecord rec1, rec2, rec3;
  rec1.pos[0] = 2.1766;
  rec1.pos[1] = 1.8176;
  rec1.pos[2] = 5.1762;
  rec1.norm[0] = -.573576;
  rec1.norm[1] = 0;
  rec1.norm[2] = -.819152;
  rec1.irradiance[0] = 1;
  rec1.irradiance[1] = 0;
  rec1.irradiance[2] = 0;
  rec1.hmd = 1.0;

  records.clear();
  records.push_back(rec1);
  */
  float eye[3];
  float eyedir[3];
  float up[3] = {0.0, 1.0, 0.0};
  view->getEye(eye);
  view->getCenter(eyedir);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, splatBufFBOID);
  glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
  glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			    GL_TEXTURE_2D, splatTex, 0);
  glViewport(0, 0, windWidth, windHeight);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);//add all the contributions together
  
  glUseProgram(splatProgram);
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  viewProjSetup(eye, eyedir, up, 45.0, 1.0, defaultFront, defaultBack);

  float color[4] = {1, 0, 1, 1};
  glPointSize(5);
  glColor4fv(color);

  //set up samplers
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, coordTexBase[0]);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, coordTexBase[1]);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, coordTexBase[2]);
  
  float ws[2] = {windWidth, windHeight};
  std::cout << splatWindSizeUniform << std::endl;
  glUniform2fv(splatWindSizeUniform, 1, ws);

  glUniform1f(splatAUniform, a);

  glUniform1i(splatWorldPosUniform, 0);
  glUniform1i(splatWorldNormUniform, 1);
  glUniform1i(splatDiffuseUniform, 2);

  for(size_t i = 0; i < records.size(); ++i)
    {
      glUniform1f(splatUniform,records[i].hmd*a);
      glUniform1f(splatHmdUniform, records[i].hmd);
      glBegin(GL_QUADS);
      for(int j = 0; j < 4; ++j)
	{
	  glVertexAttrib1f(splatAttribute, 3 - j);
	  glNormal3fv(records[i].norm);
	  glColor3fv(records[i].irradiance);
	  glVertex3fv(records[i].pos);
	}
      glEnd();
    }

  glFlush();
  //glutSwapBuffers();
  std::cout << records.size() << "records splatted" << std::endl;
  Helpers::getGLErrors("end of splat");
  glIsShader(999);
  glIsTexture(222);

  //now do another frag shader pass
  
  //now read the buffer back and to the CPU traversal
  
  glBindTexture(GL_TEXTURE_2D, splatTex);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, splatBuffer);

  int startNewRecords = records.size();
  float * newpos;
  float * newnorm;

  double modelviewmat[16];
  double projectionmat[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelviewmat);
  matrix4x4 modmat(modelviewmat);
  glGetDoublev(GL_PROJECTION_MATRIX, projectionmat);
  matrix4x4 projmat(projectionmat);

  double identity[16] = { 1, 0, 0, 0,
			  0, 1, 0, 0,
			  0, 0, 1, 0,
			  0, 0, 0, 1};

  vector3d xyz, norm, eyeCoord, perpLeft, perpDown;
  float upv[3] = {0, 1, 0}, right[3] = {1, 0, 0};
  vector3d upvec(upv);
  vector3d rightvec(right);

  vector3d botleft, topright;
  vector3d projbotLeft, projtopRight;

  float hmd;

  double x1, y1, z1, x2, y2, z2;//for gluproject

  GLint vp[4] = {0, 0, windWidth, windHeight};

  int xmin, xmax, ymin, ymax;

  for(size_t y = 0; y < windHeight; ++y)
    {
      for(size_t x = 0; x < windWidth; ++x)
	{
	  if(splatBuffer[(y*windWidth +x)*4 + 3] < a)
	    {
	      newpos = &objectCoords[(y*windWidth +x)*3];
	      newnorm = &objectNormals[(y*windWidth +x)*3];
	      generateRecord(newpos, newnorm);

	      hmd = records.back().hmd;

	      //do a simple cpusplat to update nearby weights
	      //mimic the v and f shaders almost exactly
	      xyz = vector3d(newpos);
	      norm = vector3d(newnorm);

	      eyeCoord = modmat.mult(xyz);
	      
	      perpLeft = (((eyeCoord.normalize()).cross(upvec)).normalize()).scale(a*hmd);
	      perpDown = (((eyeCoord.normalize()).cross(rightvec)).normalize()).scale(a*hmd);
	      

	      botleft = eyeCoord + perpLeft + perpDown;
	      topright = eyeCoord - perpLeft - perpDown;
	      
	      gluProject(botleft.xyz[0], botleft.xyz[1], botleft.xyz[2],
			 identity, projectionmat, vp,
			 &x1, &y1, &z1);

	      gluProject(topright.xyz[0], topright.xyz[1], topright.xyz[2],
			 identity, projectionmat, vp,
			 &x2, &y2, &z2);
	      
	      //bottom left is actually bottom right
	      // top right is actually top left
	      //WTF!!

	      xmin = std::max(0, int(x2));
	      xmax = std::min(int(windWidth), int(ceil(x1)));
	      ymin = std::max(0, int(y1));
	      ymax = std::min(int(windHeight), int(ceil(y2)));
	      //std::cout << "updating range: x1 x2, y1 y2: " << xmin <<
	      //' ' << xmax << ' ' << ymin << ' ' << ymax << std::endl;
	      //splat the w coord
	      int offset;
	      float w, dist, sqrtnrm, dot;
	      for(int fragy = ymin; fragy < ymax; ++fragy)
		{
		  for(int fragx = xmin; fragx < xmax; ++fragx)
		    {
		      //compute distance:
		      offset = (fragy*windWidth + fragx)*3;


		      dist = sqrt(pow(newpos[0] - objectCoords[offset], 2) +
				  pow(newpos[1] - objectCoords[offset+1],2)+
				  pow(newpos[2] - objectCoords[offset+2],2));
		      
		      if(dist >= a*hmd)
			continue;
		      dot = newnorm[0]*objectNormals[offset] +
			newnorm[1]*objectNormals[offset +1] +
			newnorm[2]*objectNormals[offset +2];
		      dot = std::min(1.0f, dot);
		      sqrtnrm = sqrt(1.0 - dot);

		      w = 1.0/((dist/hmd) + sqrtnrm);
		      if (w >= 1.0/a)
			{
			  splatBuffer[(fragy*windWidth +fragx)*4 + 3] += w;
			}
		      
		    }

		}

	    }
	}
    }

  std::cout << "about to splat new records" << std::endl;
  glUseProgram(splatProgram);
  //splat the newly added records
  for(size_t i = startNewRecords; i < records.size(); ++i)
    {
      glUniform1f(splatUniform,records[i].hmd*a);
      glUniform1f(splatHmdUniform, records[i].hmd);
      glBegin(GL_QUADS);
      for(int j = 0; j < 4; ++j)
	{
	  glVertexAttrib1f(splatAttribute, 3 - j);
	  glNormal3fv(records[i].norm);
	  glColor3fv(records[i].irradiance);
	  glVertex3fv(records[i].pos);
	}
      glEnd();
    }
  std::cout << records.size() - startNewRecords << " new records created" <<
    std::endl;
  glFlush();


}


void Scene::warmupCache(int numRecs)
{
  
  int x, y;
  
  float point[3];
  float normal[3];
  
  for(int i = 0; i < numRecs; ++i)
    {
      x = rand() % windWidth;
      y = rand() % windHeight;
      
      int offset = (y*windWidth + x)*3;
      
      point[0] = objectCoords[offset];
      point[1] = objectCoords[offset +1];
      point[2] = objectCoords[offset +2];
      
      normal[0] = objectNormals[offset];
      normal[1] = objectNormals[offset +1];
      normal[2] = objectNormals[offset +2];
      
      /*std::cout << "generating record at: (" << x << ", " << y <<
	") with worldCoord: (" << point[0] << ", " << point[1] << 
	", " << point[2] << ") and normal: (" << normal[0] << ", " <<
	normal[1] << ", " << normal[2] << ")" << std::endl;*/
      
      
      generateRecord(point, normal);
    }
}

void Scene::directIllumination()
{

  float eye[3];
  float eyedir[3];
  float up[3] = {0.0, 1.0, 0.0};
  view->getEye(eye);
  view->getCenter(eyedir);

  drawAtPoint(eye, eyedir, up, directFBOID, windWidth, windHeight);

  //glutSwapBuffers();

  glIsShader(999);
  //std::cin.get();
  //glClear(GL_COLOR_BUFFER_BIT);
  //glFlush();
  //  glutSwapBuffers();
  //std::cin.get();
  //Helpers::getGLErrors("End of directIllumination");
  drawAtPoint(eye, eyedir, up, directFBOID, windWidth, windHeight);
  //glutSwapBuffers();
  glIsShader(999);//debug

}

void Scene::generateRecord(float *pos, float* normal)
{
  float eps = .00001;
  float up[3];
  if((fabs(normal[0]) <= eps) && ((fabs(normal[1] -1) <= eps) ||
				  (fabs(normal[1] +1) <= eps)) &&
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
  glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
  
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			    GL_TEXTURE_2D, recordTexBase[0], 0);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT,
			    GL_TEXTURE_2D, recordTexBase[1], 0);
  glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

  drawAtPoint(pos, lookat, up, recordFBOID, recordWidth, recordHeight,
  recordFOV, recordFront, recordBack);


  glBindTexture(GL_TEXTURE_2D, recordTexBase[0]);

  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, IMap);

  float r = 0.0;
  float g = 0.0;
  float b = 0.0;


  float planeDim = 2*closePlane/cos(recordFOV*M_PI/360.0);//width/height of
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
  //std::cout << "sum: " << r << ' ' << g << ' ' << b << std::endl;
  //std::cout << "hmd: " << hmd << std::endl;

  if (hmdsum < .0001 || (r < .0001 && g < .0001 && b < .0001))
    {
      glIsTexture(999);
      return;
    }
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

  glIsShader(999);
}


void Scene::drawAtPoint(float*point, float* direction,
			float*up, GLuint fbo, int width, int height,
			float fovy, float front, float back)
{



  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBOID);

  glUseProgram(0);
  glViewport(0,0,shadowMapSize, shadowMapSize);
  
  for(size_t i = 0; i < lights.size(); ++i)
    lights[i]->execute();

  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glCullFace(GL_FRONT);


  for (unsigned light = 0; light < numLights; ++ light)
    {
      glFramebufferTextureLayerARB(GL_FRAMEBUFFER_EXT, 
				   GL_DEPTH_ATTACHMENT_EXT,
				   shadowMapTexture, 0, light);
      
      glClear(GL_DEPTH_BUFFER_BIT);

      float* lightPos;
      float lightLookAt[3];
      lightPos = lights[light]->getPosition();
      lights[light]->lookAt(lightLookAt);

      float cutoff = lights[light]->getCutoff();
      
      viewProjSetup(lightPos, lightLookAt, up, cutoff);

      drawObjects();
  
      viewProjSetup(lightPos, lightLookAt, up, cutoff);

      texMatSetup(light);//load bias and projection/
    }

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);//render to window
  glViewport(0,0,width, height);

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(shadowProgram);
  glUniform1i(shadowTexUniform, texUnitBase);
  glUniform1i(uniformTexUnitBase, texUnitBase);
  glUniform1i(uniformNumLights, numLights);

  viewProjSetup(point, direction, up, fovy, 1.0, front, back);

  glCullFace(GL_BACK);
  drawObjects();
  glFlush();

}





void Scene::viewProjSetup(float *eye, float*eyedir, float* up, float fovy,
			  float aspectRatio, float front, float back)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(fovy, aspectRatio, front, back);
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


void Scene::loadFinalShader()
{
  GLuint fragHandle;
  fragHandle = loadShader("finalFragmentShader.c", GL_FRAGMENT_SHADER);
  finalProgram = glCreateProgram();
  glAttachShader(finalProgram, fragHandle);
  glLinkProgram(finalProgram);

  GLint result;
  glGetProgramiv(finalProgram, GL_LINK_STATUS, &result);
  if(! result)
    {
      GLint logLength;
      std::cerr << "final shader link failed : " << std::endl;
      glGetProgramiv(finalProgram, GL_INFO_LOG_LENGTH, &logLength);
      char * log = new char[logLength];
      GLint actualLength;
      glGetProgramInfoLog(finalProgram, logLength, &actualLength, log);
      std::cerr << "Error: " << log << std::endl;

      /*GLint shaderLen;
      glGetProgramiv(handle, GL_SHADER_SOURCE_LENGTH, &shaderLen);
      std::cerr << "Shader length: " << shaderLen << std::endl;
      */
      delete [] log;
    }

  finalDirect = glGetUniformLocation(finalProgram, "direct");
  finalSplat = glGetUniformLocation(finalProgram, "splat");
  finalWindSize = glGetUniformLocation(finalProgram, "windSize");
  Helpers::getGLErrors("end of load final shader");
}

void Scene::loadSplatShader()
{
  GLuint vShaderHandle, fShaderHandle;
  vShaderHandle = loadShader("splatVertexShader.c", GL_VERTEX_SHADER);
  fShaderHandle = loadShader("splatFragmentShader.c", GL_FRAGMENT_SHADER);

  splatProgram = glCreateProgram();
  glAttachShader(splatProgram, vShaderHandle);
  glAttachShader(splatProgram, fShaderHandle);
  
  glLinkProgram(splatProgram);


  GLint result;
  glGetProgramiv(splatProgram, GL_LINK_STATUS, &result);
  if(! result)
    {
      GLint logLength;
      std::cerr << "splat link failed : " << std::endl;
      glGetProgramiv(splatProgram, GL_INFO_LOG_LENGTH, &logLength);
      char * log = new char[logLength];
      GLint actualLength;
      glGetProgramInfoLog(splatProgram, logLength, &actualLength, log);
      std::cerr << "Error: " << log << std::endl;

      /*GLint shaderLen;
      glGetProgramiv(handle, GL_SHADER_SOURCE_LENGTH, &shaderLen);
      std::cerr << "Shader length: " << shaderLen << std::endl;
      */
      delete [] log;
    }



  splatUniform = glGetUniformLocation(splatProgram, "radius");
  splatHmdUniform = glGetUniformLocation(splatProgram, "hmd");
  splatWorldPosUniform = glGetUniformLocation(splatProgram, "worldPoss");
  splatWorldNormUniform = glGetUniformLocation(splatProgram, "worldNorms");
  splatDiffuseUniform = glGetUniformLocation(splatProgram, 
					     "diffuseMaterial");
  splatAttribute = glGetAttribLocation(splatProgram, "inCorner");
  splatAUniform = glGetUniformLocation(splatProgram, "a");

  splatWindSizeUniform = glGetUniformLocation(splatProgram,
						     "windSize");
  
  Helpers::getGLErrors("end of loadSplatShader");

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
  glGenTextures(4, coordTexBase);
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

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, windWidth, windHeight,
	       0, GL_RGBA, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, coordTexBase[3]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windWidth, windHeight,
	       0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);


  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboid);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			 GL_TEXTURE_2D, coordTexBase[0], 0);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT,
			 GL_TEXTURE_2D, coordTexBase[1], 0);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT,
			 GL_TEXTURE_2D, coordTexBase[2], 0);

  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
			    GL_TEXTURE_2D, coordTexBase[3], 0);

  glViewport(0,0, windWidth, windHeight);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);

  glReadBuffer(GL_NONE);
  const GLenum drawbufs[3] = {GL_COLOR_ATTACHMENT0_EXT, 
			      GL_COLOR_ATTACHMENT1_EXT,
			      GL_COLOR_ATTACHMENT2_EXT};
  glDrawBuffers(3, drawbufs);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  float eye[3];
  float eyedir[3];
  float up[3] = {0, 1, 0};
  view->getEye(eye);
  view->getCenter(eyedir);
  //viewProjSetup(eye, eyedir, up);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  //WARNING FIXME TODO MAKE SURE THIS IS THE SAME ANGLE AS THE SCENE FOVY
  gluPerspective(45.0, 1.0, closePlane, 30);
  gluLookAt(eye[0], eye[1], eye[2], eyedir[0], eyedir[1], eyedir[2],
	    up[0], up[1], up[2]);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

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
