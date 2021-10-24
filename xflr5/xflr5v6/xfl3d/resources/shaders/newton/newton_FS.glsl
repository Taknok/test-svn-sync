#version 330

#define cmul(a, b) vec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)
#define cdiv(a, b) vec2((a.x*b.x+a.y*b.y)/(b.x*b.x+b.y*b.y), (a.y*b.x-a.x*b.y)/(b.x*b.x+b.y*b.y))
#define csub(a, b) vec2(a.x-b.x, a.y-b.y)

uniform int maxiters;
uniform float tolerance;

uniform vec4 color0;
uniform vec4 color1;
uniform vec4 color2;

uniform vec2 root0;
uniform vec2 root1;
uniform vec2 root2;

in vec2 pos;


layout(location = 0) out vec4 FragmentColor;


vec2 polynomial(vec2 z)
{
    vec2 res = cmul(cmul(csub(z, root0), csub(z, root1)), csub(z, root2));

    return res;
}


vec2 derivative(vec2 z)
{
    vec2 res = cmul(csub(z,root1), csub(z,root0)) +
               cmul(csub(z,root2), csub(z,root0)) +
               cmul(csub(z,root2), csub(z,root1));
    return res;
}


void main(void)
{
    vec4 colors[3];
    colors[0] = color0;
    colors[1] = color1;
    colors[2] = color2;

    vec2 roots[3];
    roots[0] = root0;
    roots[1] = root1;
    roots[2] = root2;

    vec2 z = pos;

    for(int iteration=0; iteration<maxiters; iteration++)
    {
        z = z - cdiv(polynomial(z), derivative(z));

        for(int i=0; i<3; i++)
        {
            vec2 difference = z - roots[i];

            if (abs(difference.x)<tolerance && abs(difference.y)<tolerance)
            {
                FragmentColor = colors[i] * min(32.0, float(iteration))/32.0;
                return;
            }
        }
    }

    FragmentColor = vec4(0,0,0,1);
}



