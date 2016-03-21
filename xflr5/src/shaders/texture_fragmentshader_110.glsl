#version 110

uniform bool lightOn;
uniform bool hasTexture;
uniform vec3 LightPosition_worldspace;
uniform vec4 LightColor;
uniform float LightAmbient, LightDiffuse, LightSpecular;
uniform float Kc, Kl, Kq;
uniform int MaterialShininess;
uniform sampler2D textureSampler;
uniform vec4 clipPlane0; // defined in view-space


varying vec3 Position_worldspace;
varying vec3 Normal_cameraspace;
varying vec3 EyeDirection_cameraspace;
varying vec3 LightDirection_cameraspace;
varying vec4 vertexcolor;
varying vec2 UV;
varying vec3 vPosition;

void main()
{
	// Material properties
	vec4 MaterialAmbientColor, MaterialDiffuseColor, MaterialSpecularColor;

	if (vPosition.z > clipPlane0.w)
	{
		discard;
		return;
	}

	if(lightOn)
	{
		if(hasTexture)
		{
			MaterialAmbientColor  = vec4(texture2D(textureSampler, UV).rgb*LightAmbient, vertexcolor.a) ;
			MaterialDiffuseColor  = vec4(texture2D(textureSampler, UV).rgb*LightDiffuse, vertexcolor.a);
		}
		else
		{
			MaterialAmbientColor  = vec4(vertexcolor.rgb * LightAmbient, vertexcolor.a);
			MaterialDiffuseColor  = vec4(vertexcolor.rgb * LightDiffuse, vertexcolor.a);
		}
		MaterialSpecularColor = vec4(1.0, 1.0, 1.0, 1.0);

		// Distance to the light
		float distance = length(LightPosition_worldspace - Position_worldspace);

		// Normal of the computed fragment, in camera space
		vec3 N = normalize(Normal_cameraspace);

		// Direction of the light (from the fragment to the light)
		vec3 L = normalize(LightDirection_cameraspace);

		// Cosine of the angle between the normal and the light direction,
		// clamped above 0
		//  - light is at the vertical of the triangle -> 1
		//  - light is perpendicular to the triangle -> 0
		//  - light is behind the triangle -> 0
		float cosTheta = clamp(dot(N,L), 0.0, 1.0);

		// Eye vector (towards the camera)
		vec3 E = normalize(EyeDirection_cameraspace);

		// Direction in which the triangle reflects the light
		vec3 R = reflect(-L,N);

		// Cosine of the angle between the Eye vector and the Reflect vector,
		float cosAlpha = clamp(dot(E,R), 0.0, 1.0);

		float attenuation_factor = 1.0/(Kc + Kl*distance + Kq*distance*distance);

		gl_FragColor =
			  MaterialAmbientColor  * LightColor +
			 (MaterialDiffuseColor  * LightDiffuse  * cosTheta)                         * LightColor * attenuation_factor
			+(MaterialSpecularColor * LightSpecular * pow(cosAlpha, MaterialShininess)) * LightColor * attenuation_factor;
//		fragColor = vec4(cosAlpha,cosAlpha,cosAlpha,1.0);
	}
	else
	{
		if(hasTexture)
		{
			gl_FragColor  = vec4(texture2D(textureSampler, UV).rgb, vertexcolor.a) ;
		}
		else
		{
			gl_FragColor  = vertexcolor;
		}
	}
}
















