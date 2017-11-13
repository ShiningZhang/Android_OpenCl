 
__kernel void RGB2Gray(__global uchar*  const Src,  __global uchar*Dst,int width,int col,int channel) {
	int id_col=get_global_id(0);
	int id_row=get_global_id(1);
	
	int localsize=get_local_size(0);
	float3  coefficient=(float3)(0.3f,0.59f,0.11f);

	int index=id_row*channel*width+id_col*channel;
	uchar3 data1=vload3(0,Src+index);
	float3 data2=(float3)(data1.x,data1.y,data1.z);
	index=id_row*width+id_col;
	Dst[index]=(uchar)dot(data2,coefficient);

}

