
uniform sampler2D direct;
uniform sampler2D splat;
uniform vec2 windSize;

void main()
{
  vec4 splatRec = texture2D(splat, gl_TexCoord[0].xy);

  vec3 splatColor = splatRec.rgb/(splatRec.a);
  vec4 directRec = texture2D(direct, gl_TexCoord[0].xy);
  
  //vec4 outCol = 0.75*directRec + vec4(splatColor, 1.0);

  //gl_FragColor = vec4(outCol.rgb, 1.0);

  gl_FragColor = vec4(splatColor, 1.0) + .5*directRec;

}
