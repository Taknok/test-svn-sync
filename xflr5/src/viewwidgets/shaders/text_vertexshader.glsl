
// Input vertex data, different for all executions of this shader.
attribute highp vec4 vertexPosition_modelspace;
attribute mediump vec3 vertexNormal_modelspace;
attribute mediump vec2 vertexUV;
uniform lowp vec4 incolor;

// Values that stay constant for the whole mesh.
uniform mat4 pvmMatrix;
uniform mat4 vMatrix;
uniform mat4 mMatrix;
uniform vec3 LightPosition_worldspace;


// Output data; will be interpolated for each fragment.
varying mediump vec3 Position_worldspace;
varying mediump vec3 Normal_cameraspace;
varying mediump vec3 EyeDirection_cameraspace;
varying mediump vec3 LightDirection_cameraspace;
varying mediump vec4 vertexcolor;
varying mediump vec2 UV;


void main()
{
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  pvmMatrix * vec4(vertexPosition_modelspace);

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



