#include "SceneObject.h"
#include <iostream>
#include <fstream>

int main(int argc, char** argv)
{
  
  std::ifstream ins;
  ins.open(argv[1]);
  SceneObject p(ins);

  return 0;
}
