#include "Light.h"
#include "Helpers.h"
#include <string>
#include <vector>
#include <iostream>
Light::Light(std::istream& ins, GLenum _lightNum)
  :ambient(NULL), diffuse(NULL), specular(NULL), position(NULL),
   spotDirection(NULL), spotExpOn(false), spotCutOn(false), 
   constAttenOn(false), linAttenOn(false), quadAttenOn(false),
   lightNum(_lightNum)
{
  parseLight(ins);
}

void Light::parseLight(std::istream& ins)
{
  std::string curLine;
  std::vector<std::string> tokens;

  while(std::getline(ins, curLine))
    {
      curLine = curLine.substr(0, curLine.find_first_of('#'));
      Helpers::tokenize(curLine, tokens);
      if(!tokens.size())
	continue;
      
      if(tokens[0] == "ambient")
	{
	  if(tokens.size() != 5)
	    {
	      std::cerr << "ignoring malformed ambient line" <<
		curLine << std::endl;
	      tokens.clear();
	      continue;
	    }
	  ambient = new float[4];
	  ambient[0] = Helpers::str2float(tokens[1]);
	  ambient[1] = Helpers::str2float(tokens[2]);
	  ambient[2] = Helpers::str2float(tokens[3]);
	  ambient[3] = Helpers::str2float(tokens[4]);
	  
	}
      else if(tokens[0] == "diffuse")
	{
	  if(tokens.size() != 5)
	    {
	      std::cerr << "ignoring malformed diffuse line" <<
		curLine << std::endl;
	      tokens.clear();
	      continue;
	    }
	  diffuse = new float[4];
	  diffuse[0] = Helpers::str2float(tokens[1]);
	  diffuse[1] = Helpers::str2float(tokens[2]);
	  diffuse[2] = Helpers::str2float(tokens[3]);
	  diffuse[3] = Helpers::str2float(tokens[4]);
	 
	}
      else if(tokens[0] == "specular")
	{
	  if(tokens.size() != 5)
	    {
	      std::cerr << "ignoring malformed specular line" <<
		curLine << std::endl;
	      tokens.clear();
	      continue;
	    }
	  specular = new float[4];
	  specular[0] = Helpers::str2float(tokens[1]);
	  specular[1] = Helpers::str2float(tokens[2]);
	  specular[2] = Helpers::str2float(tokens[3]);
	  specular[3] = Helpers::str2float(tokens[4]);
	 
	}

      else if(tokens[0] == "position")
	{
	  if(tokens.size() != 5)
	    {
	      std::cerr << "ignoring malformed position line" <<
		curLine << std::endl;
	      tokens.clear();
	      continue;
	    }
	  position = new float[4];
	  position[0] = Helpers::str2float(tokens[1]);
	  position[1] = Helpers::str2float(tokens[2]);
	  position[2] = Helpers::str2float(tokens[3]);
	  position[3] = Helpers::str2float(tokens[4]);
	 
	}
      else if(tokens[0] == "spotDirection")
	{
	  if(tokens.size() != 5)
	    {
	      std::cerr << "ignoring malformed spotDirection line" <<
		curLine << std::endl;
	      tokens.clear();
	      continue;
	    }
	  spotDirection = new float[3];
	  spotDirection[0] = Helpers::str2float(tokens[1]);
	  spotDirection[1] = Helpers::str2float(tokens[2]);
	  spotDirection[2] = Helpers::str2float(tokens[3]);
	}
      else if(tokens[0] == "spotExponent")
	{
	  if(tokens.size() != 2)
	    {
	      std::cerr << "Ignoring malformed spotExponent line"<<
		curLine << std::endl;
	    }
	  spotExpOn = true;
	  spotExponent = Helpers::str2float(tokens[1]);
	} 

      else if(tokens[0] == "spotCutoff")
	{
	  if(tokens.size() != 2)
	    {
	      std::cerr << "Ignoring malformed spotCutoff line"<<
		curLine << std::endl;
	    }
	  spotCutOn = true;
	  spotCutoff = Helpers::str2float(tokens[1]);
	} 
      else if(tokens[0] == "constantAttenuation")
	{
	  if(tokens.size() != 2)
	    {
	      std::cerr << "Ignoring malformed constAtten line"<<
		curLine << std::endl;
	    }
	  constAttenOn = true;
	  constantAttenuation = Helpers::str2float(tokens[1]);
	} 
      else if(tokens[0] == "linearAttenuation")
	{
	  if(tokens.size() != 2)
	    {
	      std::cerr << "Ignoring malformed linAttenuation line"<<
		curLine << std::endl;
	    }
	  linAttenOn = true;
	  linearAttenuation = Helpers::str2float(tokens[1]);
	} 
      else if(tokens[0] == "quadraticAttenuation")
	{
	  if(tokens.size() != 2)
	    {
	      std::cerr << "Ignoring malformed quadAtten line"<<
		curLine << std::endl;
	    }
	  quadAttenOn = true;
	  quadraticAttenuation = Helpers::str2float(tokens[1]);
	} 
      else
	{
	  std::cerr << "Ignoring light line: " << curLine << std::endl;
	}
      tokens.clear();

    }

}

void Light::execute()
{
  if(ambient)
    glLightfv(lightNum, GL_AMBIENT, ambient);
  if(diffuse)
    glLightfv(lightNum, GL_DIFFUSE, diffuse);
  if(specular)
    glLightfv(lightNum, GL_SPECULAR, specular);
  if(position)
    glLightfv(lightNum, GL_POSITION, position);
  if(spotDirection)
    glLightfv(lightNum, GL_SPOT_DIRECTION, spotDirection);
  if(spotExpOn)
    glLightf(lightNum, GL_SPOT_EXPONENT, spotExponent);
  if(spotCutOn)
    glLightf(lightNum, GL_SPOT_CUTOFF, spotCutoff);
  if(constAttenOn)
    glLightf(lightNum, GL_CONSTANT_ATTENUATION, constantAttenuation);
  if(linAttenOn)
    glLightf(lightNum, GL_LINEAR_ATTENUATION, linearAttenuation);
  if(quadAttenOn)
    glLightf(lightNum, GL_QUADRATIC_ATTENUATION, quadraticAttenuation);

  glEnable(GL_LIGHTING);
  glEnable(lightNum);
}
