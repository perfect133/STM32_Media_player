#include "sys.h" 
//#include "fontupd.h"
//#include "flash.h"
#include "lcd.h"
#include "text.h"	
#include "string.h"			
#include "key.h"
#include "led.h"
#include "delay.h"
#include "exti.h"
#include "vs10XX.h"	
#include "mp3player.h"
/*
	����
1.������������
2.���ַ������
3.���ֲ���ģʽ����

*/
//extern u16 ForeColor;
//extern u16 DemiCOlor;
//extern u16 BackColor;
//���ƽ�����

void Draw_Menumsg(u16 Options_num ,u16  Options_DisplayMax)
{
	u16 x;
	LCD_Fill(18, 110, 222, 111,DemiCOlor);	
	x=(Options_num*1.0)/(Options_DisplayMax)*200;	//ѡ��������
	LCD_Fill(20, 107, 20 + x, 113, ForeColor); 
}

void mp3_Playmode()
{
	u8 OptionsFlag;
	extern u8 mp3_play_mode;
	LCD_Fill(20,20,240,320,BackColor);
	switch(mp3_play_mode)
	{
		case MP3_Next_Flag:OptionsFlag = 0;break;
		case MP3_WKUP_Flag:OptionsFlag = 1;break;
	}
	while(1)
	{
		if(key==DISPLAY_PRES)
		{
			LCD_Fill(40,30,240,50,BackColor);	
			Show_Str(40,30,240,16,"���ֲ���ģʽ:",16,0);
			if(0==OptionsFlag)
				LCD_Fill(40,60,240,79,BackColor);
			else if(1==OptionsFlag)
				LCD_Fill(40,80,240,99,BackColor);
			else if(2==OptionsFlag)
				LCD_Fill(40,100,240,119,BackColor);
			else if(3==OptionsFlag)
				LCD_Fill(40,120,240,139,BackColor);
			POINT_COLOR=ForeColor;
			BACK_COLOR=BackColor;
			Show_Str(40,60,240,16,"˳�򲥷�",16,0);
			Show_Str(40,80,240,16,"����ѭ��",16,0);
			Show_Str(40,100,240,16,"�������",16,0);
			Show_Str(40,120,240,16,"�б�ѭ��",16,0);
			if(0==OptionsFlag)
			{
				POINT_COLOR=BackColor;
				BACK_COLOR=ForeColor;
				Show_Str(40,60,240,16,"˳�򲥷�",16,0);
				POINT_COLOR=ForeColor;
				BACK_COLOR=BackColor;
			}else if(1==OptionsFlag)
			{
				POINT_COLOR=BackColor;
				BACK_COLOR=ForeColor;
				Show_Str(40,80,240,16,"����ѭ��",16,0);
				POINT_COLOR=ForeColor;
				BACK_COLOR=BackColor;
			}else if(2==OptionsFlag)
			{
				POINT_COLOR=BackColor;
				BACK_COLOR=ForeColor;
				Show_Str(40,100,240,16,"�������",16,0);
				POINT_COLOR=ForeColor;
				BACK_COLOR=BackColor;
			}else
			{
				POINT_COLOR=BackColor;
				BACK_COLOR=ForeColor;
				Show_Str(40,120,240,16,"�б�ѭ��",16,0);
				POINT_COLOR=ForeColor;
				BACK_COLOR=BackColor;
			}
			key=LEISURE_PRES;
		}
		if(key==KEY1_PRES)		//��һ��
		{
			if(OptionsFlag)OptionsFlag--;
			key=DISPLAY_PRES;
		}else if(key==KEY0_PRES)//��һ��
		{		   	
			if(OptionsFlag<3)OptionsFlag++;//ѡ������
			key=DISPLAY_PRES;
		}else if(key==WKUP_PRES)
		{
			key=DISPLAY_PRES;
			switch(OptionsFlag)
			{
				case 0:mp3_play_mode=MP3_Next_Flag;break;//��һ��
				case 1:mp3_play_mode=MP3_WKUP_Flag;break;//ѭ������
				case 2:mp3_play_mode=MP3_Next_Flag;break;//�������
				case 3:mp3_play_mode=MP3_Next_Flag;break;//��ʱ�޷�ʵ��
			}
			
			LCD_Clear(BackColor);
			POINT_COLOR=ForeColor;
			BACK_COLOR=BackColor; 
		}else if(key==CLOSE_PRES) {key=DISPLAY_PRES;break;}
		delay_ms(10);
	}
}

void mp3_Sysvolume()
{
	LCD_Fill(40,30,240,50,BackColor);	
	Show_Str(40,30,240,16,"������������:",16,0);
	while(1)
	{
	if(key==DISPLAY_PRES)
		{
			LCD_Fill(40,80,240,300,BackColor);	
			Draw_Menumsg(vsset.mvol/10,25);
			key=LEISURE_PRES;
		}
		
		if(key==KEY1_PRES)		//������С
		{
			if(vsset.mvol>0)vsset.mvol-=10;
			key=DISPLAY_PRES;
		}else if(key==KEY0_PRES)//��������
		{		   	
			if(vsset.mvol<250)vsset.mvol+=10;//��������
			key=DISPLAY_PRES;
		}else if(key==CLOSE_PRES) {key=DISPLAY_PRES;break;}
		delay_ms(10);
	}
}
void TXT_SpeedSys()
{
	extern u16 TXT_Speed;
	LCD_Fill(20,20,240,320,BackColor);
	Show_Str(40,30,240,16,"�ı���ҳ�������:",16,0);
	while(1)
	{
	if(key==DISPLAY_PRES)
		{
			LCD_Fill(40,80,240,300,BackColor);	
			Draw_Menumsg(TXT_Speed/50,12);
			key=LEISURE_PRES;
		}
		
		if(key==KEY1_PRES)		
		{
			if(TXT_Speed>50) TXT_Speed-=50;
			key=DISPLAY_PRES;
		}else if(key==KEY0_PRES)
		{		   	
			if(TXT_Speed<650)TXT_Speed+=50;
			key=DISPLAY_PRES;
		}else if(key==CLOSE_PRES) {key=DISPLAY_PRES;break;}
		delay_ms(10);
	}
}
void ColorStyle()
{
	u8 OptionsFlag = 0;
	LCD_Fill(20,20,240,320,BackColor);
	while(1)
	{
		if(key==DISPLAY_PRES)
		{
			LCD_Fill(40,30,240,50,BackColor);	
			Show_Str(40,30,240,16,"���ַ������:",16,0);
			if(0==OptionsFlag)
				LCD_Fill(40,60,240,79,BackColor);
			else if(1==OptionsFlag)
				LCD_Fill(40,80,240,99,BackColor);
			else if(2==OptionsFlag)
				LCD_Fill(40,100,240,119,BackColor);
			else if(3==OptionsFlag)
				LCD_Fill(40,120,240,139,BackColor);
//			jpg_decode_new();
			POINT_COLOR=ForeColor;
			BACK_COLOR=BackColor;
			Show_Str(40,60,240,16,"����׺�",16,0);
			Show_Str(40,80,240,16,"������ɫ",16,0);
			Show_Str(40,100,240,16,"����ůɫ",16,0);
			Show_Str(40,120,240,16,"ҹ��ڰ�",16,0);
			if(0==OptionsFlag)
			{
				POINT_COLOR=BackColor;
				BACK_COLOR=ForeColor;
				Show_Str(40,60,240,16,"����׺�",16,0);
				POINT_COLOR=ForeColor;
				BACK_COLOR=BackColor;
			}else if(1==OptionsFlag)
			{
				POINT_COLOR=BackColor;
				BACK_COLOR=ForeColor;
				Show_Str(40,80,240,16,"������ɫ",16,0);
				POINT_COLOR=ForeColor;
				BACK_COLOR=BackColor;
			}else if(2==OptionsFlag)
			{
				POINT_COLOR=BackColor;
				BACK_COLOR=ForeColor;
				Show_Str(40,100,240,16,"����ůɫ",16,0);
				POINT_COLOR=ForeColor;
				BACK_COLOR=BackColor;
			}else
			{
				POINT_COLOR=BackColor;
				BACK_COLOR=ForeColor;
				Show_Str(40,120,240,16,"ҹ��ڰ�",16,0);
				POINT_COLOR=ForeColor;
				BACK_COLOR=BackColor;
			}
			key=LEISURE_PRES;
		}
		if(key==KEY1_PRES)		//��һ��
		{
			if(OptionsFlag)OptionsFlag--;
			key=DISPLAY_PRES;
		}else if(key==KEY0_PRES)//��һ��
		{		   	
			if(OptionsFlag<3)OptionsFlag++;//��ĩβ��ʱ��,�Զ���ͷ��ʼ
			key=DISPLAY_PRES;
		}else if(key==WKUP_PRES)
		{
			key=DISPLAY_PRES;
			switch(OptionsFlag)
			{
				case 0:ForeColor=BLACK;BackColor=WHITE;DemiCOlor=GRAY;break;
				case 1:ForeColor=GREEN;BackColor=0xFFFC;DemiCOlor=GBLUE;break;
				case 2:ForeColor=BRRED;BackColor=0xFFFC;DemiCOlor=YELLOW;break;
				case 3:ForeColor=WHITE;BackColor=BLACK;DemiCOlor=GRAY;break;
			}
			
			LCD_Clear(BackColor);
			POINT_COLOR=ForeColor;
			BACK_COLOR=BackColor; 
		}else if(key==CLOSE_PRES) {key=DISPLAY_PRES;break;}
		delay_ms(10);
	}
}
void System_Funtion()	//����ѡ��
{
	u8 OptionsFlag = 0;
	LCD_Clear(BackColor);
	while(1)
	{
		if(key==DISPLAY_PRES)
		{
			if(0==OptionsFlag)
				LCD_Fill(40,60,240,79,BackColor);
			else if(1==OptionsFlag)
				LCD_Fill(40,80,240,99,BackColor);
			else if(2==OptionsFlag)
				LCD_Fill(40,100,240,119,BackColor);
			else if(3==OptionsFlag)
				LCD_Fill(40,120,240,139,BackColor);
			POINT_COLOR=ForeColor;
			BACK_COLOR=BackColor;
			Show_Str(40,60,240,16,"������������",16,0);
			Show_Str(40,80,240,16,"���ַ������",16,0);
			Show_Str(40,100,240,16,"���ֲ���ģʽ����",16,0);
			Show_Str(40,120,240,16,"С˵��ҳ�������",16,0);
			if(0==OptionsFlag)
			{
				POINT_COLOR=BackColor;
				BACK_COLOR=ForeColor;
				Show_Str(40,60,240,16,"������������",16,0);
				POINT_COLOR=ForeColor;
				BACK_COLOR=BackColor;
			}else if(1==OptionsFlag)
			{
				POINT_COLOR=BackColor;
				BACK_COLOR=ForeColor;
				Show_Str(40,80,240,16,"���ַ������",16,0);
				POINT_COLOR=ForeColor;
				BACK_COLOR=BackColor;
			}else if(2==OptionsFlag)
			{
				POINT_COLOR=BackColor;
				BACK_COLOR=ForeColor;
				Show_Str(40,100,240,16,"���ֲ���ģʽ����",16,0);
				POINT_COLOR=ForeColor;
				BACK_COLOR=BackColor;
			}else
			{
				POINT_COLOR=BackColor;
				BACK_COLOR=ForeColor;
				Show_Str(40,120,240,16,"С˵��ҳ�������",16,0);
				POINT_COLOR=ForeColor;
				BACK_COLOR=BackColor;
			}
			key=LEISURE_PRES;
		}
		
		if(key==KEY1_PRES)		//��һ��
		{
			if(OptionsFlag)OptionsFlag--;
			key=DISPLAY_PRES;
		}else if(key==KEY0_PRES)//��һ��
		{		   	
			if(OptionsFlag<3)OptionsFlag++;//��ĩβ��ʱ��,�Զ���ͷ��ʼ
			key=DISPLAY_PRES;
		}else if(key==WKUP_PRES)
		{
			key=DISPLAY_PRES;
			switch(OptionsFlag)
			{
				case 0:mp3_Sysvolume();break;
				case 1:ColorStyle();break;
				case 2:mp3_Playmode();break;
				case 3:TXT_SpeedSys();break;
			}
			//load region LR_IROM1 size (262156 bytes) exceeds limit .Region contains 33bytes of padding and 0 bytes of veneers (total 33 bytes of linker generated content)
				
			LCD_Clear(BackColor);
			POINT_COLOR=ForeColor;
			BACK_COLOR=BackColor; 
		}else if(key==CLOSE_PRES) {key=DISPLAY_PRES;break;}
		delay_ms(10);
	}
}
