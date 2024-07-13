#ifndef __TEXT_H__
#define __TEXT_H__	 
#include <stm32f10x.h>
#include "fontupd.h"
#include "malloc.h"
#include "ff.h"
#include "exfuns.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK MiniSTM32开发板
//汉字显示 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/3/14
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
					     
void Get_HzMat(unsigned char *code,unsigned char *mat,u8 size);			//得到汉字的点阵码
void Show_Font(u16 x,u16 y,u8 *font,u8 size,u8 mode);					//在指定位置显示一个汉字
void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode);	//在指定位置显示一个字符串 
void Show_Str_Mid(u16 x,u16 y,u8*str,u8 size,u8 len);
void Show_Str_rSTRLin(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode,u8 line_size);	//增加换行返回值
u8 Str_rSTRLin(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode,u8 line_size);		//读取每一页字符数
u8 process_from_txt(const u8 *filename);							//动态读取txt文件
u8 *Shorten_Long_Str(u8 *Str);											//长句短缩
u8 file_path_break(const u8 *filename);
#endif
