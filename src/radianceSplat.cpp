#include "SceneObject.h"
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GL/glut.h>
#include "Scene.h"

Scene * scn;


unsigned windWidth = 800, windHeight = 800;

void displayWrapper()
{
  //std::cout << "displayWrapper" << std::endl;
  //std::cin.get();
  scn->directIllumination();
  //scn->noShadows();
}

int main(int argc, char** argv)
{

  
  
  glutInit(&argc, argv);
  glutInitWindowSize(windWidth,windHeight);
  glutInitWindowPosition(10,10);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | 
		      GLUT_DEPTH | GLUT_ACCUM);
 
 
  glutCreateWindow("Radiance Cache Splatting");

  GLenum err = glewInit();
  if (GLEW_OK != err)
    {
      /* Problem: glewInit failed, something is seriously wrong. */
      std::cerr <<  "Error: " << glewGetErrorString(err) << std::endl;
      
    }

  GLint texUnits;
  glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &texUnits);
  std::cout << texUnits << " texture units" << std::endl;
  //return 0;

  glClearColor(0.0, 0.0,0.0, 0.0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_NORMALIZE);

  glutDisplayFunc(displayWrapper);

  std::cout << glGetString(GL_VENDOR) << std::endl 
	    << glGetString(GL_RENDERER) << std::endl 
	    << glGetString(GL_VERSION) << std::endl;


  std::ifstream ins;
  ins.open(argv[1]);
  scn = new Scene(ins);

  
  glutMainLoop();

  return 0;
}
