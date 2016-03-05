attribute highp vec4 vertexPosition_modelspace;
attribute lowp vec4 vertexColor;

varying mediump vec4 outColor;

uniform mat4 pvmMatrix;

void main()
{
   outColor = vertexColor;
   gl_Position = pvmMatrix * vertexPosition_modelspace;
}
