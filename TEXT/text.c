#include "sys.h" 
#include "fontupd.h"
#include "flash.h"
#include "lcd.h"
#include "text.h"	
#include "string.h"			
#include "key.h"
#include "led.h"
#include "delay.h"
#include "exti.h"

#define TXT_FLAG 1			//txt文件读取		
#define TXT_Test_Title_name 0
//code 字符指针开始
//从字库中查找出字模
//code 字符串的开始地址,GBK码
//mat  数据存放地址 (size/8+((size%8)?1:0))*(size) bytes大小	
//size:字体大小
u16 TXT_Speed=300;
void Get_HzMat(unsigned char *code,unsigned char *mat,u8 size)
{		    
	unsigned char qh,ql;
	unsigned char i;					  
	unsigned long foffset; 
	u8 csize=(size/8+((size%8)?1:0))*(size);//得到字体一个字符对应点阵集所占的字节数	 
	qh=*code;
	ql=*(++code);
	if(qh<0x81||ql<0x40||ql==0xff||qh==0xff)//非 常用汉字
	{   		    
	    for(i=0;i<csize;i++)*mat++=0x00;//填充满格
	    return; //结束访问
	}          
	if(ql<0x7f)ql-=0x40;//注意!
	else ql-=0x41;
	qh-=0x81;   
	foffset=((unsigned long)190*qh+ql)*csize;	//得到字库中的字节偏移量  		  
	switch(size)
	{
		case 12:
			SPI_Flash_Read(mat,foffset+ftinfo.f12addr,24);
			break;
		case 16:
			SPI_Flash_Read(mat,foffset+ftinfo.f16addr,32);
			break;
		case 24:
			SPI_Flash_Read(mat,foffset+ftinfo.f24addr,72);
			break;
			
	}     												    
}  
//显示一个指定大小的汉字
//x,y :汉字的坐标
//font:汉字GBK码
//size:字体大小
//mode:0,正常显示,1,叠加显示	   
void Show_Font(u16 x,u16 y,u8 *font,u8 size,u8 mode)
{
	u8 temp,t,t1;
	u16 y0=y;
	u8 dzk[72];   
	u8 csize=(size/8+((size%8)?1:0))*(size);//得到字体一个字符对应点阵集所占的字节数	 
	if(size!=12&&size!=16&&size!=24)return;	//不支持的size
	Get_HzMat(font,dzk,size);	//得到相应大小的点阵数据 
	for(t=0;t<csize;t++)
	{   												   
		temp=dzk[t];			//得到点阵数据                          
		for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
			else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR); 
			temp<<=1;
			y++;
			if((y-y0)==size)
			{
				y=y0;
				x++;
				break;
			}
		}  	 
	}  
}
//在指定位置开始显示一个字符串	    
//支持自动换行
//(x,y):起始坐标
//width,height:区域
//str  :字符串
//size :字体大小
//mode:0,非叠加方式;1,叠加方式    	   		   
void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode)
{					
	u16 x0=x;
	u16 y0=y;							  	  
    u8 bHz=0;     //字符或者中文  	    				    				  	  
    while(*str!=0)//数据未结束
    { 
        if(!bHz)
        {
	        if(*str>0x80)bHz=1;//中文 
	        else              //字符
	        {      
                if(x>(x0+width-size/2))//换行
				{				   
					y+=size+4;
					x=x0;	   
				}							    
		        if(y>(y0+height-size))break;//越界返回      
		        if(*str=='\n')//换行符号
		        {         
		            y+=size+4;
					x=x0;
		            str++; 
		        }  
		        else LCD_ShowChar(x,y,*str,size,mode);//有效部分写入 
				str++; 
		        x+=size/2; //字符,为全字的一半 
	        }
        }else//中文 
        {     
            bHz=0;//有汉字库    
            if(x>(x0+width-size))//换行
			{	    
				y+=size+4;
				x=x0;		  
			}
	        if(y>(y0+height-size))break;//越界返回  						     
	        Show_Font(x,y,str,size,mode); //显示这个汉字,空心显示 
	        str+=2; 
	        x+=size;//下一个汉字偏移	    
        }						 
    }   
}  	


/*
对文本进行显示解析
line_size:文本行间隔
返回值:无法显示的剩余字节数
								*/
u8 Str_rSTRLin(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode,u8 line_size)
{					
	u16 x0=x;
	u16 y0=y;							  	  
    u8 bHz=0;     //字符或者中文  
    while(*str!=0)//数据未结束
    { 
        if(!bHz)
        {
	        if(*str>0x80)bHz=1;//中文 
	        else              //字符
	        {      
                if(x>(x0+width-size/2))//换行
				{				   
					y+=size+line_size;
					x=x0;	 				
				}							    
		        if(y>(y0+height-size))
				{
					if(*str!='\r')					//兼容windowsCRLF格式
						str++;
					if(*str!='\n')
						str++;
					return strlen((char *)str);//越界返回 
				}      
				if(*str=='\n')//换行符号
		        {         
		            y+=size+line_size;
					x=x0;
		            str++; 
		        }  
		        else if(*str=='\r')
				{
					y+=size+line_size;
					x=x0;
					str++; 
				}
				str++; 
		        x+=size/2; //字符,为全字的一半 
	        }
        }else//中文 
        {     
            bHz=0;//有汉字库    
            if(x>(x0+width-size))//换行
			{	    
				y+=size+line_size;
				x=x0;					
			}
	        if(y>(y0+height-size))
			{
				if(*str!='\r')
					str++;
				if(*str!='\n')
					str++;
				return strlen((char *)str);//越界返回 
			}  
	        str+=2; 
	        x+=size;//下一个汉字偏移	    
        }						 
    }   
	return 0;
}
/*
对文本进行解析显示
line_size:文本行间隔
中文自动换行
*/
void Show_Str_rSTRLin(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode,u8 line_size)
{					
	u16 x0=x;
	u16 y0=y;							  	  
    u8 bHz=0;     //字符或者中文  
    while(*str!=0)//数据未结束
    { 
        if(!bHz)
        {
	        if(*str>0x80)bHz=1;//中文 
	        else              //字符
	        {      
                if(x>(x0+width-size/2))//换行
				{				   
					y+=size+line_size;
					x=x0;	 				
				}							    
		        if(y>(y0+height-size)) 
				{
					if(*str!='\r')
						str++;
					if(*str!='\n')
						str++;
					return ;//越界返回   
				}
		        if(*str=='\n')//换行符号
		        {         
		            y+=size+line_size;
					x=x0;
		            str++; 
		        }  
		        else if(*str=='\r')
				{
					y+=size+line_size;
					x=x0;
					str++; 
				}
				else LCD_ShowChar(x,y,*str,size,mode);//有效部分写入 
				str++; 
		        x+=size/2; //字符,为全字的一半 
	        }
        }else//中文 
        {     
            bHz=0;//有汉字库    
            if(x>(x0+width-size))//换行
			{	    
				y+=size+line_size;
				x=x0;					
			}
	        if(y>(y0+height-size))
			{
				if(*str!='\r')
					str++;
				if(*str!='\n')
					str++;
				return ;//越界返回   
			}  		     
	        Show_Font(x,y,str,size,mode); //显示这个汉字,空心显示 
	        str+=2; 
	        x+=size;//下一个汉字偏移	    
        }						 
    }   
	return ;
}
//在指定宽度的中间显示字符串
//如果字符长度超过了len,则用Show_Str显示
//len:指定要显示的宽度			  
void Show_Str_Mid(u16 x,u16 y,u8*str,u8 size,u8 len)
{
	u16 strlenth=0;
   	strlenth=strlen((const char*)str);
	strlenth*=size/2;
	if(strlenth>len)Show_Str(x,y,lcddev.width,lcddev.height,str,size,1);
	else
	{
		strlenth=(len-strlenth)/2;
	    Show_Str(strlenth+x,y,lcddev.width,lcddev.height,str,size,1);
	}
}   




#if TXT_FLAG
u8 file_path_break(const u8 *filename)
{
	u8 i;
	u8 right_flag=0;
	for(i=0;*(filename+i);i++)
	{
		if(*(filename+i)=='/')
			right_flag=i;
	}
	return right_flag;
}
u8 Chinese2Char(u8 * buff, u16 size) //检测字符串最后一个字节是否是中文第一个字节，
{                                   //是返回0，不是返回1
    u16 i = 0;
    while(i != size - 1 && i != size -2) //顺序检测，直到最后剩下一个或两个字节
    {
        if(*buff < 128)  {buff++;i++;}
        else  {buff += 2;i += 2;}
    }
    if(i == size - 2)      //剩下最后两个字节
    {
        if(*buff > 128)  return 0;  
        else
        {
            buff++;
            if(*buff > 128)  return 1;
            else  return 0;
        }
    }
    if(i == size - 1)  //剩下最后一个字节
    {
        if(*buff > 128)  return 1;
        else  return 0;
    }
    return 0;
}

u8 *Shorten_Long_Str(u8 *Str)
{
	u8 StrCopy[50],*p;
	u8 Len=strlen((char *)Str),Len_txt=0;
	while(*(Str+Len_txt))
	{
		Len_txt++;
		if(*(Str+Len_txt)=='.') break;
	}
	strcpy((char *)StrCopy,(char *)Str);
	strcpy((char *)(StrCopy+18),"..");
	strcpy((char *)(StrCopy+20),(char *)(Str+Len_txt));
	*(StrCopy+20+Len-Len_txt)='\0';
	p=StrCopy;
	return p;
}
#define MAX_LINE_SIZE 400
#define MAX_PAGE_SIZE 50
u16 page[MAX_PAGE_SIZE+1]={0};
// 动态读取txt文件并进行处理
u8 process_from_txt(const u8 *filename) 
{
    FRESULT res;
    char line[MAX_LINE_SIZE]={0};
	u8 pause=0;			//暂停标记
	u16 t=0;
	u8 str_num=0;
	UINT brr;
	u16 ye=0;
	u32 count=0;
	u8 yebuff[10];
	
    res = f_open(file, (const TCHAR*)filename, FA_READ);  // 打开文件
    if (res != FR_OK) {
		Show_Str(60,10,240,16,"Failed to open file:Flase",16,0);
		
        return res;
    }
	BACK_COLOR=BackColor;
	POINT_COLOR=ForeColor;
	
#if TXT_Test_Title_name
	strcpy((char *)txt_filename,(const char*)(filename+file_path_break(filename)+1));
	if(strlen((const char *)txt_filename)>23)
		Show_Str(1,1,230,20,(u8 *)Shorten_Long_Str(txt_filename),16,0);	//小说名作为标题
	else
		Show_Str(1,1,230,20,(u8 *)txt_filename,16,0);	//小说名作为标题
#endif
	Show_Str(1,1,230,20,(u8 *)(filename+file_path_break(filename)+1),16,0);	//小说名作为标题
	LCD_DrawLine(5,20,235,20);
	LCD_DrawLine(5,285,235,285);
	key = KEY0_PRES;
    while (!f_eof(file)) 			// 按行读取文件内容
	{  
		
			if(key == DISPLAY_PRES)    //按键1、2都没有按下时就不重复显示，避免闪烁
			{
				//LCD_Clear(BLACK);			//清屏	
				LCD_Fill(10,23,240,284,BackColor);
				LCD_Fill(10,288,240,320,BackColor);
//				if(0!=ye)
					Show_Str_rSTRLin(10,30,230,270,(u8 *)line,16,0,10);
				sprintf((char *)yebuff, "第%d页", ye);
				Show_Str_rSTRLin(160,290,230,320,(u8 *)yebuff,16,0,10);
				key = LEISURE_PRES;
			}else if(key==KEY0_PRES)		//下一张
			{
				
				t = 0;
				if(f_size(file) - count > MAX_LINE_SIZE)
				{
					f_read(file, line, MAX_LINE_SIZE, &brr);
					str_num=Str_rSTRLin(10,30,230,270,(u8 *)line,16,0,10);
					if(0!=str_num)
						f_lseek(file,f_tell(file)-str_num);
					if(Chinese2Char((u8 *)line, MAX_LINE_SIZE-str_num))    //检测是否乱码
					{
						count += MAX_LINE_SIZE-str_num - 1;
						f_lseek(file, count);
						page[ye%MAX_PAGE_SIZE] = MAX_LINE_SIZE-str_num - 1;
						line[MAX_LINE_SIZE - str_num - 1] = '\0';
					} else
					{
						count += MAX_LINE_SIZE - str_num;
						page[ye%MAX_PAGE_SIZE] = MAX_LINE_SIZE-str_num;
						line[MAX_LINE_SIZE - str_num] = '\0';
					}
					ye++;
				} else if(f_size(file) - count > 0)
				{
					f_read(file, line, MAX_LINE_SIZE, &brr);
					str_num=Str_rSTRLin(10,30,230,270,(u8 *)line,16,0,10);
					if(0!=str_num)
						f_lseek(file,f_tell(file)-str_num);
					page[ye%MAX_PAGE_SIZE] = MAX_LINE_SIZE-str_num;
					count += MAX_LINE_SIZE - str_num;
					ye++;
				}
				key = DISPLAY_PRES;			//回到显示状态
			}else if(key==KEY1_PRES)		//上一张
			{
				t = 0;
				if(ye > 1)
				{
					ye--;
					count -= page[ye%MAX_PAGE_SIZE];
					f_lseek(file, count - page[(ye-1)%MAX_PAGE_SIZE]);
					f_read(file, line, page[(ye-1)%MAX_PAGE_SIZE], &brr);
//					if(page[ye - 1] == MAX_LINE_SIZE - 1)
					line[page[(ye-1)%MAX_PAGE_SIZE]] = '\0';
				}					
				key = DISPLAY_PRES;
			}else if(key==WKUP_PRES)
			{
				pause=!pause;
				LED1=!pause; 	//暂停的时候LED1亮. 
				LED0 = 1;				
				key = DISPLAY_PRES;
			}else if(key == CLOSE_PRES)   //按键3长按，关闭文件
			{
				key = LEISURE_PRES;
				res = f_close(file);  // 关闭文件
				LED0 = 1;
				LED1 = 1;
				return res;	
			}
			if(pause==0)t++;
			delay_ms(10); 				// 读取下一页之前可以进行必要的等待或延时操作
			if(t>TXT_Speed)
					key=1;				//模拟一次按下KEY0  
			if((t%100)==0)LED0=!LED0;	//LED0闪烁,提示程序正在运行.
				
		
    }
	res = f_close(file);  // 关闭文件
	return res;	
}

#endif




















		  






