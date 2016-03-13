#version 110

uniform vec4 color;
varying vec3 vPosition;
uniform vec4 clipPlane0; // defined in view-space

void main(void)
{
	if (vPosition.z > clipPlane0.w) {
	  discard;
	} else {
		gl_FragColor = color;
	}
}
