#include "piclib.h"
#include "lcd.h"
#include "string.h"
#include "text.h"
#include "key.h"
#include "led.h"
#include "delay.h"
#include "exti.h"
#include "mp3player.h"

//////////////////////////////////////////////////////////////////////////////////	 
//升级说明
//V2.0
//1,将jpeg解码库换成了TJPGD,支持更多的jpg/jpeg文件,支持小尺寸图片快速jpeg显示 
//2,pic_phy里面新增fillcolor函数,用于填充显示,以提高小尺寸jpg图片的显示速度
//3,ai_load_picfile函数,新增一个参数:fast,用于设置是否使能jpeg/jpg快速显示
//注意:这里的小尺寸是指:jpg/jpeg图片尺寸小于等于LCD尺寸.
//////////////////////////////////////////////////////////////////////////////////

_pic_info picinfo;	 	//图片信息
_pic_phy pic_phy;		//图片显示物理接口	
//////////////////////////////////////////////////////////////////////////
//lcd.h没有提供划横线函数,需要自己实现
void piclib_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if((len==0)||(x0>lcddev.width)||(y0>lcddev.height))return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//填充颜色
//x,y:起始坐标
//width，height：宽度和高度。
//*color：颜色数组
void piclib_fill_color(u16 x,u16 y,u16 width,u16 height,u16 *color)
{  
	LCD_Color_Fill(x,y,x+width-1,y+height-1,color);	
}
//////////////////////////////////////////////////////////////////////////
//画图初始化,在画图之前,必须先调用此函数
//指定画点/读点
void piclib_init(void)
{
	pic_phy.read_point=LCD_ReadPoint;  		//读点函数实现
	pic_phy.draw_point=LCD_Fast_DrawPoint;	//画点函数实现
	pic_phy.fill=LCD_Fill;					//填充函数实现
	pic_phy.draw_hline=piclib_draw_hline;  	//画线函数实现
	pic_phy.fillcolor=piclib_fill_color;  	//颜色填充函数实现 

	picinfo.lcdwidth=lcddev.width;	//得到LCD的宽度像素
	picinfo.lcdheight=lcddev.height;//得到LCD的高度像素

	picinfo.ImgWidth=0;	//初始化宽度为0
	picinfo.ImgHeight=0;//初始化高度为0
	picinfo.Div_Fac=0;	//初始化缩放系数为0
	picinfo.S_Height=0;	//初始化设定的高度为0
	picinfo.S_Width=0;	//初始化设定的宽度为0
	picinfo.S_XOFF=0;	//初始化x轴的偏移量为0
	picinfo.S_YOFF=0;	//初始化y轴的偏移量为0
	picinfo.staticx=0;	//初始化当前显示到的x坐标为0
	picinfo.staticy=0;	//初始化当前显示到的y坐标为0
}
//快速ALPHA BLENDING算法.
//src:源颜色
//dst:目标颜色
//alpha:透明程度(0~32)
//返回值:混合后的颜色.
u16 piclib_alpha_blend(u16 src,u16 dst,u8 alpha)
{
	u32 src2;
	u32 dst2;	 
	//Convert to 32bit |-----GGGGGG-----RRRRR------BBBBB|
	src2=((src<<16)|src)&0x07E0F81F;
	dst2=((dst<<16)|dst)&0x07E0F81F;   
	//Perform blending R:G:B with alpha in range 0..32
	//Note that the reason that alpha may not exceed 32 is that there are only
	//5bits of space between each R:G:B value, any higher value will overflow
	//into the next component and deliver ugly result.
	dst2=((((dst2-src2)*alpha)>>5)+src2)&0x07E0F81F;
	return (dst2>>16)|dst2;  
}
//初始化智能画点
//内部调用
void ai_draw_init(void)
{
	float temp,temp1;	   
	temp=(float)picinfo.S_Width/picinfo.ImgWidth;
	temp1=(float)picinfo.S_Height/picinfo.ImgHeight;						 
	if(temp<temp1)temp1=temp;//取较小的那个	 
	if(temp1>1)temp1=1;	  
	//使图片处于所给区域的中间
	picinfo.S_XOFF+=(picinfo.S_Width-temp1*picinfo.ImgWidth)/2;
	picinfo.S_YOFF+=(picinfo.S_Height-temp1*picinfo.ImgHeight)/2;
	temp1*=8192;//扩大8192倍	 
	picinfo.Div_Fac=temp1;
	picinfo.staticx=0xffff;
	picinfo.staticy=0xffff;//放到一个不可能的值上面			 										    
}   
//判断这个像素是否可以显示
//(x,y) :像素原始坐标
//chg   :功能变量. 
//返回值:0,不需要显示.1,需要显示
u8 is_element_ok(u16 x,u16 y,u8 chg)
{				  
	if(x!=picinfo.staticx||y!=picinfo.staticy)
	{
		if(chg==1)
		{
			picinfo.staticx=x;
			picinfo.staticy=y;
		} 
		return 1;
	}else return 0;
}
u8 ai_load_jpg(const u8 *filename,u16 x,u16 y,u16 width,u16 height,u8 fast)
{	
	u8	res;//返回值
	u8 temp;	
	if((x+width)>picinfo.lcdwidth)return PIC_WINDOW_ERR;		//x坐标超范围了.
	if((y+height)>picinfo.lcdheight)return PIC_WINDOW_ERR;		//y坐标超范围了.  
	//得到显示方框大小	  	 
	if(width==0||height==0)return PIC_WINDOW_ERR;	//窗口设定错误
	picinfo.S_Height=height;
	picinfo.S_Width=width;
	//显示区域无效
	if(picinfo.S_Height==0||picinfo.S_Width==0)
	{
		picinfo.S_Height=lcddev.height;
		picinfo.S_Width=lcddev.width;
		return FALSE;   
	}
	if(pic_phy.fillcolor==NULL)fast=0;//颜色填充函数未实现,不能快速显示
	//显示的开始坐标点
	picinfo.S_YOFF=y;
	picinfo.S_XOFF=x;
	//文件名传递		 
	temp=f_typetell((u8*)filename);	//得到文件的类型
	switch(temp)
	{											  
		case T_BMP:
			res=stdbmp_decode(filename); 				//解码bmp	  	  
			break;
		case T_JPG:
		case T_JPEG:
			res=jpg_decode(filename,fast,0);				//解码JPG/JPEG	  	  
			break;
		case T_GIF:
			res=gif_decode(filename,x,y,width,height);	//解码gif  	  
			break;
		default:
	 		res=PIC_FORMAT_ERR;  						//非可识别格式!!!  
			break;
	}  											   
	return res;
}
//智能画图
//FileName:要显示的图片文件  BMP/JPG/JPEG/GIF
//x,y,width,height:坐标及显示区域尺寸
//fast:使能jpeg/jpg小图片(图片尺寸小于等于液晶分辨率)快速解码,0,不使能;1,使能.
//图片在开始和结束的坐标点范围内显示
u8 ai_load_picfile(const u8 *filename,u16 x,u16 y,u16 width,u16 height,u8 fast)
{	
	u8	res;//返回值
	u8 temp;	
	if((x+width)>picinfo.lcdwidth)return PIC_WINDOW_ERR;		//x坐标超范围了.
	if((y+height)>picinfo.lcdheight)return PIC_WINDOW_ERR;		//y坐标超范围了.  
	//得到显示方框大小	  	 
	if(width==0||height==0)return PIC_WINDOW_ERR;	//窗口设定错误
	picinfo.S_Height=height;
	picinfo.S_Width=width;
	//显示区域无效
	if(picinfo.S_Height==0||picinfo.S_Width==0)
	{
		picinfo.S_Height=lcddev.height;
		picinfo.S_Width=lcddev.width;
		return FALSE;   
	}
	if(pic_phy.fillcolor==NULL)fast=0;//颜色填充函数未实现,不能快速显示
	//显示的开始坐标点
	picinfo.S_YOFF=y;
	picinfo.S_XOFF=x;
	//文件名传递		 
	temp=f_typetell((u8*)filename);	//得到文件的类型
	switch(temp)
	{											  
		case T_BMP:
			Show_Str(2,2,240,16,(u8 *)filename,16,1);
			res=stdbmp_decode(filename); 				//解码bmp	  	  
			break;
		case T_JPG:
		case T_JPEG:
			Show_Str(2,2,240,16,(u8 *)filename,16,1);
			res=jpg_decode(filename,fast,1);				//解码JPG/JPEG	  	  
			break;
		case T_GIF:
			Show_Str(2,2,240,16,(u8 *)filename,16,1);
			res=gif_decode(filename,x,y,width,height);	//解码gif  	  
			break;
		case T_TEXT:
			res=process_from_txt(filename);		//解码txt
			break;
		case T_MP3:
		case T_OGG:
		case T_ACC:
		case T_WMA:
		case T_WAV:
		case T_MID:
			res=mp3_play_song((u8 *)filename);		//解码音频格式
			break;
		default:
	 		res=PIC_FORMAT_ERR;
			break;
	}  											   
	return res;
}


extern FATFS *fs[2];
extern FIL *file;          //文件1
extern UINT br, bw;         //读写变量

// 将字符串写入txt文件
void write_to_txt(const char *filename, const char *str) {
    FRESULT res;

    res = f_open(file, filename, FA_CREATE_ALWAYS | FA_WRITE);  // 打开或创建文件
    if (res != FR_OK) {
		Show_Str(60,150,240,16,"Failed to open file:Flase",16,0); 
        return;
    }

    res = f_write(file, str, strlen(str), &bw);  // 写入数据
    if (res != FR_OK) {
		Show_Str(60,170,240,16,"Failed to write file:Flase",16,0); 
        f_close(file);
        return;
    }

    res = f_close(file);  // 关闭文件
    if (res != FR_OK) {
		Show_Str(60,190,240,16,"Failed to close file:Flase",16,0);
        return;
    }
	Show_Str(60,210,240,16,"Write successfully, bytes written\r\n",16,0);
}

void append_to_file(const char *filename, const char *data) {
    FRESULT res;

    res = f_open(file, filename, FA_WRITE | FA_OPEN_ALWAYS);  // 打开文件
    if (res != FR_OK) {
		Show_Str(60,10,240,16,"Failed to open file:Flase",16,0);
        return;
    }

    res = f_lseek(file, file->fsize);  // 将写入指针定位到文件末尾
    if (res != FR_OK) {
        
		Show_Str(60,10,240,16,"Failed to seek file:Flase",16,0);
        f_close(file);
        return;
    }

    res = f_write(file, data, strlen(data), &bw);  // 写入数据到文件末尾
    if (res != FR_OK || bw == 0) {
        
		Show_Str(60,10,240,16,"Failed to write file:Flase",16,0);
        f_close(file);
        return;
    }

    res = f_close(file);  // 关闭文件
    if (res != FR_OK) {
        
		Show_Str(60,10,240,16,"Failed to close file:Flase",16,0);
        return;
    }
}
void Draw_Optionsbar(u8 Options_num ,u8 Options_Maxnum,u8  Options_DisplayMax)
{
	u16 x,y;
	LCD_DrawRectangle(227, 75, 227, 250);
	x=(Options_DisplayMax*1.0)/(Options_Maxnum)*170;	//选项条长度
	y=1.0/(Options_Maxnum)*170;						//单个选项页长度
	LCD_Fill(224, 80+y*Options_num, 230, 80+y*Options_num+x, ForeColor); 
}











