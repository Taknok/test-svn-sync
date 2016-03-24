#version 330

in vec4 vertexPosition_modelspace;
in vec4 vertexColor;

out vec4 outColor;

uniform mat4 pvmMatrix;

void main()
{
   outColor = vertexColor;
   gl_Position = pvmMatrix * vertexPosition_modelspace;
}
