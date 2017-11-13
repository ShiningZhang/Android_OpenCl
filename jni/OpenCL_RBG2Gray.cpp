#include"readbmp.h"
#include<stdio.h>
#include<stdlib.h>
#include<CL\cl.h>
#include"time_cal.h"
#include"OpenCL_phone.h"
void checkErr(cl_int err,int num)
{
	if(CL_SUCCESS!=err){
		printf("OpenCL error(%d) at %d\n",err,num-1);
	}
}

 char* readFile(const char* filename, size_t* length)
 {
 	FILE* file = NULL;
 	size_t sourceLength;
 	char* sourceString;
 	int ret;
 	file = fopen(filename, "rb");
 	if(file == NULL) {
 			printf("%s at %d :Can't open %s\n",__FILE__,__LINE__-2, filename);
 			return NULL;
 	}

 	fseek(file, 0, SEEK_END);
 	sourceLength = ftell(file);
     fseek(file, 0, SEEK_SET);

    	sourceString = (char *)malloc(sourceLength + 1);
     ret= fread((sourceString), sourceLength, 1, file);

     if(ret == 0) {
     	printf("%s at %d : Can't read source %s\n",__FILE__,__LINE__-2, filename);
     	return NULL;
     }


     fclose(file);
     if(length != 0){
         *length = sourceLength;
     }
     sourceString[sourceLength] = '\0';

     return sourceString;
 }


void OpenCL_Init(cl_device_id *device)
{
	cl_int err;
	cl_uint num_platform;
	cl_uint num_device;
	cl_platform_id *platform;
	cl_device_id *devices;

	int loadedCL=load_Func();
	
	err=rclGetPlatformIDs(0,0,&num_platform);
	checkErr(err,__LINE__);
	platform=(cl_platform_id*)malloc(sizeof(cl_platform_id)*num_platform);
	err=rclGetPlatformIDs(num_platform,platform,NULL);
#if 1
	char name1[200];
	for(int i=0;i<num_platform;i++){
		err=rclGetPlatformInfo(platform[i],CL_PLATFORM_NAME,200,name1,NULL);
		printf("platform name:%s\n",name1);
	}
#endif
	checkErr(err,__LINE__);
	err=rclGetDeviceIDs(platform[0],CL_DEVICE_TYPE_ALL,0,NULL,&num_device);
	devices=(cl_device_id*)malloc(sizeof(cl_device_id)*num_device);
	err=rclGetDeviceIDs(platform[0],CL_DEVICE_TYPE_ALL,num_device,devices,NULL);
	*device=devices[0];
#if 1
	char name[200];
	err=rclGetDeviceInfo(devices[0],CL_DEVICE_NAME,200,name,NULL);
	printf("name:%s\n",name);
	size_t max;
	err=rclGetDeviceInfo(devices[0],CL_DEVICE_MAX_WORK_GROUP_SIZE,sizeof(max),&max,NULL);
	printf("max=%d\n",max);
#endif
	free(platform);
	free(devices);

}
void RBG2Gray_GPU(ClImage *Src,ClImage* Dst)
{
	
	cl_device_id device;
	cl_context context;
	cl_int err;
	cl_command_queue comqueue;
	cl_mem dSrc;
	cl_mem dDst;
	cl_kernel kernel;
	cl_program program;
	cl_event gpuExecution;
	cl_event datacopy[2];

	

	OpenCL_Init(&device);
	context=rclCreateContext(NULL,1,&device,NULL,NULL,&err);
	checkErr(err,__LINE__);

	comqueue=rclCreateCommandQueue(context,device,CL_QUEUE_PROFILING_ENABLE,&err);
	checkErr(err,__LINE__);

	size_t program_length;
	char*const source=readFile("kernel.cl",&program_length);
	program=rclCreateProgramWithSource(context,1,(const char **) &source, NULL, &err);
	checkErr(err,__LINE__);

	err=rclBuildProgram(program,1,&device,NULL,NULL,NULL);
	checkErr(err,__LINE__);
	if(CL_SUCCESS!=err){
		char *buffer;
		printf("Error: Failed to build program executable!");
		size_t logsize;
		err=rclGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG,0,NULL,&logsize);
		if(CL_SUCCESS==err){
			printf("logsize=%d\n",logsize);
		}
		buffer=(char*)malloc(logsize*sizeof(char));
		err=rclGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG,logsize,buffer,NULL);
		checkErr(err,__LINE__)  ;
		printf("log:%s\n",buffer);
		free(buffer);
	}
	kernel=rclCreateKernel(program,"RGB2Gray",&err);
	checkErr(err,__LINE__);


	/*¼ÆËãÊ±¼ä*/
	StartTimer();

	int buffersize=Src->height*Src->width*Src->channels*sizeof(unsigned char);
	dSrc=rclCreateBuffer(context,CL_MEM_READ_ONLY,buffersize,0,&err);
	checkErr(err,__LINE__);
	err=rclEnqueueWriteBuffer(comqueue,dSrc,CL_TRUE,0,buffersize,Src->imageData,0,NULL,NULL);

	buffersize=Src->height*Src->width*sizeof(unsigned char);
	dDst=rclCreateBuffer(context,CL_MEM_WRITE_ONLY,buffersize,0,&err);


	err=rclSetKernelArg(kernel,0,sizeof(cl_mem),&dSrc);
	checkErr(err,__LINE__);
	err=rclSetKernelArg(kernel,1,sizeof(cl_mem),&dDst);
	checkErr(err,__LINE__);
	err=rclSetKernelArg(kernel,2,sizeof(int),&Src->width);
	checkErr(err,__LINE__);
	err=rclSetKernelArg(kernel,3,sizeof(int),&Src->height);
	checkErr(err,__LINE__);
	err=rclSetKernelArg(kernel,4,sizeof(int),&Src->channels);
	checkErr(err,__LINE__);

	

	size_t LocalWorkSize=256;
	size_t GlobalWokrSize[2]={Src->width,Src->height};

	err=rclEnqueueNDRangeKernel(comqueue,kernel,2,NULL,GlobalWokrSize,NULL,0,NULL,&gpuExecution);
	checkErr(err,__LINE__);
	err=rclWaitForEvents(1,&gpuExecution);

	cl_ulong start,end;
	err=rclGetEventProfilingInfo(gpuExecution,CL_PROFILING_COMMAND_START,sizeof(cl_ulong),&start,NULL);
	//err=clGetEventProfilingInfo(gpuExecution,CL_PROFILING_COMMAND_SUBMIT,sizeof(cl_ulong),&start,NULL);
	checkErr(err,__LINE__);
	err=rclGetEventProfilingInfo(gpuExecution,CL_PROFILING_COMMAND_END,sizeof(cl_ulong),&end,NULL);
	checkErr(err,__LINE__);
	double Compute_time =  (double)1.0e-9 * (end - start);
	//printf("GPU kernel time is %f(s)\n",Compute_time);

	err=rclEnqueueReadBuffer(comqueue,dDst,CL_TRUE,0,buffersize,Dst->imageData,0,NULL,NULL);
	checkErr(err,__LINE__);
	
	double gpu_ms = GetTimer()/1000;

	printf("GPU time is %f(s)\n",gpu_ms);
	err=rclReleaseContext(context);
	checkErr(err,__LINE__);
	err=rclReleaseCommandQueue(comqueue);
	checkErr(err,__LINE__);
	err=rclReleaseKernel(kernel);
	checkErr(err,__LINE__);
	err=rclReleaseProgram(program);
	checkErr(err,__LINE__);
	err=rclReleaseMemObject(dSrc);
	checkErr(err,__LINE__);
	err=rclReleaseMemObject(dDst);
	checkErr(err,__LINE__);
	
}