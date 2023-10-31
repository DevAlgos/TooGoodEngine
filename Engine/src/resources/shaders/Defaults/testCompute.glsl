#version 460 core
layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D screen;




void main()
{
	vec4 value = vec4(0.0, 0.0, 1.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
	float v = float(1/texelCoord);

	float radius = 0.8;

	if(v > radius)
	{
		imageStore(screen, texelCoord,value);
	}

	
}

