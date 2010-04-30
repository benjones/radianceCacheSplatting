#ifndef _MATRIX_H
#define _MATRIX_H

#include<cmath>
#include <iostream>

class vector3d
{
 public:
 vector3d(float* in) 
  {
    xyz[0] = in[0];
    xyz[1] = in[1];
    xyz[2] = in[2];
  }
  vector3d() {}

  vector3d cross(vector3d rhs)
  {
    vector3d out;
    out.xyz[0] = (xyz[1]*rhs.xyz[2] - xyz[2]*rhs.xyz[1]);
    out.xyz[1] = -(xyz[0]*rhs.xyz[2] - xyz[2]*rhs.xyz[0]);
    out.xyz[2] = (xyz[0]*rhs.xyz[1] - xyz[1]*rhs.xyz[0]);
    return out;
    
  }

  vector3d normalize()
  {
    vector3d out(xyz);
    float scale = 1.0/sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2]);
    out.xyz[0] *= scale;
    out.xyz[1] *= scale;
    out.xyz[2] *= scale;
    return out;
  } 

  vector3d scale(float s)
  {
    vector3d out;
    out.xyz[0] = xyz[0]*s;
    out.xyz[1] = xyz[1]*s;
    out.xyz[2] = xyz[2]*s;
    return out;
  }

  float xyz[3];

  vector3d operator +(vector3d rhs)
  {
    vector3d out;
    out.xyz[0] = xyz[0] + rhs.xyz[0];
    out.xyz[1] = xyz[1] + rhs.xyz[1];
    out.xyz[2] = xyz[2] + rhs.xyz[2];
    return out;
  }
  vector3d operator -(vector3d rhs)
  {
    vector3d out;
    out.xyz[0] = xyz[0] - rhs.xyz[0];
    out.xyz[1] = xyz[1] - rhs.xyz[1];
    out.xyz[2] = xyz[2] - rhs.xyz[2];
    return out;
  }

  friend  std::ostream& operator <<(std::ostream& outs, const vector3d& v)
    {
      outs << v.xyz[0] << ' ' << v.xyz[1] << ' ' << v.xyz[2] << '\t';
      return outs;
    }

};

class matrix4x4
{
  
 public:
  matrix4x4(double* in) {aa = in;}
  
  vector3d mult(vector3d in)
  {
    vector3d out;
    out.xyz[0] = aa[0]*in.xyz[0] + aa[4]*in.xyz[1] +aa[8]*in.xyz[2] +aa[12];
    out.xyz[1] = aa[1]*in.xyz[0] + aa[5]*in.xyz[1] +aa[9]*in.xyz[2] +aa[13];
    out.xyz[2] = aa[5]*in.xyz[0] + aa[6]*in.xyz[1] +aa[10]*in.xyz[2] +aa[14];

    return out;
  }

  double* aa;

};

#endif
