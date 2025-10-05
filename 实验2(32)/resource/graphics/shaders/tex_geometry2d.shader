#shader_type vertex
#version 450 core  

layout (location = 0) in vec4 in_positon;
layout (location = 1) in vec2 in_tex_crood;
layout (location = 2) in vec4 in_color;  
layout (location = 3) in vec4 in_bound;
layout (location = 4) in float in_tex_id_start;
layout (location = 5) in float in_tex_id_end;

uniform mat4 u_MVP;
uniform mat4 u_tex_mat[gl_MaxTextureImageUnits];

layout (location = 0) out vec2 v_tex_crood;
layout (location = 1) out vec4 v_color;
layout (location = 2) out vec4 v_bound;
layout (location = 3) out float v_tex_id_start;
layout (location = 4) out float v_tex_id_end;

void main() {
	gl_Position = u_MVP * in_positon;
	v_tex_crood = (
		vec2(
			u_tex_mat[uint(v_tex_id_start)]
			* vec4(in_tex_crood ,0.0f ,1.0f)
		)
		 + 1.0f
	)/2.0f;	
	v_color = in_color;
	v_bound = in_bound;
	v_tex_id_start = in_tex_id_start;
	v_tex_id_end = in_tex_id_end;
};

#shader_type fragment
#version 450 core  

layout (location = 0) in vec2 v_tex_crood;
layout (location = 1) in vec4 v_color;
layout (location = 2) in vec4 v_bound;
layout (location = 3) in float v_tex_id_start;
layout (location = 4) in float v_tex_id_end;

layout (location = 0) out vec4 out_color;

uniform sampler2D u_texture[gl_MaxTextureImageUnits];

bool IsInBound(vec4 bound) {
	return	gl_FragCoord.x > v_bound.x 
		&& gl_FragCoord.x < v_bound.y 
		&& gl_FragCoord.y < v_bound.z 
		&& gl_FragCoord.y > v_bound.w;
}

void main() {
	if(!IsInBound(v_bound)) {
		discard;
	}
	out_color = texture(
		u_texture[int(v_tex_id_start)], 
		v_tex_crood
	);
	for (
		int i = int(v_tex_id_start)+1; 
		i < int(v_tex_id_end);
		i++
	) {
		out_color = mix(
			out_color, 
			texture(u_texture[i], v_tex_crood), 
			0.5f
		);
	}
	out_color = out_color * v_color;
};         