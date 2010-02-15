#include "SceneObject.h"
#include <string>
#include "Helpers.h"

SceneObject::SceneObject(std::instream ins)
{
  parseOBJ(ins);
}

void SceneObject::parseOBJ(std::isteam ins)
{
  std::string curLine;
  std::vector<std::string> tokens;
  while (std::getline(inst, curLine))
    {
      Helpers.tokenize(curLine, tokens);
      for(tokens::iterator i = tokens.start(); i != tokens.end(); ++i)
	std::cout << "token: " << *i << std::endl;

    }

}
