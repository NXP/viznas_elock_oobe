/*
 * Copyright 2019 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 *
 */

#ifndef _VIDEO_FONT_H
#define _VIDEO_FONT_H

/*******************************************************************************
 * Definitions
 *******************************************************************************/
#define SUPPORT_CHINESE_FONT  1

typedef enum
{
    OPENSANS8  = 1, // OpenSans8 font height=15 widthMax=10
    OPENSANS16 = 2, // OpenSans16 font height=28 widthMax=20
    CAMBO10    = 3,
    CAMBO18    = 4,
    RALEWAY10  = 5,
    RALEWAY16  = 6
} font_vizn;

/*******************************************************************************
 * API
 *******************************************************************************/
int get_stringwidth(char *s, font_vizn type);
int get_fontheight(font_vizn type);
void put_string(int x, int y, char *s, int font_color16b, int bg_color16b, font_vizn type, uint16_t *buf, int pitch);
void put_string_chinese(int x, int y, uint16_t *s, int font_color16b, int bg_color16b, font_vizn type, uint16_t *buf, int pitch);
void put_string_utf8(int x, int y, char *s, int font_color16b, int bg_color16b, font_vizn typeAscII, uint16_t *buf, int pitch);
#endif /* _VIDEO_FONT_H */
