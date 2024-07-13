#include "mp3player.h"
#include "vs10xx.h"	 
#include "delay.h"
#include "led.h"
#include "key.h"
#include "lcd.h"		 
#include "malloc.h"
#include "text.h"
#include "string.h"
#include "exfuns.h"
#include "fattester.h"	 
#include "ff.h"   
#include "flac.h"	
#include "usart.h"	
#include "exti.h"

//显示曲目索引
//index:当前索引
//total:总文件数
u8 mp3_play_mode = MP3_Next_Flag;
u8 mp3_state_flag = MP3_Free_Flag;
void mp3_index_show(u16 index,u16 total)
{
	//显示当前曲目的索引,及总曲目数
	LCD_ShowxNum(40+0,250,index,3,16,0X80);		//索引
	LCD_ShowChar(40+24,250,'/',16,0);
	LCD_ShowxNum(40+32,250,total,3,16,0X80); 	//总曲目				  	  
}
//绘制进度条
void Draw_msg(u16 Options_num ,u16  Options_DisplayMax)
{
	u16 x;
	x=(Options_num*1.0)/(Options_DisplayMax)*200;	//选项条长度
	LCD_Fill(20, 217, 20 + x, 223, ForeColor); 
}
//显示当前音量
void mp3_vol_show(u8 vol)
{			    
	LCD_ShowString(40+110,250,200,16,16,"VOL:");	  	  
	LCD_ShowxNum(40+142,250,vol,2,16,0X80); 	//显示音量	 
}
u16 f_kbps=0;//歌曲文件位率	
//显示播放时间,比特率 信息 
//lenth:歌曲总长度
void mp3_msg_show(u32 lenth)
{	
	static u16 playtime=0;//播放时间标记	     
 	u16 time=0;// 时间变量
	u16 temp=0;	  
	if(f_kbps==0xffff)//未更新过
	{
		playtime=0;
		f_kbps=VS_Get_HeadInfo();//获得比特率
	}	 	 
	time=VS_Get_DecodeTime(); //得到解码时间

	if(playtime==0)playtime=time;
	else if((time!=playtime)&&(time!=0))//1s时间到,更新显示数据
	{
 		playtime=time;//更新时间 	 				    
 		temp=VS_Get_HeadInfo(); //获得比特率	   				 
		if(temp!=f_kbps)
		{
			f_kbps=temp;//更新KBPS	  				     
		}			 
		//显示播放时间			 
		LCD_ShowxNum(40,230,time/60,2,16,0X80);		//分钟
		LCD_ShowChar(40+16,230,':',16,0);
		LCD_ShowxNum(40+24,230,time%60,2,16,0X80);	//秒钟		
 		LCD_ShowChar(40+40,230,'/',16,0); 	
		//显示总时间
		if(f_kbps)time=(lenth/f_kbps)/125;//得到秒钟数   (文件长度(字节)/(1000/8)/比特率=持续秒钟数    	  
		else time=0;//非法位率	 
 		LCD_ShowxNum(40+48,230,time/60,2,16,0X80);	//分钟
		LCD_ShowChar(40+64,230,':',16,0);
		LCD_ShowxNum(40+72,230,time%60,2,16,0X80);	//秒钟	  		    
		//显示位率			   
   		LCD_ShowxNum(40+110,230,f_kbps,3,16,0X80); 	//显示位率	 
		LCD_ShowString(40+134,230,200,16,16,"Kbps");	  	  
 		LED1=!LED1;		//DS0翻转
		Draw_msg(playtime,time);
	}   		 
}			  		 

//
//播放一曲指定的歌曲	
//pname:歌曲路径+名字
//返回值:0,正常播放完成
//		 1,下一曲
//       2,上一曲
//       0XFF,出现错误了
u8 mp3_play_song(u8 *pname)
{	 
 	FIL* fmp3;
	u16 br;
	u8 res,rval;	  
	u8 *databuf;	   		   
	u16 i=0;     
	u8 pause=0;		//暂停标志 
	rval=0;  
	//UI部分
	POINT_COLOR = DemiCOlor;
	Show_Str(40,15,160,16,(u8 *)"音乐名：",16,0);
	if(mp3_play_mode == MP3_Next_Flag)
		Show_Str(150,15,160,16,(u8 *)"顺序播放",16,0);	//播放模式显示
	else
		Show_Str(150,15,160,16,(u8 *)"单曲循环",16,0);	
	LCD_DrawRectangle(20,80,220,210);
	LCD_Fill(18, 220, 222, 221,DemiCOlor);						//预先显示进度条母条
	POINT_COLOR = ForeColor;
	Show_Str(40,36,160,40,(u8 *)(pname+file_path_break(pname)+1),16,0);	//音乐文件名称
	
	
	//逻辑部分
	mp3_vol_show((vsset.mvol)/10);
	fmp3=(FIL*)mymalloc(sizeof(FIL));	//申请内存
	databuf=(u8*)mymalloc(4096);		//开辟4096字节的内存区域
	if(databuf==NULL||fmp3==NULL)rval=0XFF ;//内存申请失败.
	if(rval==0)
	{	  
	  VS_Restart_Play();  					//重启播放 
		VS_Set_All();        					//设置音量等信息 			 
		VS_Reset_DecodeTime();					//复位解码时间 	  
		res=f_typetell(pname);	 	 			//得到文件后缀	 			  	 						 
		if(res==0x4c)//如果是flac,加载patch
		{	
			VS_Load_Patch((u16*)vs1053b_patch,VS1053B_PATCHLEN);
		}  				 		   		 						  
		res=f_open(fmp3,(const TCHAR*)pname,FA_READ);//打开文件
		key = DISPLAY_PRES;
		//printf("sram :%d",mem_perused(0));
		if(res==0)//打开成功.
		{ 
			VS_SPI_SpeedHigh();	//高速						   
			while(rval==0)
			{
				res=f_read(fmp3,databuf,4096,(UINT*)&br);//读出4096个字节  
				i=0;
				do//主播放循环
			    {  	
					if((VS_Send_MusicData(databuf+i)==0)&&(pause==0))//给VS10XX发送音频数据
					{
						i+=32;
					}else   
					{
						if(key == DISPLAY_PRES)
						{
							mp3_msg_show(fmp3->fsize);//显示信息	
						}else if(key == KEY0_PRES)
						{
							mp3_state_flag=MP3_Next_Flag;
							f_close(fmp3);
							myfree(databuf);	  	 		  	    
							myfree(fmp3);
							return 0;
							
						}
						else if(key == KEY1_PRES)
						{
							mp3_state_flag=MP3_Pre_Flag;
							f_close(fmp3);
							myfree(databuf);	  	 		  	    
							myfree(fmp3);
							return 0;
							
						}
						else if(key == CLOSE_PRES)   //按键3长按，关闭文件
						{
							key = DISPLAY_PRES;
							mp3_state_flag=MP3_Free_Flag;
							rval=KEY0_PRES;
							break;
						}else if(key == WKUP_PRES)   //按键3单击，暂停播放
						{
							key = DISPLAY_PRES;
							pause=!pause;
						}
					}	    	    
				}while(i<4096);//循环发送4096个字节 
				if(br!=4096||res!=0)
				{
					rval=KEY0_PRES;
					mp3_state_flag=mp3_play_mode;	//下一首
					break;//读完了.	
				} 							 
			}
			f_close(fmp3);
		}else rval=0XFF;//出现错误	   	  
	}						     	 
	myfree(databuf);	  	 		  	    
	myfree(fmp3);
	return rval;	  	 		  	    
}




























