#ifndef __MP3PLAYER_H
#define __MP3PLAYER_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//MP3播放驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/3/25
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved								  						    								  
//////////////////////////////////////////////////////////////////////////////////

#define MP3_Free_Flag	0
#define MP3_Ready_Flag	1
#define MP3_Next_Flag	2
#define MP3_Pre_Flag	3
#define MP3_WKUP_Flag	4


//void mp3_play(void);
u16 mp3_get_tnum(u8 *path);
u8 mp3_play_song(u8 *pname);
void mp3_index_show(u16 index,u16 total);
void mp3_msg_show(u32 lenth);
#endif












