#ifndef _SCENE_OBJECT_H
#define _SCENE_OBJECT_H
#include <istream>
#include<vector>

class SceneObject
{
 public:
  SceneObject(std::instream ins);
 private:
struct VertexData
{
  float x,y,z; //vertex
  float nx, ny, nz; //normal
  float r,g,b; //color TODO: Actual material props
};

 parseOBJ(std::istream ins);

 std::vector<VertexData> vertices;
 
};


#endif //_SCENE_OBJECT_H
