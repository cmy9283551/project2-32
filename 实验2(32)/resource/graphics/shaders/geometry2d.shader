#shader_type vertex
#version 450 core  

layout (location = 0) in vec4 in_positon;
layout (location = 1) in vec4 in_color;
layout (location = 2) in vec4 in_bound;  

layout (location = 0) out vec4 v_color;
layout (location = 1) out vec4 v_bound;

uniform mat4 u_MVP;

void main() {
	gl_Position = u_MVP * in_positon;
	v_color = in_color;
	v_bound = in_bound;
};

#shader_type fragment
#version 450 core  

layout (location = 0) in vec4 v_color;
layout (location = 1) in vec4 v_bound;

layout (location = 0) out vec4 out_color;

bool IsInBound(vec4 bound) {
	return	gl_FragCoord.x > v_bound.x 
		&& gl_FragCoord.x < v_bound.y 
		&& gl_FragCoord.y < v_bound.z 
		&& gl_FragCoord.y > v_bound.w;
}

void main() {
	if (!IsInBound(v_bound)) {
		discard;
	}
	out_color = v_color;
};         