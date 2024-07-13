#ifndef __PICLIB_H
#define __PICLIB_H	  		  
#include "sys.h" 
#include "lcd.h"
#include "malloc.h"
#include "ff.h"
#include "exfuns.h"
#include "bmp.h"
#include "tjpgd.h"
#include "gif.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK MiniSTM32������
//ͼƬ���� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/3/14
//�汾��V2.0
//��Ȩ���У�����ؾ���
//Copyright(C) �������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//********************************************************************************
//����˵��
//V2.0
//1,��jpeg����⻻����TJPGD,֧�ָ����jpg/jpeg�ļ�,֧��С�ߴ�ͼƬ����jpeg��ʾ 
//2,pic_phy��������fillcolor����,���������ʾ,�����С�ߴ�jpgͼƬ����ʾ�ٶ�
//3,ai_load_picfile����,����һ������:fast,���������Ƿ�ʹ��jpeg/jpg������ʾ
//ע��:�����С�ߴ���ָ:jpg/jpegͼƬ�ߴ�С�ڵ���LCD�ߴ�.
//////////////////////////////////////////////////////////////////////////////////

#define PIC_FORMAT_ERR		0x27	//��ʽ����
#define PIC_SIZE_ERR		0x28	//ͼƬ�ߴ����
#define PIC_WINDOW_ERR		0x29	//�����趨����
#define PIC_MEM_ERR			0x11	//�ڴ����
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif 

//ͼƬ��ʾ������ӿ�  
//����ֲ��ʱ��,�������û��Լ�ʵ���⼸������
typedef struct 
{
	u16(*read_point)(u16,u16);				//u16 read_point(u16 x,u16 y)						���㺯��
	void(*draw_point)(u16,u16,u16);			//void draw_point(u16 x,u16 y,u16 color)		    ���㺯��
 	void(*fill)(u16,u16,u16,u16,u16);		///void fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color) ��ɫ��亯�� 	 
 	void(*draw_hline)(u16,u16,u16,u16);		//void draw_hline(u16 x0,u16 y0,u16 len,u16 color)  ��ˮƽ�ߺ���	 
 	void(*fillcolor)(u16,u16,u16,u16,u16*);	//void piclib_fill_color(u16 x,u16 y,u16 width,u16 height,u16 *color) ��ɫ���
}_pic_phy; 

extern _pic_phy pic_phy;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ͼ����Ϣ
typedef struct
{		
	u16 lcdwidth;	//LCD�Ŀ���
	u16 lcdheight;	//LCD�ĸ߶�
	u32 ImgWidth; 	//ͼ���ʵ�ʿ��Ⱥ͸߶�
	u32 ImgHeight;

	u32 Div_Fac;  	//����ϵ�� (������8192����)
	
	u32 S_Height; 	//�趨�ĸ߶ȺͿ���
	u32 S_Width;
	
	u32	S_XOFF;	  	//x���y���ƫ����
	u32 S_YOFF;

	u32 staticx; 	//��ǰ��ʾ���ģ�������
	u32 staticy;																 	
}_pic_info;
extern _pic_info picinfo;//ͼ����Ϣ
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void piclib_init(void);								//��ʼ����ͼ
u16 piclib_alpha_blend(u16 src,u16 dst,u8 alpha);	//alphablend����
void ai_draw_init(void);							//��ʼ�����ܻ�ͼ
u8 is_element_ok(u16 x,u16 y,u8 chg);				//�ж������Ƿ���Ч
u8 ai_load_picfile(const u8 *filename,u16 x,u16 y,u16 width,u16 height,u8 fast);//���ܻ�ͼ
u8 ai_load_jpg(const u8 *filename,u16 x,u16 y,u16 width,u16 height,u8 fast);//X
void Draw_Optionsbar(u8 Options_num ,u8 Options_Maxnum,u8  Options_DisplayMax);//����ѡ����
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif





























