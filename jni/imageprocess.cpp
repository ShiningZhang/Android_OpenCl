#include<stdio.h>
#include<stdlib.h>
#include"readbmp.h"
#include"time_cal.h"
ClImage* RGB2Gray(ClImage *Src)
{
	ClImage* Dst; 
	int Row=Src->height;
	int Col=Src->width;
	int channel=Src->channels;

	Dst = (ClImage*)malloc(sizeof(ClImage)); 
	Dst->width = Col;  
	Dst->height = Row;  
    Dst->channels = 1;  
	Dst->imageData = (unsigned char*)malloc(sizeof(unsigned char)*Row*Col);  
	/*º∆À„ ±º‰*/
	StartTimer();
	for(int i=0;i<Row;i++){
		for(int j=0;j<Col;j++){
			int B=Src->imageData[i*channel*Col+channel*j];
			int G=Src->imageData[i*channel*Col+channel*j+1];
			int R=Src->imageData[i*channel*Col+channel*j+2];

			int data=(int)(0.3*B+0.59*G+0.11*R);

			Dst->imageData[i*Col+j]=data;
		}
	}

	double cpu_ms = GetTimer()/1000;

	printf("CPU time is %f(s)\n",cpu_ms);
	return  Dst;
}