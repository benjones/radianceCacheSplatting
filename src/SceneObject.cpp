#include "SceneObject.h"
#include <string>
#include <iostream>
#include "Helpers.h"
#include <algorithm>
#include <limits>
SceneObject::SceneObject(std::istream& ins)
{
  parseOBJ(ins);
  dumpData();
}

void SceneObject::drawTriangles()
{
  std::cout << "drawing triangles" << std::endl;

}


void SceneObject::parseOBJ(std::istream& ins)
{
  std::string curLine;
  std::vector<std::string> tokens;
  size_t nVertices = 0, nNormals= 0;//ignore other parts of .obj format
  while (std::getline(ins, curLine))
    {
      //strip off comments:
      curLine = curLine.substr(0, curLine.find_first_of('#'));
      Helpers::tokenize(curLine, tokens);
      
      if(!tokens.size())
	continue;

      //vertex lines
      if(tokens[0] == "v"){
	std::cout << "vertex:" << curLine <<std::endl;
	//parse components:
	if(tokens.size() != 4)
	  {
	    std::cerr << "Ignoring malformed vertex definition: " << 
	      curLine << std::endl;
	    tokens.clear();
	    continue;
	  }
	float x, y, z;
	x = Helpers::str2float(tokens[1]);
	y = Helpers::str2float(tokens[2]);
	z = Helpers::str2float(tokens[3]);
	if(vertices.size() <= std::max(nVertices, nNormals -1))
	  {
	    vertices.push_back(VertexData(x,y,z,0,0,0,0,0,0));
	  }
	else
	  {
	    vertices[nVertices].x = x;
	    vertices[nVertices].y = y;
	    vertices[nVertices].z = z;
	  }
	//std::cout << vertices[nVertices].x << vertices[nVertices].y << vertices[nVertices].z << std::endl;
	nVertices++;
      }

      //texture coordinates
      else if(tokens[0] == "vt"){
	if(tokens.size() != 3)
	  {
	    std::cerr << "Ignoring malformed texture coordinates: " << 
	      curLine << std::endl;
	    tokens.clear();
	    continue;
	  }
	std::cerr << "Ignoring texture line: " << curLine
		  <<std::endl;
      }
      else if(tokens[0] == "vn"){
	//handle normal
	if(tokens.size() != 4)
	  {
	    std::cerr << "Ignoring malformed normal definition: " <<
	      curLine << std::endl;
	    tokens.clear();
	    continue;
	  }
	float nx, ny, nz;
	nx = Helpers::str2float(tokens[1]);
	ny = Helpers::str2float(tokens[2]);
	nz = Helpers::str2float(tokens[3]);
	if(vertices.size() <= std::max(nVertices -1, nNormals))
	  {
	    std::cout << "pushing normal" << nVertices << nNormals <<  vertices.size() << curLine << std::endl;
	    vertices.push_back(VertexData(0,0,0,nx,ny,nz,0,0,0));
	  }
	else
	  {
	    vertices[nNormals].nx = nx;
	    vertices[nNormals].ny = ny;
	    vertices[nNormals].nz = nz;
	  }
	nNormals++;
      }
      else if(tokens[0] == "f"){
	//Faces.  TODO: Handle non-triangles (quads/polygons)
	if(tokens.size() != 4)
	  {
	    std::cerr << "Ignoring malformed face definition: " <<
	      curLine << std::endl;
	    tokens.clear();
	    continue;
	  }
	//see if we're dealing with the v1 v2 v3
	//v1/vt1 v2/vt2 ... or
	//v1/vt1/vn1 v2/vt2/vn3
	std::vector<std::string> pieces;
	Helpers::tokenize(tokens[1], pieces, "/");
	switch(Helpers::countChars(tokens[1], '/'))
	  {
	  case 0:
	    {
	      if(!pieces.size())
		{
		   //tokens contains just a slash?
		  std::cerr << "Ignoring malformed face line: " <<
		    curLine << std::endl;
		  break;
		}
	      //should just be v1 v2 v3
	      //but obj indexes start at 1, not 0
	      int v1, v2, v3;
	      v1 = Helpers::str2int(tokens[1]) -1;
	      v2 = Helpers::str2int(tokens[2]) -1;
	      v3 = Helpers::str2int(tokens[3]) -1;
	      if(v1 < 0 || v2 < 0 || v3 < 0)
		{
		  std::cerr << "Ignoring malformed face line: " <<
		    curLine << std::endl;
		  break;
		}
	      //markers for bad indeces
	      faces.push_back(Face(v1, v2, v3, 
				   std::numeric_limits<unsigned short>::max(),
				   std::numeric_limits<unsigned short>::max(),
				   std::numeric_limits<unsigned short>::max()));
	      break;
	    }
	    
	  case 1:
	    {
	      //must be v1/vt1 
	      int v1, v2, v3;
	      std::vector<std::string> pieces2, pieces3;
	      Helpers::tokenize(tokens[2], pieces2, "/");
	      Helpers::tokenize(tokens[3], pieces3, "/");
	      if(pieces2.size() != 2 || pieces3.size() != 2)
		{
		  std::cerr << "ignoring malformed face line: " <<
		    curLine << std::endl;
		  break;
		}
	      v1 = Helpers::str2int(pieces[0]) -1;
	      v2 = Helpers::str2int(pieces2[0]) -1;
	      v3 = Helpers::str2int(pieces3[0]) -1;
	      if(v1 < 0 || v2 < 0 || v3 < 0)
		{
		  std::cerr << "case 2 < 0: ignoring malformed face line: " << 
		    curLine << std::endl;
		  tokens.clear();
		  break;
		}
	      faces.push_back(Face(v1, v2, v3,
				   std::numeric_limits<unsigned short>::max(),
				   std::numeric_limits<unsigned short>::max(),
				   std::numeric_limits<unsigned short>::max()));
	      break; 
	    }
	  case 2:
	    {
	      //2 cases here: v1//vn1 or v1/vt1/vn1
	      int v1, v2, v3, n1, n2, n3;
	      std::vector<std::string> pieces2, pieces3;
	      Helpers::tokenize(tokens[2], pieces2, "/");
	      Helpers::tokenize(tokens[3], pieces3, "/");
	      if(pieces2.size() == 2 && pieces3.size() == 2)
		{
		  v1 = Helpers::str2int(pieces[0]) -1;
		  v2 = Helpers::str2int(pieces2[0]) -1;
		  v3 = Helpers::str2int(pieces3[0]) -1;
		  n1 = Helpers::str2int(pieces[1]) -1;
		  n2 = Helpers::str2int(pieces2[1]) -1;
		  n3 = Helpers::str2int(pieces3[1]) -1;
	     
		}
	      else if(pieces2.size() == 3 && pieces3.size() == 3)
		{
		  v1 = Helpers::str2int(pieces[0]) -1;
		  v2 = Helpers::str2int(pieces2[0]) -1;
		  v3 = Helpers::str2int(pieces3[0]) -1;
		  n1 = Helpers::str2int(pieces[2]) -1;
		  n2 = Helpers::str2int(pieces2[2]) -1;
		  n3 = Helpers::str2int(pieces3[2]) -1;
		}
	      else
		{
		  std::cerr << "ignoring malformed face line: " <<
		    curLine << std::endl;
		  break;
		}
	      if(v1 < 0 || v2 < 0 || v3 < 0 || n1 < 0 || n2 < 0 || n3 < 0)
		{
		  std::cerr << "Ignoring malformed face line: " <<
		    curLine <<std::endl;
		  break;
		}
	      faces.push_back(Face(v1, v2, v3, n1, n2, n3));
	      break;
	    }
	  default:
	    std::cerr << "ignoring malformed face line: " <<
	      curLine << std::endl;
	  }
      }
      else
	{
	  std::cerr << "ignoring line:" << curLine << std::endl;
	}
      
      tokens.clear();
    }

}


void SceneObject::dumpData()
{
  std::cout << "Vertices: " << std::endl;
  for(std::vector<VertexData>::iterator i = vertices.begin(); 
      i != vertices.end(); ++i)
    {
      std::cout << "x:" << i->x << "\ty:" << i->y << "\tz:" << i->z << "\tnx:" << i->nx << "\tny:" <<i->ny << "\tnz:" << i->nz  <<std::endl;
    }
      
  std::cout << "faces" << std::endl;
  for(std::vector<Face>::iterator i = faces.begin(); i != faces.end(); ++i)
    {
      std::cout << "v1:" << i->v1 << "\tv2:" <<i->v2 << "\tv3:" << i->v3 <<
	"\tn1:" << i->n1 << "\tn2:" <<i->n2 << "\tn3:" <<i->n3 << std::endl;
    }
}
