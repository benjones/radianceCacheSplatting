#ifndef _SCENE_OBJECT_H
#define _SCENE_OBJECT_H
#include <istream>
#include <vector>
#include <GL/glew.h>


class SceneObject
{
 public:
  SceneObject(std::istream& ins);
  void drawTriangles();
 private:
  struct VertexData
  {
    float x,y,z; //vertex
    float nx, ny, nz; //normal
    //float r,g,b; //color TODO: Actual material props
    
  VertexData(float _x, float _y, float _z, float _nx, float _ny, float _nz)
	     //float _r, float _g, float _b)
  :x(_x), y(_y), z(_z), nx(_nx), ny(_ny), nz(_nz)//, r(_r), g(_g), b(_b)
    {}
  };
  struct Face
  {
    unsigned short v1,v2,v3; //vertices
    unsigned short n1, n2, n3; //normals
    //TODO add textures
  Face(unsigned short _v1, unsigned short _v2, unsigned short _v3,
       unsigned short _n1, unsigned short _n2, unsigned short _n3)
  :v1(_v1), v2(_v2), v3(_v3), n1(_n1), n2(_n2), n3(_n3)
    {}
  };

  struct Vec3D
  {
    float x,y,z;
    Vec3D(float _x, float _y, float _z)
    :x(_x), y(_y), z(_z)
    {}
  };
  
  void parseOBJ(std::istream& ins);
  void dumpData();
  std::vector<VertexData> vertices;
  //std::vector<Face> faces;
  std::vector<unsigned short> indeces;
  GLuint VBOID, IBOID;//buffers for vertices/indeces
};


#endif //_SCENE_OBJECT_H
