#version 400

//Input type
layout (lines_adjacency) in; // 4 points
//output type
layout (triangle_strip, max_vertices = 120) out;

uniform float width;
#define tension 0.0f

uniform mat4 mvp;

vec4 GetCrossDir(vec3 a, vec3 b)
{
	vec3 dir = normalize(b - a);
	vec3 tangent = cross(dir, vec3(0,0,1));
	return vec4(tangent * width, 0.0);
}

vec3 GetTangent(vec3 p0, vec3 p1, float c)
{
	return (1.0f - c) * ((p0 - p1) / 2.0f);
}

vec3 GetPoint(vec3 p0, vec3 p1, vec3 m0, vec3 m1, float t)
{
	float t2 = t * t;
	float t3 = t * t * t;

	vec3 pt =
		(2 * t3 - 3 * t2 + 1) * p0 +
		(t3 - 2 * t2 + t) * m0 +
		(-2 * t3 + 3 * t2) * p1 +
		(t3 - t2) * m1;

	return pt;
}

void main()
{
	vec3 pn1 = gl_in[0].gl_Position.xyz;
	vec3 p0  = gl_in[1].gl_Position.xyz;
	vec3 p1  = gl_in[2].gl_Position.xyz;
	vec3 p2  = gl_in[3].gl_Position.xyz;
	
	int divisions = int(max(1, length(p0 - p1) / 2.0f));

	vec3 m0 = GetTangent(p1, pn1, tension);
	vec3 m1 = GetTangent(p2, p0, tension);
	
	vec4 tangent;
	vec4 previousTangent =  GetCrossDir(
		pn1,
		p0
	);

	for (int i = 0; i < divisions; i++)
	{
		vec4 a = vec4(GetPoint(p0, p1, m0, m1, float(i + 0) / float(divisions)), 1.0f);
		vec4 b = vec4(GetPoint(p0, p1, m0, m1, float(i + 1) / float(divisions)), 1.0f);
		
		gl_Position = mvp * (a - previousTangent);
		EmitVertex();
	
		gl_Position = mvp * (a + previousTangent);
		EmitVertex();
    
		if (i != divisions - 1)
		{
			tangent = GetCrossDir(a.xyz, b.xyz);
			previousTangent = tangent;
		}
		else
		{
			tangent = GetCrossDir(p0, p1);
		}

		gl_Position = mvp * (b - tangent);
		EmitVertex();
	
		gl_Position = mvp * (b + tangent);
		EmitVertex();
		
	
		EndPrimitive();
	}
}