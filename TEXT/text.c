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

#define TXT_FLAG 1			//txt�ļ���ȡ		
#define TXT_Test_Title_name 0
//code �ַ�ָ�뿪ʼ
//���ֿ��в��ҳ���ģ
//code �ַ����Ŀ�ʼ��ַ,GBK��
//mat  ���ݴ�ŵ�ַ (size/8+((size%8)?1:0))*(size) bytes��С	
//size:�����С
u16 TXT_Speed=300;
void Get_HzMat(unsigned char *code,unsigned char *mat,u8 size)
{		    
	unsigned char qh,ql;
	unsigned char i;					  
	unsigned long foffset; 
	u8 csize=(size/8+((size%8)?1:0))*(size);//�õ�����һ���ַ���Ӧ������ռ���ֽ���	 
	qh=*code;
	ql=*(++code);
	if(qh<0x81||ql<0x40||ql==0xff||qh==0xff)//�� ���ú���
	{   		    
	    for(i=0;i<csize;i++)*mat++=0x00;//�������
	    return; //��������
	}          
	if(ql<0x7f)ql-=0x40;//ע��!
	else ql-=0x41;
	qh-=0x81;   
	foffset=((unsigned long)190*qh+ql)*csize;	//�õ��ֿ��е��ֽ�ƫ����  		  
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
//��ʾһ��ָ����С�ĺ���
//x,y :���ֵ�����
//font:����GBK��
//size:�����С
//mode:0,������ʾ,1,������ʾ	   
void Show_Font(u16 x,u16 y,u8 *font,u8 size,u8 mode)
{
	u8 temp,t,t1;
	u16 y0=y;
	u8 dzk[72];   
	u8 csize=(size/8+((size%8)?1:0))*(size);//�õ�����һ���ַ���Ӧ������ռ���ֽ���	 
	if(size!=12&&size!=16&&size!=24)return;	//��֧�ֵ�size
	Get_HzMat(font,dzk,size);	//�õ���Ӧ��С�ĵ������� 
	for(t=0;t<csize;t++)
	{   												   
		temp=dzk[t];			//�õ���������                          
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
//��ָ��λ�ÿ�ʼ��ʾһ���ַ���	    
//֧���Զ�����
//(x,y):��ʼ����
//width,height:����
//str  :�ַ���
//size :�����С
//mode:0,�ǵ��ӷ�ʽ;1,���ӷ�ʽ    	   		   
void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode)
{					
	u16 x0=x;
	u16 y0=y;							  	  
    u8 bHz=0;     //�ַ���������  	    				    				  	  
    while(*str!=0)//����δ����
    { 
        if(!bHz)
        {
	        if(*str>0x80)bHz=1;//���� 
	        else              //�ַ�
	        {      
                if(x>(x0+width-size/2))//����
				{				   
					y+=size+4;
					x=x0;	   
				}							    
		        if(y>(y0+height-size))break;//Խ�緵��      
		        if(*str=='\n')//���з���
		        {         
		            y+=size+4;
					x=x0;
		            str++; 
		        }  
		        else LCD_ShowChar(x,y,*str,size,mode);//��Ч����д�� 
				str++; 
		        x+=size/2; //�ַ�,Ϊȫ�ֵ�һ�� 
	        }
        }else//���� 
        {     
            bHz=0;//�к��ֿ�    
            if(x>(x0+width-size))//����
			{	    
				y+=size+4;
				x=x0;		  
			}
	        if(y>(y0+height-size))break;//Խ�緵��  						     
	        Show_Font(x,y,str,size,mode); //��ʾ�������,������ʾ 
	        str+=2; 
	        x+=size;//��һ������ƫ��	    
        }						 
    }   
}  	


/*
���ı�������ʾ����
line_size:�ı��м��
����ֵ:�޷���ʾ��ʣ���ֽ���
								*/
u8 Str_rSTRLin(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode,u8 line_size)
{					
	u16 x0=x;
	u16 y0=y;							  	  
    u8 bHz=0;     //�ַ���������  
    while(*str!=0)//����δ����
    { 
        if(!bHz)
        {
	        if(*str>0x80)bHz=1;//���� 
	        else              //�ַ�
	        {      
                if(x>(x0+width-size/2))//����
				{				   
					y+=size+line_size;
					x=x0;	 				
				}							    
		        if(y>(y0+height-size))
				{
					if(*str!='\r')					//����windowsCRLF��ʽ
						str++;
					if(*str!='\n')
						str++;
					return strlen((char *)str);//Խ�緵�� 
				}      
				if(*str=='\n')//���з���
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
		        x+=size/2; //�ַ�,Ϊȫ�ֵ�һ�� 
	        }
        }else//���� 
        {     
            bHz=0;//�к��ֿ�    
            if(x>(x0+width-size))//����
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
				return strlen((char *)str);//Խ�緵�� 
			}  
	        str+=2; 
	        x+=size;//��һ������ƫ��	    
        }						 
    }   
	return 0;
}
/*
���ı����н�����ʾ
line_size:�ı��м��
�����Զ�����
*/
void Show_Str_rSTRLin(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode,u8 line_size)
{					
	u16 x0=x;
	u16 y0=y;							  	  
    u8 bHz=0;     //�ַ���������  
    while(*str!=0)//����δ����
    { 
        if(!bHz)
        {
	        if(*str>0x80)bHz=1;//���� 
	        else              //�ַ�
	        {      
                if(x>(x0+width-size/2))//����
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
					return ;//Խ�緵��   
				}
		        if(*str=='\n')//���з���
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
				else LCD_ShowChar(x,y,*str,size,mode);//��Ч����д�� 
				str++; 
		        x+=size/2; //�ַ�,Ϊȫ�ֵ�һ�� 
	        }
        }else//���� 
        {     
            bHz=0;//�к��ֿ�    
            if(x>(x0+width-size))//����
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
				return ;//Խ�緵��   
			}  		     
	        Show_Font(x,y,str,size,mode); //��ʾ�������,������ʾ 
	        str+=2; 
	        x+=size;//��һ������ƫ��	    
        }						 
    }   
	return ;
}
//��ָ����ȵ��м���ʾ�ַ���
//����ַ����ȳ�����len,����Show_Str��ʾ
//len:ָ��Ҫ��ʾ�Ŀ��			  
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
u8 Chinese2Char(u8 * buff, u16 size) //����ַ������һ���ֽ��Ƿ������ĵ�һ���ֽڣ�
{                                   //�Ƿ���0�����Ƿ���1
    u16 i = 0;
    while(i != size - 1 && i != size -2) //˳���⣬ֱ�����ʣ��һ���������ֽ�
    {
        if(*buff < 128)  {buff++;i++;}
        else  {buff += 2;i += 2;}
    }
    if(i == size - 2)      //ʣ����������ֽ�
    {
        if(*buff > 128)  return 0;  
        else
        {
            buff++;
            if(*buff > 128)  return 1;
            else  return 0;
        }
    }
    if(i == size - 1)  //ʣ�����һ���ֽ�
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
// ��̬��ȡtxt�ļ������д���
u8 process_from_txt(const u8 *filename) 
{
    FRESULT res;
    char line[MAX_LINE_SIZE]={0};
	u8 pause=0;			//��ͣ���
	u16 t=0;
	u8 str_num=0;
	UINT brr;
	u16 ye=0;
	u32 count=0;
	u8 yebuff[10];
	
    res = f_open(file, (const TCHAR*)filename, FA_READ);  // ���ļ�
    if (res != FR_OK) {
		Show_Str(60,10,240,16,"Failed to open file:Flase",16,0);
		
        return res;
    }
	BACK_COLOR=BackColor;
	POINT_COLOR=ForeColor;
	
#if TXT_Test_Title_name
	strcpy((char *)txt_filename,(const char*)(filename+file_path_break(filename)+1));
	if(strlen((const char *)txt_filename)>23)
		Show_Str(1,1,230,20,(u8 *)Shorten_Long_Str(txt_filename),16,0);	//С˵����Ϊ����
	else
		Show_Str(1,1,230,20,(u8 *)txt_filename,16,0);	//С˵����Ϊ����
#endif
	Show_Str(1,1,230,20,(u8 *)(filename+file_path_break(filename)+1),16,0);	//С˵����Ϊ����
	LCD_DrawLine(5,20,235,20);
	LCD_DrawLine(5,285,235,285);
	key = KEY0_PRES;
    while (!f_eof(file)) 			// ���ж�ȡ�ļ�����
	{  
		
			if(key == DISPLAY_PRES)    //����1��2��û�а���ʱ�Ͳ��ظ���ʾ��������˸
			{
				//LCD_Clear(BLACK);			//����	
				LCD_Fill(10,23,240,284,BackColor);
				LCD_Fill(10,288,240,320,BackColor);
//				if(0!=ye)
					Show_Str_rSTRLin(10,30,230,270,(u8 *)line,16,0,10);
				sprintf((char *)yebuff, "��%dҳ", ye);
				Show_Str_rSTRLin(160,290,230,320,(u8 *)yebuff,16,0,10);
				key = LEISURE_PRES;
			}else if(key==KEY0_PRES)		//��һ��
			{
				
				t = 0;
				if(f_size(file) - count > MAX_LINE_SIZE)
				{
					f_read(file, line, MAX_LINE_SIZE, &brr);
					str_num=Str_rSTRLin(10,30,230,270,(u8 *)line,16,0,10);
					if(0!=str_num)
						f_lseek(file,f_tell(file)-str_num);
					if(Chinese2Char((u8 *)line, MAX_LINE_SIZE-str_num))    //����Ƿ�����
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
				key = DISPLAY_PRES;			//�ص���ʾ״̬
			}else if(key==KEY1_PRES)		//��һ��
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
				LED1=!pause; 	//��ͣ��ʱ��LED1��. 
				LED0 = 1;				
				key = DISPLAY_PRES;
			}else if(key == CLOSE_PRES)   //����3�������ر��ļ�
			{
				key = LEISURE_PRES;
				res = f_close(file);  // �ر��ļ�
				LED0 = 1;
				LED1 = 1;
				return res;	
			}
			if(pause==0)t++;
			delay_ms(10); 				// ��ȡ��һҳ֮ǰ���Խ��б�Ҫ�ĵȴ�����ʱ����
			if(t>TXT_Speed)
					key=1;				//ģ��һ�ΰ���KEY0  
			if((t%100)==0)LED0=!LED0;	//LED0��˸,��ʾ������������.
				
		
    }
	res = f_close(file);  // �ر��ļ�
	return res;	
}

#endif




















		  






