#include "Scene.h"

#include <iostream>
#include <fstream>
#include <string>
#include "Helpers.h"
#include <GL/glut.h>
#include "Light.h"

const GLenum Scene::lights[8] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3,
				 GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7};

Scene::Scene(std::istream& ins)
 :numLights(0)
{
  parseScene(ins);

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
	  if(tokens.size() != 10)
	    {
	      std::cerr << "Ignoring malformed camera line: " <<
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
	  view.push_back(new GLULookAt(ix,iy,iz,cx,cy,cz,ux,uy,uz));
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
	  model.push_back(new GLRotate(angle, x, y, z));
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
	  model.push_back(new GLTranslate(x,y,z));
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
	  model.push_back(new GLScale(x,y,z));
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
	  model.push_back(new GLPushMatrix);
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
	  model.push_back(new GLPopMatrix);
	}
      else if(tokens[0] == "proj")
	{
	  if(tokens.size() != 5)
	    {
	      std::cerr << "ignoring malformed projection line" <<
		curLine << std::endl;
	      tokens.clear();
	      continue;
	    }
	  float fovy, aspect, near, far;
	  fovy = Helpers::str2float(tokens[1]);
	  aspect = Helpers::str2float(tokens[2]);
	  near = Helpers::str2float(tokens[3]);
	  far = Helpers::str2float(tokens[4]);
	  projection.push_back(new GLUPerspective(fovy, aspect, near, far));
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
	      Light * light = new Light(ltStr, lights[numLights]);
	      model.push_back(new GLLight(light));
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
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //std::cout << "cleared" << std::endl;
  //std::cin.get();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  for(std::vector<GLCommand*>::iterator i = projection.begin();
      i != projection.end(); ++i)
    {
      (*i)->execute();
    }

  //std::cout << "Projection finished" << std::endl;
  //std::cin.get();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  for(std::vector<GLCommand*>::iterator i = view.begin();
      i != view.end(); ++i)
    {
      (*i)->execute();
    }
  //std::cout << "view done" << std::endl;
  //std::cin.get();
  for(std::vector<GLCommand*>::iterator i = model.begin();
      i != model.end(); ++i)
    {
      (*i)->execute();
      
    }

  //std::cout << "model done" << std::endl;
  //std::cin.get();
  glFlush();
  glutSwapBuffers();
}
