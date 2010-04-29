attribute float inCorner;
uniform float radius;
uniform float hmd;
//0 (bot left), 1 (bottom right), 2(top right), 3(top left)

varying vec3 worldCoord;
varying vec3 worldNormal;

void main()
{
  int corner = int(inCorner);

  worldCoord = gl_Vertex;
  worldNormal = gl_Normal;

  gl_FrontColor = gl_Color;
  //gl_FrontColor = vec4(1.0,  0.0, 1.0, 1.0);//gl_Color;
  
  //do the computation in eye space
  vec4 eyeCoord = gl_ModelViewMatrix* gl_Vertex;
  //compute delta x/delta z.  Overestimate, go over the perpindicular
  //vector by R
  
  vec4 horizPerpRight = vec4(radius*normalize( 
					      cross(normalize(vec3(eyeCoord)
							      ), 
						    vec3(0.0, 1.0, 0.0))), 
			     0.0);

  vec4 vertPerpUp = vec4(radius*normalize(cross(normalize(vec3(eyeCoord)
							   ), 
						vec3(1.0, 0.0, 0.0))),
			 0.0);

  if(corner == 0)
    {
      gl_Position = gl_ProjectionMatrix*(eyeCoord - horizPerpRight - 
					 vertPerpUp);
    }
  else if(corner == 1)
    {
      gl_Position = gl_ProjectionMatrix*(eyeCoord + horizPerpRight -
					 vertPerpUp);
    }
  else if(corner == 2)
    {
      gl_Position = gl_ProjectionMatrix*(eyeCoord + horizPerpRight +
					 vertPerpUp);
    }
  else
    {
      gl_Position = gl_ProjectionMatrix*(eyeCoord - horizPerpRight +
					 vertPerpUp);
    }

}
