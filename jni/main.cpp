#include<stdio.h>
#include<stdlib.h>
#include"readbmp.h"
#include"imageprocess.h"


void RBG2Gray_GPU(ClImage *Src,ClImage* Dst);
extern ClImage* RGB2Gray(ClImage *Src);
int main()
{
	ClImage *picData;
	ClImage *picGray,*picGPU;
	picData=clLoadImage("test.bmp");
	
	picGPU=(ClImage*)malloc(sizeof(ClImage));
	picGPU->channels=1;
	picGPU->height=picData->height;
	picGPU->width=picData->width;
	picGPU->imageData=(unsigned char*)malloc(sizeof(unsigned char)*picData->height*picData->width);
	RBG2Gray_GPU(picData,picGPU);
	clSaveImage("gpu.bmp",picGPU);

	picGray=RGB2Gray(picData);
	clSaveImage("cpu.bmp",picGray);

	free(picGPU->imageData);
	free(picGPU);
	free(picData->imageData);
	free(picData);
	return 0;
}