#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<stdio.h>

typedef struct FileHeadr
{
	unsigned short bfType;
	unsigned int bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int bfOffbits;
}__attribute__((packed))FileHeader;

typedef struct InfoHeader
{
	unsigned int biSize;
	int biWidth;
	int biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int bitCompression;
	unsigned int biSzieImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
}__attribute__((packed))InfoHeader;

typedef struct tagRGBQUAD
{
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;
}RGBQUAD;

typedef struct{
	unsigned char r;
	unsigned char g;
	unsigned char b;
}RGB_data;

unsigned char *pBmpBuf;
RGBQUAD* pColorTable;
int bmpWidth;
int bmpHeight;
int biBitCount;
char *readBmp(char*bmpName){
	FILE* fp=fopen(bmpName,"rb");
	if(fp==NULL){
		printf("The file is not exist!!!");
		return 0;
	}

	fseek(fp,sizeof(FileHeader),SEEK_SET);
	InfoHeader head;
	fread(&head,sizeof(InfoHeader),1,fp);
	bmpWidth=head.biWidth;
	bmpHeight=head.biHeight;
	biBitCount=head.biBitCount;
	int lineByte=(bmpWidth*biBitCount/8+3)/4*4;
	printf("the lineBype is%d\n",lineByte);
	if(biBitCount==8){
		pColorTable=(RGBQUAD*)malloc(sizeof(RGBQUAD)*1024);
		fread(pColorTable,sizeof(RGBQUAD),256,fp);
	}
	pBmpBuf=(unsigned char*)malloc(sizeof(unsigned char)*lineByte*bmpHeight);
	fread(pBmpBuf,1,lineByte*bmpHeight,fp);
	fclose(fp);
	return pBmpBuf;
}

int clip_value(int x,int min_val,int max_val){
	if(x>max_val){
		return max_val;
	}else if(x<min_val){
		return min_val;
	}else{
		return x;
	}
}
/*将RGB24转换成YV12*/
int RGB24_TO_YV12(char*filename){
	unsigned char*ptrY,*ptrU,*ptrV,*ptrRgb;
	unsigned char r,g,b;
	int y,u,v;
	int i,j;
	int yuvBufsize=(bmpHeight/2*2)*(bmpWidth/2*2)*3/2;
	unsigned char*yuvBuf=(unsigned char*)malloc(yuvBufsize);
	FILE*fp;
	int lineByte=(bmpWidth*biBitCount/8+3)/4*4;
	fp=fopen(filename,"wb+");
	ptrY=yuvBuf;
	ptrV=yuvBuf+(bmpWidth/2*2)*(bmpHeight/2*2);
	ptrU=ptrV+((bmpWidth/2*2)*(bmpHeight/2*2))/4;
	for(i=0;i<bmpHeight/2*2;i++){
		ptrRgb=pBmpBuf+(bmpHeight-i-1)*lineByte;
		for(j=0;j<bmpWidth/2*2;j++){
			b=*(ptrRgb++);
			g=*(ptrRgb++);
			r=*(ptrRgb++);
			y=((66*r+129*g+25*b)>>8)+16;
			u=((-38*r-74*g+112*b)>>8)+128;
			v=((112*r-94*g-18*b)>>8)+128;
			*(ptrY++)=(unsigned char)clip_value(y,0,255);
			if(i%2==0&&j%2==0){
				*(ptrV++)=(unsigned char)clip_value(v,0,255);
			}
			else{
				if(j%2==0){
					*(ptrU++)=(unsigned char)clip_value(u,0,255);
				}
			}

		}

	}
	fwrite(yuvBuf,1,yuvBufsize,fp);
	free(yuvBuf);
	return 1;

}
/*将RGB24转换成YUY2*/
int RGB24_TO_YUY2(char*filename){
	unsigned char*ptrYUY2,*ptrRgb;
	unsigned char r,g,b;
	int y,u,v;
	int i,j;
	unsigned char*yuvBuf=(unsigned char*)malloc((bmpHeight*2/2)*(bmpWidth*2/2)*2);
	FILE*fp;
	int lineByte=(bmpWidth*biBitCount/8+3)/4*4;
	fp=fopen(filename,"wb+");	
	ptrYUY2=yuvBuf;
	for(i=0;i<bmpHeight/2*2;i++){
		ptrRgb=pBmpBuf+(bmpHeight-i-1)*lineByte;
		for(j=0;j<bmpWidth/2*2;j++){
			b=*(ptrRgb++);
			g=*(ptrRgb++);
			r=*(ptrRgb++);
			y=((66*r+129*g+25*b)>>8)+16;
			u=((-38*r-74*g+112*b)>>8)+128;
			v=((112*r-94*g-18*b)>>8)+128;
			if(j%2==0){
				*(ptrYUY2++)=(unsigned char)clip_value(y,0,255);
				*(ptrYUY2++)=(unsigned char)clip_value(u,0,255);
			
			}
			else{
				*(ptrYUY2++)=(unsigned char)clip_value(y,0,255);
				*(ptrYUY2++)=(unsigned char)clip_value(v,0,255);
			}
		}

	}
	fwrite(yuvBuf,1,bmpWidth*bmpHeight*2,fp);
	free(yuvBuf);
	fclose(fp);
	return 1;

}
/*将Bmp报头去除，将RGB单独存到.RGB文件*/
int BMP_TO_RGB(char* filename){
	unsigned char*ptrRgb;
	unsigned char r,g,b;
	FILE*fp;
	int i,j;
	int lineByte=(bmpWidth*biBitCount/8+3)/4*4;
	fp=fopen(filename,"wb+");
	RGB_data Rgb_data;
	for(i=0;i<bmpHeight;i++){
		ptrRgb=pBmpBuf+(bmpHeight-i-1)*lineByte;
		for(j=0;j<bmpWidth;j++){
			Rgb_data.b=*(ptrRgb++);
			Rgb_data.g=*(ptrRgb++);
			Rgb_data.r=*(ptrRgb++);
			fwrite(&Rgb_data,sizeof(Rgb_data),1,fp);
		}
		
	}
	fclose(fp);
	return 1;
}

int main(){
	char readPath[]="test.bmp";
	char writePath[]="test-yv12.yuv";
	char BmptoRgb[]="test.rgb";
	char writePath2[]="test-yuy2.yuv";
	readBmp(readPath);
	RGB24_TO_YV12(writePath);
	RGB24_TO_YUY2(writePath2);
	BMP_TO_RGB(BmptoRgb);
	printf("width=%d ,height=%d, biBitCount=%d\n",bmpWidth,bmpHeight,biBitCount);
	free(pBmpBuf);
	if(biBitCount==8)
	{
		free(pColorTable);
	}
	
	return 0;
}
