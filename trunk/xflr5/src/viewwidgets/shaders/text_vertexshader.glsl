#version 130

// Input vertex data, different for all executions of this shader.
in vec4 vertexPosition_modelspace;
in vec3 vertexNormal_modelspace;
in vec2 vertexUV;

uniform  vec4 incolor;

// Values that stay constant for the whole mesh.
uniform mat4 pvmMatrix;
uniform mat4 vMatrix;
uniform mat4 mMatrix;
uniform vec4 clipPlane0; // defined in view-space
uniform vec3 LightPosition_worldspace;

// Output data; will be interpolated for each fragment.
out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;
out vec4 vertexcolor;
out vec2 UV;
out vec3 vPosition;


void main()
{
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  pvmMatrix * vec4(vertexPosition_modelspace);

	vec4 vsPos = vMatrix * mMatrix * vertexPosition_modelspace; // position of vertex in viewspace
//	gl_ClipDistance[0] = dot(clipPlane0,vsPos);
	vPosition = vsPos.xyz / vsPos.w;

	// Position of the vertex, in worldspace : M * position
	Position_worldspace = (mMatrix * vec4(vertexPosition_modelspace)).xyz;

	// Vector that goes from the vertex to the camera, in camera space.
	// In camera space, the camera is at the origin (0,0,0).
	vec3 vertexPosition_cameraspace = ( vMatrix * mMatrix * vec4(vertexPosition_modelspace)).xyz;
	EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
//	vec3 LightPosition_cameraspace = ( vMatrix * vec4(LightPosition_worldspace,1)).xyz;
	vec3 LightPosition_cameraspace = LightPosition_worldspace;
	LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

	// Normal of the the vertex, in camera space
	Normal_cameraspace = ( vMatrix * mMatrix * vec4(vertexNormal_modelspace,0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.

	vertexcolor = incolor;

	UV = vertexUV;
}



