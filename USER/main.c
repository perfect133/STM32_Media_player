#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "key.h"
#include "usmart.h"
#include "malloc.h"
#include "MMC_SD.h"
#include "ff.h"
#include "exfuns.h"
#include "fontupd.h"
#include "text.h"
#include "piclib.h"
#include "string.h"
#include "exti.h"
#include "bmp_file.h"
#include "vs10XX.h"
#include "mp3player.h"
#include "Menu.h"

#define OptionsNum 9
// 得到path路径下,目标文件的总个数
// path:路径
// 返回值:总有效文件数
// 全局变量区域

u8 filename_link[80][30];

u16 file_get_tnum(u8 *path, u8 parsing_num)
{
    u8 res;
    u16 rval = 0;
    DIR tdir;          // 临时目录
    FILINFO tfileinfo; // 临时文件信息
    u8 *fn;
    res              = f_opendir(&tdir, (const TCHAR *)path); // 打开目录
    tfileinfo.lfsize = _MAX_LFN * 2 + 1;                      // 长文件名最大长度
    tfileinfo.lfname = mymalloc(tfileinfo.lfsize);            // 为长文件缓存区分配内存
    if (res == FR_OK && tfileinfo.lfname != NULL) {
        while (1) // 查询总的有效文件数
        {
            res = f_readdir(&tdir, &tfileinfo);                 // 读取目录下的一个文件
            if (res != FR_OK || tfileinfo.fname[0] == 0) break; // 错误了/到末尾了,退出
            fn  = (u8 *)(*tfileinfo.lfname ? tfileinfo.lfname : tfileinfo.fname);
            res = f_typetell(fn);
            if ((res & 0XF0) == parsing_num) // 取高四位,看看是不是可识别文件
            {
                rval++; // 有效文件数增加1
            }
        }
    }
    return rval;
}

void file_parsing(u8 *path, u8 Flag)
{
    u8 res, *pname, *fn, line_num = 1, parsing_num, picnum_flag, new_picnum;
    u16 totpicnum, *picindextbl, curindex, temp;
    DIR picdir;
    FILINFO picfileinfo;
    volatile extern u8 mp3_state_flag;
    switch (Flag) {
        case 0:
            parsing_num = 0x50;
            break;
        case 1:
            parsing_num = 0x30;
            break;
        case 2:
            parsing_num = 0x40;
            break;
        case 3:
            System_Funtion();
            break;
    }
    if (Flag != 3) {
        while (f_opendir(&picdir, (const TCHAR *)path)) // 打开图片文件夹
        {
            Show_Str(60, 170, 240, 16, "文件夹错误!", 16, 0);
            delay_ms(200);
            LCD_Fill(60, 170, 240, 186, BackColor); // 清除显示
            delay_ms(200);
        }
        totpicnum = file_get_tnum(path, parsing_num); // 得到总有效文件数
        while (totpicnum == NULL)                     // 图片文件为0
        {
            Show_Str(60, 170, 240, 16, "没有发现该类型文件!", 16, 0);
            delay_ms(3000);
            return;
        }
        picfileinfo.lfsize = _MAX_LFN * 2 + 1;                                     // 长文件名最大长度
        picfileinfo.lfname = mymalloc(picfileinfo.lfsize);                         // 为长文件缓存区分配内存
        pname              = mymalloc(picfileinfo.lfsize);                         // 为带路径的文件名分配内存
        picindextbl        = mymalloc(2 * totpicnum);                              // 申请2*totpicnum个字节的内存,用于存放图片索引
        while (picfileinfo.lfname == NULL || pname == NULL || picindextbl == NULL) // 内存分配出错
        {
            Show_Str(60, 170, 240, 16, "内存分配失败!", 16, 0);
            delay_ms(200);
            LCD_Fill(60, 170, 240, 186, BackColor); // 清除显示
            delay_ms(200);
        }
        // 记录索引
        res = f_opendir(&picdir, (const TCHAR *)path); // 打开目录
        if (res == FR_OK) {
            curindex = 0; // 当前索引为0
            while (1)     // 全部查询一遍
            {
                temp = picdir.index;                                  // 记录当前index
                res  = f_readdir(&picdir, &picfileinfo);              // 读取目录下的一个文件
                if (res != FR_OK || picfileinfo.fname[0] == 0) break; // 错误了/到末尾了,退出
                fn = (u8 *)(*picfileinfo.lfname ? picfileinfo.lfname : picfileinfo.fname);
                //			strcpy((char *)filename_link[curindex],(const char*)fn);
                if (strlen((char *)fn) > 23)
                    strcpy((char *)filename_link[curindex], (const char *)Shorten_Long_Str(fn));
                else
                    strcpy((char *)filename_link[curindex], (const char *)fn);
                if (line_num < OptionsNum) {
                    //				Show_Str(20,10+line_num*20,240,16,(u8 *)filename_link[curindex],16,0);
                    line_num++;
                }
                res = f_typetell(fn);
                if ((res & 0XF0) == parsing_num) // 取高四位,看看是不是可识别文件
                {
                    picindextbl[curindex] = temp; // 记录索引
                    curindex++;
                }
            }
        } // 初始化画图
        curindex    = 0;                                       // 从0开始显示
        res         = f_opendir(&picdir, (const TCHAR *)path); // 打开目录Test/Demo1
        picnum_flag = 0;
        while (res == FR_OK) // 打开成功
        {
            LCD_Clear(BackColor);
            Show_Str(10, 5, 240, 16, (u8 *)path, 16, 0);
            line_num = 1;
            if (totpicnum + 1 > OptionsNum) {
                Draw_Optionsbar(picnum_flag, totpicnum, OptionsNum);
            }
            for (new_picnum = picnum_flag; new_picnum < totpicnum && new_picnum < (picnum_flag + OptionsNum); new_picnum++) {
                if (curindex == new_picnum) {
                    POINT_COLOR = BackColor;
                    BACK_COLOR  = ForeColor;
                    if (mp3_state_flag == MP3_Free_Flag)
                        Show_Str(20, 10 + line_num * 30, 240, 16, (u8 *)filename_link[new_picnum], 16, 0);
                    line_num++;
                    POINT_COLOR = ForeColor;
                    BACK_COLOR  = BackColor;
                } else {
                    if (mp3_state_flag == MP3_Free_Flag)
                        Show_Str(20, 10 + line_num * 30, 240, 16, (u8 *)filename_link[new_picnum], 16, 0);
                    line_num++;
                }
            }
            if (mp3_state_flag == MP3_WKUP_Flag) {
                key            = WKUP_PRES;
                mp3_state_flag = MP3_Ready_Flag;
            } else if (mp3_state_flag == MP3_Next_Flag) {
                key            = KEY0_PRES;
                mp3_state_flag = MP3_WKUP_Flag;
            } else if (mp3_state_flag == MP3_Pre_Flag) {
                key            = KEY1_PRES;
                mp3_state_flag = MP3_WKUP_Flag;
            } else if (mp3_state_flag == MP3_Ready_Flag) {
                mp3_state_flag = MP3_Free_Flag;
            }
            while (1) {
                if (key == KEY1_PRES) // 上一张
                {
                    if (curindex <= picnum_flag) {
                        if (picnum_flag > 0)
                            picnum_flag--;
                    }
                    if (curindex)
                        curindex--;
                    else {
                        curindex    = totpicnum - 1;
                        picnum_flag = totpicnum - OptionsNum;
                    }
                    key = DISPLAY_PRES;
                    break;
                } else if (key == KEY0_PRES) // 下一张
                {
                    if (curindex >= picnum_flag + OptionsNum - 1) {
                        if (picnum_flag < totpicnum - OptionsNum)
                            picnum_flag++;
                    }
                    if (curindex < totpicnum - 1)
                        curindex++; // 到末尾的时候,自动从头开始
                    else {
                        curindex    = 0;
                        picnum_flag = 0;
                    }
                    key = DISPLAY_PRES;
                    break;
                } else if (key == WKUP_PRES) {
                    key = DISPLAY_PRES;
                    dir_sdi(&picdir, picindextbl[curindex]);              // 改变当前目录索引
                    res = f_readdir(&picdir, &picfileinfo);               // 读取目录下的一个文件
                    if (res != FR_OK || picfileinfo.fname[0] == 0) break; // 错误了/到末尾了,退出
                    fn = (u8 *)(*picfileinfo.lfname ? picfileinfo.lfname : picfileinfo.fname);
                    strcpy((char *)pname, (char *)path); // 复制路径(目录)
                    strcat((char *)pname, (const char *)"/");
                    strcat((char *)pname, (const char *)fn); // 将文件名接在后面
                    LCD_Clear(BackColor);
                    if (Flag == 2) // MP3格式读取风格设置
                    {
                        LCD_Clear(BackColor);
                        POINT_COLOR = ForeColor;
                        mp3_index_show(curindex + 1, totpicnum);
                    }
                    ai_load_picfile(pname, 0, 0, lcddev.width, lcddev.height, 1); // 打开并解析文件
                    curindex    = curindex;
                    POINT_COLOR = ForeColor;
                    BACK_COLOR  = BackColor;
                    break;
                } else if (key == CLOSE_PRES)
                    break;
                delay_ms(10);
            }
            if (key == CLOSE_PRES) {
                key = DISPLAY_PRES;
                break;
            }
            res = 0;
        }
        myfree(picfileinfo.lfname); // 释放内存
        myfree(pname);              // 释放内存
        myfree(picindextbl);        // 释放内存
    }
}

int main(void)
{
    u8 OptionsFlag;
    delay_init();                                   // 延时函数初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组2
    uart_init(9600);                                // 串口初始化为9600
    LCD_Init();                                     // 初始化液晶
    LED_Init();                                     // LED初始化
    VS_Init();                                      // 初始化VS1053
    EXTIX_Init();                                   // 外部中断初始化
    usmart_dev.init(72);                            // usmart初始化
    mem_init();                                     // 初始化内部内存池
    exfuns_init();                                  // 为fatfs相关变量申请内存
    f_mount(fs[0], "0:", 1);                        // 挂载SD卡
    f_mount(fs[1], "1:", 1);                        // 挂载FLASH.
    POINT_COLOR = ForeColor;
    //	update_font(20,110,16);
    while (font_init()) // 检查字库
    {

        LCD_ShowString(60, 50, 200, 16, 16, "Font Error!");
        delay_ms(200);
        LCD_Fill(60, 50, 240, 66, BackColor); // 清除显示
        delay_ms(200);
    }
    VS_Sine_Test();
    VS_HD_Reset();
    VS_Soft_Reset();
    vsset.mvol = 200; // 默认设置音量为200.
    piclib_init();
    OptionsFlag = 0;
    LCD_Clear(BackColor);
    while (1) {
        Show_Str(100, 20, 240, 16, "选项菜单", 16, 0);
        if (key == DISPLAY_PRES) {
            if (0 == OptionsFlag)
                LCD_Fill(60, 110, 120, 160, BackColor);
            else if (1 == OptionsFlag)
                LCD_Fill(160, 110, 240, 160, BackColor);
            else if (2 == OptionsFlag)
                LCD_Fill(60, 240, 120, 320, BackColor);
            else if (3 == OptionsFlag)
                LCD_Fill(160, 240, 240, 320, BackColor);
            POINT_COLOR = ForeColor;
            BACK_COLOR  = BackColor;
            Show_Str(60, 110, 240, 16, "图片", 16, 0);
            Show_Str(160, 110, 240, 16, "小说", 16, 0);
            Show_Str(60, 240, 240, 16, "音乐", 16, 0);
            Show_Str(160, 240, 240, 16, "设置", 16, 0);
            if (0 == OptionsFlag) {
                POINT_COLOR = BackColor;
                BACK_COLOR  = ForeColor;
                Show_Str(60, 110, 240, 16, "图片", 16, 0);
                POINT_COLOR = ForeColor;
                BACK_COLOR  = BackColor;
            } else if (1 == OptionsFlag) {
                POINT_COLOR = BackColor;
                BACK_COLOR  = ForeColor;
                Show_Str(160, 110, 240, 16, "小说", 16, 0);
                POINT_COLOR = ForeColor;
                BACK_COLOR  = BackColor;
            } else if (2 == OptionsFlag) {
                POINT_COLOR = BackColor;
                BACK_COLOR  = ForeColor;
                Show_Str(60, 240, 240, 16, "音乐", 16, 0);
                POINT_COLOR = ForeColor;
                BACK_COLOR  = BackColor;
            } else {
                POINT_COLOR = BackColor;
                BACK_COLOR  = ForeColor;
                Show_Str(160, 240, 240, 16, "设置", 16, 0);
                POINT_COLOR = ForeColor;
                BACK_COLOR  = BackColor;
            }
            key = LEISURE_PRES;
			ai_load_jpg("0:/PICTURE/pic1.jpg", 60, 60, 40, 40, 1); 
			ai_load_jpg("0:/PICTURE/txt1.jpg", 160, 60, 40, 40, 1); 
			ai_load_jpg("0:/PICTURE/mus1.jpg", 60, 180, 40, 40, 1); 
			ai_load_jpg("0:/PICTURE/sys1.jpg", 160, 180, 40,40 , 1);  
        }
		
        if (key == KEY1_PRES) // 上一选项
        {
            if (OptionsFlag) OptionsFlag--;
            key = DISPLAY_PRES;
        } else if (key == KEY0_PRES) // 下一选项
        {
            if (OptionsFlag < 3) OptionsFlag++; // 到末尾的时候,自动从头开始
            key = DISPLAY_PRES;
        } else if (key == WKUP_PRES) {
            key = DISPLAY_PRES;
            // 进行不同格式的文件解析
            file_parsing("0:/Test", OptionsFlag);
            LCD_Clear(BackColor);
            POINT_COLOR = ForeColor;
            BACK_COLOR  = BackColor;
        }
        delay_ms(10);
    }
}
