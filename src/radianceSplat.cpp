#include "SceneObject.h"
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GL/glut.h>
#include "Scene.h"

Scene * scn;

void displayWrapper()
{
  scn->directIllumination();
}

int main(int argc, char** argv)
{

  
  
  glutInit(&argc, argv);
  glutInitWindowSize(800,800);
  glutInitWindowPosition(10,10);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
 
  GLenum err = glewInit();
  if (GLEW_OK != err)
    {
      /* Problem: glewInit failed, something is seriously wrong. */
      std::cerr <<  "Error: " << glewGetErrorString(err) << std::endl;
      
    }
 
  glutCreateWindow("Radiance Cache Splatting");
  glClearColor(0.0, 0.0,0.0,0.0);
  glEnable(GL_DEPTH_TEST);
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
