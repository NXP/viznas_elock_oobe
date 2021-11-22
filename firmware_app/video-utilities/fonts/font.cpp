/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 *
 */
#include "stdint.h"
#include "font.h"
#include "cambo.h"
#include "open_sans.h"
#include "raleway.h"
#include "SourceHanSerifSC.h"

/*******************************************************************************
 * Variables
 *******************************************************************************/
D4D_FONT_DESCRIPTOR d4dfnt_desc;
D4D_FONT_DESCRIPTOR d4dfnt_desc_chinese;

/* For Unicode16 Support */
#define PAGE_SIZE_DESC  256
#define UNICODE_FIRST (0x4E00)
#define UNICODE_LAST (0X9FB0)

/*******************************************************************************
 * Code
 *******************************************************************************/

static void set_d4dfnt_desc(font_vizn type)
{
    if (type == OPENSANS8)
    {
        d4dfnt_desc = d4dfnt_OpenSans8_desc;
    }
    else if (type == OPENSANS16)
    {
        d4dfnt_desc = d4dfnt_OpenSans16_desc;
    }
    else if (type == CAMBO10)
    {
        d4dfnt_desc = d4dfnt_Cambo10_desc;
    }
    else if (type == CAMBO18)
    {
        d4dfnt_desc = d4dfnt_Cambo18_desc;
    }
    else if (type == RALEWAY10)
    {
        d4dfnt_desc = d4dfnt_Raleway10_desc;
    }
    else if (type == RALEWAY16)
    {
        d4dfnt_desc = d4dfnt_Raleway16_desc;
    }
}

static void put_char(int x, int y, int c, int font_color16b, int bg_color16b, uint16_t *buf, int pitch)
{
    int i, j, k, bits, width, height, offset;
    k      = 0;
    height = d4dfnt_desc.charFullSize.height;
    width  = d4dfnt_desc.pSizeTable[c - d4dfnt_desc.startChar];
    offset = d4dfnt_desc.pOffTable[c - d4dfnt_desc.startChar];
    bits   = d4dfnt_desc.pFontData[offset++];

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++, bits <<= 1, k++)
        {
            if (k == 8 * sizeof(char))
            {
                bits = d4dfnt_desc.pFontData[offset++];
                k    = 0;
            }
            if (bits & 0x80)
            {
                *(buf + (y + i) * pitch + x + j) = font_color16b;
            }
            else if (bg_color16b != -1)
            {
                *(buf + (y + i) * pitch + x + j) = bg_color16b;
            }
        }
    }
}

void put_string(int x, int y, char *s, int font_color16b, int bg_color16b, font_vizn type, uint16_t *buf, int pitch)
{
    int width;
    set_d4dfnt_desc(type);
    for (; *s; s++)
    {
        put_char(x, y, *s, font_color16b, bg_color16b, buf, pitch);
        width = d4dfnt_desc.pSizeTable[(int)*s - d4dfnt_desc.startChar];
        x += width;
    }
}

int get_stringwidth(char *s, font_vizn type)
{
    int x = 0;
    set_d4dfnt_desc(type);
    for (; *s; s++)
    {
        x += d4dfnt_desc.pSizeTable[(int)*s - d4dfnt_desc.startChar];
    }
    return x;
}

int get_fontheight(font_vizn type)
{
    set_d4dfnt_desc(type);

    return d4dfnt_desc.charFullSize.height;
}

static void set_d4dfnt_desc_chinese(uint16_t unicode)
{
    D4D_FONT_DESCRIPTOR *pDescNow, *pDescNext;
    uint32_t MaxPage, WhichPage = 0;
    uint32_t i = 0;

    MaxPage = (UNICODE_LAST - UNICODE_FIRST) / PAGE_SIZE_DESC;
    WhichPage = (unicode - UNICODE_FIRST) / PAGE_SIZE_DESC;
    pDescNow = (D4D_FONT_DESCRIPTOR *)&d4dfnt_SourceHanSerifSC_desc;
    for(i = 0; i < WhichPage; i++)
    {
        pDescNext = (D4D_FONT_DESCRIPTOR *)pDescNow->pNext;
        pDescNow = pDescNext;
    }
    d4dfnt_desc_chinese = *pDescNow;
}

static void put_char_chinese(int x, int y, uint16_t c, int font_color16b, int bg_color16b, uint16_t *buf, int pitch)
{
    int i, j, k, bits, width, height, offset, start;
    char byte;
    k      = 0;
    height = d4dfnt_desc_chinese.charFullSize.height;
    width  = d4dfnt_desc_chinese.charFullSize.width;
    start  = d4dfnt_desc_chinese.startChar;
    offset = (d4dfnt_desc_chinese.charBmpSize) * ((c - start) % PAGE_SIZE_DESC);

    bits   = d4dfnt_desc_chinese.pFontData[offset++];

    //
    // Draw Chinese Character Bitmap
    //
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++, bits <<= 1, k++)
        {
            if (k == 8 * sizeof(char))
            {
                bits = d4dfnt_desc_chinese.pFontData[offset++];
                k    = 0;
            }
            if (bits & 0x80)
            {
                *(buf + (y + i) * pitch + x + j) = font_color16b;
            }
            else if (bg_color16b != -1)
            {
                *(buf + (y + i) * pitch + x + j) = bg_color16b;
            }
        }
    }
}

void put_string_chinese(int x, int y, uint16_t *s, int font_color16b, int bg_color16b, font_vizn type, uint16_t *buf, int pitch)
{
    int width;
    // set_d4dfnt_desc(type);
    for (; *s; s++)
    {
        set_d4dfnt_desc_chinese(*s);
        put_char_chinese(x, y, *s, font_color16b, bg_color16b, buf, pitch);
        width = d4dfnt_desc_chinese.charFullSize.width;
        x += width;
    }
}

void put_string_utf8(int x, int y, char *s, int font_color16b, int bg_color16b, font_vizn typeAscII, uint16_t *buf, int pitch)
{
    int width;
    uint16_t unicode = 0;
    int codeLen = 0;
    while(*s)
    {
        if(0 == (s[0] & 0x80))
        {
            //One byte for AscII font
            codeLen = 1;
            set_d4dfnt_desc(typeAscII);
            put_char(x, y, *s, font_color16b, bg_color16b, buf, pitch);
            width = d4dfnt_desc.pSizeTable[(int)*s - d4dfnt_desc.startChar];
            x += width;
            s += codeLen;
        }
#if SUPPORT_CHINESE_FONT
        else if(0xE0 == (s[0] & 0xF0) && 0x80 == (s[1] & 0xC0) && 0x80 == (s[2] & 0xC0))
        {
            // Three bytes for Chinese font
            codeLen = 3;
            unicode = (uint16_t)((((uint16_t)s[0] & 0x000F) << 12) | (((uint16_t)s[1] & 0x003F) << 6) | ((uint16_t)s[2] & 0x003F));
            set_d4dfnt_desc_chinese(unicode);
            put_char_chinese(x, y, unicode, font_color16b, bg_color16b, buf, pitch);
            width = d4dfnt_desc_chinese.charFullSize.width;
            x += width;
            s += codeLen;
        }
#endif
        else
        {
            //Others, not support yet!
        }
    }
}