#include "font/font_all.h"
#include "font/language_list.h"

extern void platform_putchar(struct font_info *info, u8 *pixel, u16 width, u16 height, u16 x, u16 y);
/* 语言字库配置 */
#define LANGUAGE  BIT(Chinese_Simplified)|\
	              BIT(English)

#if LANGUAGE&BIT(Chinese_Simplified)
REGISTER_LANGUAGE_BEGIN(Chinese_Simplified) = {
    .language_id = Chinese_Simplified,
    .flags = FONT_SHOW_PIXEL | FONT_SHOW_MULTI_LINE,
    .pixel.file.name = (char *)"mnt/spiflash/res/F_GB2312.PIX",
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_ASCII.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_GB2312.TAB",
    .isgb2312 = true,
    .bigendian = false,
    .putchar = platform_putchar,

    /* .language_id = Chinese_Simplified, */
    /* .flags = FONT_SHOW_PIXEL, */
    /* .pixel.file.name = (char *)"mnt/spiflash/res/F_GBK.PIX", */
    /* .ascpixel.file.name = (char *)"mnt/spiflash/res/F_ASCII.PIX", */
    /* .tabfile.name = (char *)"mnt/spiflash/res/F_GBK.TAB", */
    /* .isgb2312 = false, */
    /* .putchar = platform_putchar, */
};
#endif

#if LANGUAGE&BIT(Chinese_Traditional)
REGISTER_LANGUAGE_BEGIN(Chinese_Traditional) = {
    .language_id = Chinese_Traditional,
    .flags = FONT_SHOW_PIXEL,
    .pixel.file.name = (char *)"mnt/spiflash/res/F_BIG5.PIX",
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_ASCII.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_BIG5.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Japanese)
REGISTER_LANGUAGE_BEGIN(Japanese) = {
    .language_id = Japanese,
    .flags = FONT_SHOW_PIXEL,
    .pixel.file.name = (char *)"mnt/spiflash/res/F_SJIS.PIX",
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_SJISASC.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_SJIS.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Korean)
REGISTER_LANGUAGE_BEGIN(Korean) = {
    .language_id = Korean,
    .flags = FONT_SHOW_PIXEL,
    .pixel.file.name = (char *)"mnt/spiflash/res/F_KSC.PIX",
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_ASCII.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_KSC.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(English)
REGISTER_LANGUAGE_BEGIN(English) = {
    .language_id = English,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1252.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1252.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(French)
REGISTER_LANGUAGE_BEGIN(French) = {
    .language_id = French,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1252.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1252.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(German)
REGISTER_LANGUAGE_BEGIN(German) = {
    .language_id = German,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1252.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1252.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Italian)
REGISTER_LANGUAGE_BEGIN(Italian) = {
    .language_id = Italian,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1252.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1252.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Dutch)
REGISTER_LANGUAGE_BEGIN(Dutch) = {
    .language_id = Dutch,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1252.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1252.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Portuguese)
REGISTER_LANGUAGE_BEGIN(Portuguese) = {
    .language_id = Portuguese,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1252.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1252.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Spanish)
REGISTER_LANGUAGE_BEGIN(Spanish) = {
    .language_id = Spanish,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1252.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1252.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Swedish)
REGISTER_LANGUAGE_BEGIN(Swedish) = {
    .language_id = Swedish,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1252.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1252.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Czech)
REGISTER_LANGUAGE_BEGIN(Czech) = {
    .language_id = Czech,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1250.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1250.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Danish)
REGISTER_LANGUAGE_BEGIN(Danish) = {
    .language_id = Danish,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1252.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1252.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Polish)
REGISTER_LANGUAGE_BEGIN(Polish) = {
    .language_id = Polish,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1250.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1250.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Russian)
REGISTER_LANGUAGE_BEGIN(Russian) = {
    .language_id = Russian,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1251.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1251.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Turkey)
REGISTER_LANGUAGE_BEGIN(Turkey) = {
    .language_id = Turkey,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1254.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1254.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Hebrew)
REGISTER_LANGUAGE_BEGIN(Hebrew) = {
    .language_id = Hebrew,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1255.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1255.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Thai)
REGISTER_LANGUAGE_BEGIN(Thai) = {
    .language_id = Thai,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP874.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP874.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Hungarian)
REGISTER_LANGUAGE_BEGIN(Hungarian) = {
    .language_id = Hungarian,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1250.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1250.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Romanian)
REGISTER_LANGUAGE_BEGIN(Romanian) = {
    .language_id = Romanian,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1250.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1250.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif

#if LANGUAGE&BIT(Arabic)
REGISTER_LANGUAGE_BEGIN(Arabic) = {
    .language_id = Arabic,
    .flags = FONT_SHOW_PIXEL,
    .ascpixel.file.name = (char *)"mnt/spiflash/res/F_CP1256.PIX",
    .tabfile.name = (char *)"mnt/spiflash/res/F_CP1256.TAB",
    .bigendian = false,
    .putchar = platform_putchar,
};
#endif
