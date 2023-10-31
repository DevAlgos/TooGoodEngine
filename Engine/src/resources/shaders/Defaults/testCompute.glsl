#version 460 core
layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D screen;




void main()
{
	vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
	ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

	value.x = 1.0/gl_NumWorkGroups.x;
	value.y = 1.0/gl_NumWorkGroups.x;

	imageStore(screen, texelCoord, value);
	
}

