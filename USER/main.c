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
// �õ�path·����,Ŀ���ļ����ܸ���
// path:·��
// ����ֵ:����Ч�ļ���
// ȫ�ֱ�������

u8 filename_link[80][30];

u16 file_get_tnum(u8 *path, u8 parsing_num)
{
    u8 res;
    u16 rval = 0;
    DIR tdir;          // ��ʱĿ¼
    FILINFO tfileinfo; // ��ʱ�ļ���Ϣ
    u8 *fn;
    res              = f_opendir(&tdir, (const TCHAR *)path); // ��Ŀ¼
    tfileinfo.lfsize = _MAX_LFN * 2 + 1;                      // ���ļ�����󳤶�
    tfileinfo.lfname = mymalloc(tfileinfo.lfsize);            // Ϊ���ļ������������ڴ�
    if (res == FR_OK && tfileinfo.lfname != NULL) {
        while (1) // ��ѯ�ܵ���Ч�ļ���
        {
            res = f_readdir(&tdir, &tfileinfo);                 // ��ȡĿ¼�µ�һ���ļ�
            if (res != FR_OK || tfileinfo.fname[0] == 0) break; // ������/��ĩβ��,�˳�
            fn  = (u8 *)(*tfileinfo.lfname ? tfileinfo.lfname : tfileinfo.fname);
            res = f_typetell(fn);
            if ((res & 0XF0) == parsing_num) // ȡ����λ,�����ǲ��ǿ�ʶ���ļ�
            {
                rval++; // ��Ч�ļ�������1
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
        while (f_opendir(&picdir, (const TCHAR *)path)) // ��ͼƬ�ļ���
        {
            Show_Str(60, 170, 240, 16, "�ļ��д���!", 16, 0);
            delay_ms(200);
            LCD_Fill(60, 170, 240, 186, BackColor); // �����ʾ
            delay_ms(200);
        }
        totpicnum = file_get_tnum(path, parsing_num); // �õ�����Ч�ļ���
        while (totpicnum == NULL)                     // ͼƬ�ļ�Ϊ0
        {
            Show_Str(60, 170, 240, 16, "û�з��ָ������ļ�!", 16, 0);
            delay_ms(3000);
            return;
        }
        picfileinfo.lfsize = _MAX_LFN * 2 + 1;                                     // ���ļ�����󳤶�
        picfileinfo.lfname = mymalloc(picfileinfo.lfsize);                         // Ϊ���ļ������������ڴ�
        pname              = mymalloc(picfileinfo.lfsize);                         // Ϊ��·�����ļ��������ڴ�
        picindextbl        = mymalloc(2 * totpicnum);                              // ����2*totpicnum���ֽڵ��ڴ�,���ڴ��ͼƬ����
        while (picfileinfo.lfname == NULL || pname == NULL || picindextbl == NULL) // �ڴ�������
        {
            Show_Str(60, 170, 240, 16, "�ڴ����ʧ��!", 16, 0);
            delay_ms(200);
            LCD_Fill(60, 170, 240, 186, BackColor); // �����ʾ
            delay_ms(200);
        }
        // ��¼����
        res = f_opendir(&picdir, (const TCHAR *)path); // ��Ŀ¼
        if (res == FR_OK) {
            curindex = 0; // ��ǰ����Ϊ0
            while (1)     // ȫ����ѯһ��
            {
                temp = picdir.index;                                  // ��¼��ǰindex
                res  = f_readdir(&picdir, &picfileinfo);              // ��ȡĿ¼�µ�һ���ļ�
                if (res != FR_OK || picfileinfo.fname[0] == 0) break; // ������/��ĩβ��,�˳�
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
                if ((res & 0XF0) == parsing_num) // ȡ����λ,�����ǲ��ǿ�ʶ���ļ�
                {
                    picindextbl[curindex] = temp; // ��¼����
                    curindex++;
                }
            }
        } // ��ʼ����ͼ
        curindex    = 0;                                       // ��0��ʼ��ʾ
        res         = f_opendir(&picdir, (const TCHAR *)path); // ��Ŀ¼Test/Demo1
        picnum_flag = 0;
        while (res == FR_OK) // �򿪳ɹ�
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
                if (key == KEY1_PRES) // ��һ��
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
                } else if (key == KEY0_PRES) // ��һ��
                {
                    if (curindex >= picnum_flag + OptionsNum - 1) {
                        if (picnum_flag < totpicnum - OptionsNum)
                            picnum_flag++;
                    }
                    if (curindex < totpicnum - 1)
                        curindex++; // ��ĩβ��ʱ��,�Զ���ͷ��ʼ
                    else {
                        curindex    = 0;
                        picnum_flag = 0;
                    }
                    key = DISPLAY_PRES;
                    break;
                } else if (key == WKUP_PRES) {
                    key = DISPLAY_PRES;
                    dir_sdi(&picdir, picindextbl[curindex]);              // �ı䵱ǰĿ¼����
                    res = f_readdir(&picdir, &picfileinfo);               // ��ȡĿ¼�µ�һ���ļ�
                    if (res != FR_OK || picfileinfo.fname[0] == 0) break; // ������/��ĩβ��,�˳�
                    fn = (u8 *)(*picfileinfo.lfname ? picfileinfo.lfname : picfileinfo.fname);
                    strcpy((char *)pname, (char *)path); // ����·��(Ŀ¼)
                    strcat((char *)pname, (const char *)"/");
                    strcat((char *)pname, (const char *)fn); // ���ļ������ں���
                    LCD_Clear(BackColor);
                    if (Flag == 2) // MP3��ʽ��ȡ�������
                    {
                        LCD_Clear(BackColor);
                        POINT_COLOR = ForeColor;
                        mp3_index_show(curindex + 1, totpicnum);
                    }
                    ai_load_picfile(pname, 0, 0, lcddev.width, lcddev.height, 1); // �򿪲������ļ�
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
        myfree(picfileinfo.lfname); // �ͷ��ڴ�
        myfree(pname);              // �ͷ��ڴ�
        myfree(picindextbl);        // �ͷ��ڴ�
    }
}

int main(void)
{
    u8 OptionsFlag;
    delay_init();                                   // ��ʱ������ʼ��
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // �����ж����ȼ�����2
    uart_init(9600);                                // ���ڳ�ʼ��Ϊ9600
    LCD_Init();                                     // ��ʼ��Һ��
    LED_Init();                                     // LED��ʼ��
    VS_Init();                                      // ��ʼ��VS1053
    EXTIX_Init();                                   // �ⲿ�жϳ�ʼ��
    usmart_dev.init(72);                            // usmart��ʼ��
    mem_init();                                     // ��ʼ���ڲ��ڴ��
    exfuns_init();                                  // Ϊfatfs��ر��������ڴ�
    f_mount(fs[0], "0:", 1);                        // ����SD��
    f_mount(fs[1], "1:", 1);                        // ����FLASH.
    POINT_COLOR = ForeColor;
    //	update_font(20,110,16);
    while (font_init()) // ����ֿ�
    {

        LCD_ShowString(60, 50, 200, 16, 16, "Font Error!");
        delay_ms(200);
        LCD_Fill(60, 50, 240, 66, BackColor); // �����ʾ
        delay_ms(200);
    }
    VS_Sine_Test();
    VS_HD_Reset();
    VS_Soft_Reset();
    vsset.mvol = 200; // Ĭ����������Ϊ200.
    piclib_init();
    OptionsFlag = 0;
    LCD_Clear(BackColor);
    while (1) {
        Show_Str(100, 20, 240, 16, "ѡ��˵�", 16, 0);
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
            Show_Str(60, 110, 240, 16, "ͼƬ", 16, 0);
            Show_Str(160, 110, 240, 16, "С˵", 16, 0);
            Show_Str(60, 240, 240, 16, "����", 16, 0);
            Show_Str(160, 240, 240, 16, "����", 16, 0);
            if (0 == OptionsFlag) {
                POINT_COLOR = BackColor;
                BACK_COLOR  = ForeColor;
                Show_Str(60, 110, 240, 16, "ͼƬ", 16, 0);
                POINT_COLOR = ForeColor;
                BACK_COLOR  = BackColor;
            } else if (1 == OptionsFlag) {
                POINT_COLOR = BackColor;
                BACK_COLOR  = ForeColor;
                Show_Str(160, 110, 240, 16, "С˵", 16, 0);
                POINT_COLOR = ForeColor;
                BACK_COLOR  = BackColor;
            } else if (2 == OptionsFlag) {
                POINT_COLOR = BackColor;
                BACK_COLOR  = ForeColor;
                Show_Str(60, 240, 240, 16, "����", 16, 0);
                POINT_COLOR = ForeColor;
                BACK_COLOR  = BackColor;
            } else {
                POINT_COLOR = BackColor;
                BACK_COLOR  = ForeColor;
                Show_Str(160, 240, 240, 16, "����", 16, 0);
                POINT_COLOR = ForeColor;
                BACK_COLOR  = BackColor;
            }
            key = LEISURE_PRES;
			ai_load_jpg("0:/PICTURE/pic1.jpg", 60, 60, 40, 40, 1); 
			ai_load_jpg("0:/PICTURE/txt1.jpg", 160, 60, 40, 40, 1); 
			ai_load_jpg("0:/PICTURE/mus1.jpg", 60, 180, 40, 40, 1); 
			ai_load_jpg("0:/PICTURE/sys1.jpg", 160, 180, 40,40 , 1);  
        }
		
        if (key == KEY1_PRES) // ��һѡ��
        {
            if (OptionsFlag) OptionsFlag--;
            key = DISPLAY_PRES;
        } else if (key == KEY0_PRES) // ��һѡ��
        {
            if (OptionsFlag < 3) OptionsFlag++; // ��ĩβ��ʱ��,�Զ���ͷ��ʼ
            key = DISPLAY_PRES;
        } else if (key == WKUP_PRES) {
            key = DISPLAY_PRES;
            // ���в�ͬ��ʽ���ļ�����
            file_parsing("0:/Test", OptionsFlag);
            LCD_Clear(BackColor);
            POINT_COLOR = ForeColor;
            BACK_COLOR  = BackColor;
        }
        delay_ms(10);
    }
}
