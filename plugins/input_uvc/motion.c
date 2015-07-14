
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "alg.h"
#include "draw.h"
#define uchar unsigned char 
#define uint unsigned int



typedef struct SMOTION{
	uchar *background;//�洢��ͨ���ĻҶ�ͼ
	uchar *motion;//�˶�ͼ��ĳ������ֵ��Ϊ0˵�����˶�����
	uchar *binpic;//��ֵ��ͼ��
	uchar noise;//����ֵ
	uchar flag;//ͼ���Ƿ�仯�ı�־
	uint  threshold;//��ֵ
	uint  width;
	uint  height;
	uint  timecount;//�˶����ּ�ʱ��
}SMOTION;

#define MAX_TIMECOUNT 10
#define BRIGHTNESS      200 //���ȵ���ֵ

static int  MAX_NOISE=20;  //��������ֵ
static int  MAX_THRESHOLD=100;
static unsigned char *gBuf;//��bufƽ�������Σ�SMOTION�ṹ�е�background��motion��motion��ָ̬��ĳһ��
static SMOTION motion;
static int flag_init = -1;//��ʼ����־ -1δ��ʼ�� other ��ʼ��

#define MAX2(x, y) ((x) > (y) ? (x) : (y))
#define MAX3(x, y, z) ((x) > (y) ? ((x) > (z) ? (x) : (z)) : ((y) > (z) ? (y) : (z)))
#define NORM               100
#define ABS(x)             ((x) < 0 ? -(x) : (x))
#define DIFF(x, y)         (ABS((x) - (y)))
#define NDIFF(x, y)        (ABS(x) * NORM/(ABS(x) + 2 * DIFF(x,y)))

/*yuv4:2:2��ʽת��Ϊrgb24��ʽ*/
/*
int convert_yuv_to_rgb_pixel(int y, int u, int v)
{
	uint pixel32 = 0;
	uchar *pixel = (uchar *)&pixel32;
	int r, g, b;
	r = y + (1.370705 * (v-128));
	g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
	b = y + (1.732446 * (u-128));
	if(r > 255) r = 255;
	if(g > 255) g = 255;
	if(b > 255) b = 255;
	if(r < 0) r = 0;
	if(g < 0) g = 0;
	if(b < 0) b = 0;
	pixel[0] = r * 220 / 256;
	pixel[1] = g * 220 / 256;
	pixel[2] = b * 220 / 256;
	
	return pixel32;
}*/
/*
 * yuv4:2:2��ʽת��Ϊrgb24��ʽ
 * ���ڸ��������ٶ�̫�������ԣ��˴�ʹ�ý���ֵ����
 */
 /*
int convert_yuv_to_rgb_pixel(int y, int u, int v)
{	  
	uint pixel32 = 0;
	uchar *pixel = (uchar *)&pixel32;
	int r, g, b;
	r = (y + (359 * v)) >> 8;
	g = (y - (88 * u) - (183 * v)) >> 8;
	b = (y + (454 * u)) >> 8;

	pixel[0] = (r > 255) ? 255 : ((r < 0) ? 0 : r);
	pixel[1] = (g > 255) ? 255 : ((g < 0) ? 0 : g);
	pixel[2] = (b > 255) ? 255 : ((b < 0) ? 0 : b);
	
	return pixel32;
}
*/


int convert_yuv_to_rgb_buffer(uchar *yuv, uchar *rgb, uint width,uint height)
{
	int x,z=0;

	for (x = 0; x < width * height; x++) {
		int r, g, b;
		int y, u, v;

		if(!z) {
			y = yuv[0] << 8;
		} else {
			y = yuv[2] << 8;
		}
		
		u = yuv[1] - 128;
		v = yuv[3] - 128;

		r = (y + (359 * v)) >> 8;
		g = (y - (88 * u) - (183 * v)) >> 8;
		b = (y + (454 * u)) >> 8;

		*(rgb++) = (r > 255) ? 255 : ((r < 0) ? 0 : r);
		*(rgb++) = (g > 255) ? 255 : ((g < 0) ? 0 : g);
		*(rgb++) = (b > 255) ? 255 : ((b < 0) ? 0 : b);

		if(z++) {
			z = 0;
			yuv += 4;
		}
	}
	return 0;
}
//yuv ת��Ϊ�Ҷ�ͼ��
int convert_yuv_to_gray(uchar *yuv, uchar *rgb, uint width,uint height)
{
	uint in, out = 0;
	int y0, y1;
	for(in = 0; in < width * height * 2; in += 4) {
		y0 = yuv[in + 0];
		y1 = yuv[in + 2];

		rgb[out++] = y0;
		rgb[out++] = y0;
		rgb[out++] = y0;//rgb��һ������
		
		rgb[out++] = y1;
		rgb[out++] = y1;
		rgb[out++] = y1;
	}
	return 0;
}
//yuv ת��Ϊ�Ҷ�ͼ�� ��ͨ��
int convert_yuv_to_gray0(uchar *yuv, uchar *gray, uint width,uint height)
{
	uint in, out = 0;
	int y0, y1;
	for(in = 0; in < width * height * 2; in += 4) {

		y0 = yuv[in + 0];
		y1 = yuv[in + 2];

		gray[out++] = y0;
		gray[out++] = y1;
	}
	return 0;
}

int motion_init( uint width, uint height )
{
	memset( &motion, 0, sizeof(motion));
	gBuf = (uchar*)calloc( 1, 3*width * height );
	if( gBuf == NULL)
	{
		printf("no enough mem ,malloc fail!\n");
		return -1;
	}

	motion.background = gBuf;
	motion.motion = gBuf + width * height;
	motion.binpic = gBuf + 2 * width * height;

	motion.flag = 0;
	motion.timecount = MAX_TIMECOUNT;
	motion.threshold = MAX_THRESHOLD;
	motion.noise = MAX_NOISE;
	return 0;
}
int motion_destroy( void )
{
	if( gBuf )	
	{
		free( gBuf );
		gBuf = NULL;
		motion.background = NULL;
		motion.binpic = NULL;
		motion.motion = NULL;
	}

	return 0;
}
/* Erodes a 3x3 box */
//3*3 ��ʴ
static int erode9(unsigned char *img, int width, int height, void *buffer, unsigned char flag)
{    
	int y, i, sum = 0;    
	char *Row1,*Row2,*Row3;    
	Row1 = buffer;    
	Row2 = Row1 + width;    
	Row3 = Row1 + 2*width;    
	memset(Row2, flag, width);    
	memcpy(Row3, img, width);    
	for (y = 0; y < height; y++) {        
		memcpy(Row1, Row2, width);        
		memcpy(Row2, Row3, width);        
		if (y == height - 1)            
			memset(Row3, flag, width);        
		else            
			memcpy(Row3, img+(y + 1) * width, width);        
		for (i = width-2; i >= 1; i--) {            
			if (Row1[i-1] == 0 ||                
				Row1[i]   == 0 ||                
				Row1[i+1] == 0 ||                
				Row2[i-1] == 0 ||                
				Row2[i]   == 0 ||                
				Row2[i+1] == 0 ||                
				Row3[i-1] == 0 ||                
				Row3[i]   == 0 ||                
				Row3[i+1] == 0)                
				img[y * width + i] = 0;            
			else                
				sum++;        
		}        
		img[y * width] = img[y * width + width - 1] = flag;    
	}    
	return sum;
}

//3*3 ����
/* Dilates a 3x3 box */
static int dilate9(unsigned char *img, int width, int height, void *buffer)
{    
	/* - row1, row2 and row3 represent lines in the temporary buffer      
	 * - window is a sliding window containing max values of the columns     
	 *   in the 3x3 matrix     
	 * - widx is an index into the sliding window (this is faster than      
	 *   doing modulo 3 on i)     
	 * - blob keeps the current max value     
	 */    
	int y, i, sum = 0, widx;    
	unsigned char *row1, *row2, *row3, *rowTemp,*yp;    
	unsigned char window[3], blob, latest;    
	/* Set up row pointers in the temporary buffer. */    
	row1 = buffer;    
	row2 = row1 + width;    
	row3 = row2 + width;    
	/* Init rows 2 and 3. */    
	memset(row2, 0, width);    
	memcpy(row3, img, width);    
	/* Pointer to the current row in img. */    
	yp = img;        
	for (y = 0; y < height; y++) {        
		/* Move down one step; row 1 becomes the previous row 2 and so on. */
		rowTemp = row1;        
		row1 = row2;        
		row2 = row3;        
		row3 = rowTemp;        
		/* If we're at the last row, fill with zeros, otherwise copy from img. */        
		if (y == height - 1)            
			memset(row3, 0, width);        
		else            
			memcpy(row3, yp + width, width);                
		/* Init slots 0 and 1 in the moving window. */        
		window[0] = MAX3(row1[0], row2[0], row3[0]);        
		window[1] = MAX3(row1[1], row2[1], row3[1]);        
		/* Init blob to the current max, and set window index. */        
		blob = MAX2(window[0], window[1]);        
		widx = 2;        
		/* Iterate over the current row; index i is off by one to eliminate 
		 * a lot of +1es in the loop.         
		 */        
		for (i = 2; i <= width - 1; i++) {            
			/* Get the max value of the next column in the 3x3 matrix. */   
			latest = window[widx] = MAX3(row1[i], row2[i], row3[i]);    
			/* If the value is larger than the current max, use it.	Otherwise,             
			* calculate a new max (because the new value may not be the max.          
			*/            
			if (latest >= blob)                
				blob = latest;            
			else                
				blob = MAX3(window[0], window[1], window[2]);            
			/* Write the max value (blob) to the image. */            
			if (blob != 0) {                
				*(yp + i - 1) = blob;                
				sum++;            
			}            
			/* Wrap around the window index if necessary. */            
			if (++widx == 3)                
				widx = 0;        
		}        
		/* Store zeros in the vertical sides. */        
		*yp = *(yp + width - 1) = 0;        
		yp += width;    
	}        
	return sum;
}

//��ʶ�仯����ͬʱ��������һ����������ÿ�θ��¶�Ҫ�����λ�ü�1 ����־λ�ü�Ϊ0ʱ��˵����λ���Ѿ������˶�
//��������ʱ��ܿ죬�����ɫ������������������ƽ���˶�ʱ�������ʹ�ü�������ʽ������ɼ�⵽���˶���λֻ�����뽻������
//Ϊ�˿˷���������⣬ʹ���ӳټ�����ʽ����֤�ܱ�ʶ�������ı仯����
int pic_mark( unsigned char *motion, unsigned char *mark, unsigned int size )
{
    int i=0;
	int cnt = 0;
    
	while( i != size ) {
	    if(motion[i] > 0) motion[i]--;
	    if( mark ) {
		    if( mark[i] ) {
			    cnt++;
			    motion[i] = BRIGHTNESS;
		    }
		}
		i++;
	}

	return cnt;
}
//��ʶ���仯����Ŀǰ��ͨ���ı�仯��������Ƚ��б�ʶ
void pic_add( unsigned char *rgbimg, unsigned char *mark, unsigned int size )
{
    int i=0;
    int tmp;
    
	while( i != size ) {
		if( mark[i] ) {
		    tmp = rgbimg[i*3] + mark[i]<<1;
	        rgbimg[i*3] = tmp>255?255:tmp;
		}
		i++;
	}
}
  
//ͼ��������㣬���ر仯�����ظ���,����洢��img1��
int pic_subtraction( unsigned char *img1, unsigned char *img2, unsigned int size )
{
	int i=0;
	//int cnt = 0;

	while( i != size ) {
		//����ֵ���ܳ��ַ�ת�������ֵ
		img2[i] = img1[i] > img2[i]?img1[i] - img2[i]:img2[i] - img1[i];
		i++;
	}

	return 0;
}
//ͼ���ֵ������С��noise ������0 ����noise ������ 255�����ر仯���ظ���
int pic_binmap( unsigned char *img, unsigned char *binimg, unsigned char noise, unsigned int size )
{
	int i=0;
	int cnt = 0;

	while( i != size ) {
		binimg[i] = img[i] > noise?255:0;
		if( binimg[i] )
			cnt++;
		i++;
	}

	return cnt;
}
//ͼ���ֵ������С��noise ������0 ����noise ������ 255�����ر仯���ظ���
int pic_noise_filter( unsigned char *img, unsigned char noise, unsigned int size )
{
	int i=0;
	int cnt = 0;

	while( i != size ) {
		if( img[i] < noise ) {
			img[i] = 0;
		} else {
			cnt++;
		}
		i++;
	}

	return cnt;
}
int noise_tune( unsigned char *img, unsigned int width, unsigned int height )
{
	long sum = 0;
	int cnt = 0;
	int i;
	unsigned char pmax,pmin;

	pmax = pmin = 0;
	for(i=0;i!=width*height;i++)
	{
		if( img[i] ) {
			cnt++;
			sum += img[i];
			pmax = img[i] > pmax?img[i]:pmax;
		}		
	}

	int avg = sum / cnt;

	printf("pmax = %d cnt = %d avg=%d\n",pmax,cnt,avg);
	printf("+++++++++++++++++++++++++\n");

	return avg;
}
void param_init( int noise, int threshold )
{
	if( noise > 0 )
		MAX_NOISE = noise;
	if( threshold > 0 )
		MAX_THRESHOLD = threshold;
}
int motion_check( unsigned char *yuv, unsigned char *out, unsigned int width, unsigned int height )
{
	int change = 0;
	
	if( 0 > flag_init ) {
		if( !motion_init( width , height ) ) {
			flag_init = 0;//��ʼ�����
			motion.width = width;
			motion.height = height;
			printf("motion.background saved\n");
			initialize_chars();
			//���汳��ͼƬ
			convert_yuv_to_gray0( yuv, motion.background, width, height );
			return 0;
		} else {
			printf("motion init fail!\n");
			return -1;
		}
	}
	convert_yuv_to_rgb_buffer( yuv, out, width, height);
	convert_yuv_to_gray0( yuv, motion.motion, width, height );
	pic_subtraction( motion.motion, motion.background, width * height );
	
	uchar *tmp = motion.background;
	motion.background = motion.motion;//���±���
	motion.motion = tmp;
	char timebuf[50]={0};
	mystrtime( timebuf, "%Y-%m-%d %H:%M:%S" );

	draw_text( out, width-strlen(timebuf), height-10, width, timebuf, 0 );
	
	//�˶��������
	//pic_mark( motion.motion, NULL, width * height );
	//noise_tune( motion.motion, width, height);
	change = pic_noise_filter( motion.motion, motion.noise, width * height );
	if( change > motion.threshold ) {//���ȥ��������ı�����Ȼ�����˶������ֵ�������һ������
		struct coord crd;
		unsigned char *tmpbuf;
		tmpbuf = (unsigned char *)calloc(1,width*3);
		
		printf("-------------------------------\n");
		printf("noise=%d\n",motion.noise);
		printf("change=%d\n",change);
		printf("threshold=%d\n",motion.threshold);
		//��ʴ����
		printf("erode9=%d\n",erode9( motion.motion, width, height, tmpbuf, 0 ));
		//���ʹ���
		printf("dilate9=%d\n",dilate9( motion.motion, width, height, tmpbuf ));
		alg_locate_center_size( motion.motion, width, height,&crd );
		alg_draw_location( &crd, out, width, 1 );
		//��ʶ���仯����
		//pic_add( out, motion.motion, width * height );
		free(tmpbuf);
		printf("===================\n");
		return 0xaa;//��⵽�˶�
	}

	
	return 0;
}

