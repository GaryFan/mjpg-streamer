
#ifndef __motion_H__
#define __motion_H__


/*yuv4:2:2��ʽת��Ϊrgb24��ʽ*/
//int convert_yuv_to_rgb_pixel(int y, int u, int v);

int convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width,unsigned int height);
//yuv ת��Ϊ�Ҷ�ͼ��
int convert_yuv_to_gray(unsigned char *yuv, unsigned char *rgb, unsigned int width,unsigned int height);
//yuv ת��Ϊ�Ҷ�ͼ�� ��ͨ��
int convert_yuv_to_gray0(unsigned char *yuv, unsigned char *gray, unsigned int width,unsigned int height);

int motion_init( unsigned int width, unsigned int height );
int motion_destroy( void );
//3*3 ��ʴ
void erosion( unsigned char *img, unsigned int width, unsigned int height );
//3*3 ����
void dilation( unsigned char *img, unsigned int width, unsigned int height );
//��ʶ�仯����ͬʱ��������һ����������ÿ�θ��¶�Ҫ�����λ�ü�1 ����־λ�ü�Ϊ0ʱ��˵����λ���Ѿ������˶�
//��������ʱ��ܿ죬�����ɫ������������������ƽ���˶�ʱ�������ʹ�ü�������ʽ������ɼ�⵽���˶���λֻ�����뽻������
//Ϊ�˿˷���������⣬ʹ���ӳټ�����ʽ����֤�ܱ�ʶ�������ı仯����
int pic_mark( unsigned char *motion, unsigned char *mark, unsigned int size );
//��ʶ���仯����Ŀǰ��ͨ���ı�仯��������Ƚ��б�ʶ
void pic_add( unsigned char *rgbimg, unsigned char *mark, unsigned int size );
  
//ͼ��������㣬���ر仯�����ظ���,����洢��img1��
int pic_subtraction( unsigned char *img1, unsigned char *img2, unsigned int size );
//ͼ���ֵ������С��noise ������0 ����noise ������ 255�����ر仯���ظ���
int pic_binmap( unsigned char *img, unsigned char *binimg, unsigned char noise, unsigned int size );
int motion_check( unsigned char *yuv, unsigned char *out, unsigned int width, unsigned int height );
void param_init( int noise, int threshold );
#endif


