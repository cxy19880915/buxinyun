#include "ui/includes.h"
#include "system/includes.h"
#include "style.h"
#include "action.h"
#include "app_config.h"
#include "storage_device.h"
#include "res.h"
#include "app_database.h"
#include "sys_set.h"

#ifdef CONFIG_UI_STYLE_LY_ENABLE

#define STYLE_NAME  LY

REGISTER_UI_STYLE(STYLE_NAME)


/*
 * (begin)ÊèêÁ§∫Ê°ÜÊòæÁ§∫Êé•Âè£
 */

static u8 msg_show_f = 0;
static void __sys_msg_hide(void)
{
    if (msg_show_f) {
        msg_show_f = 0;
        ui_hide(LAYER_MY_SYS_MSG);
    }
}
static void __sys_msg_timeout_func(void *priv)
{
    __sys_msg_hide();
}

static void __sys_set_msg_show(u32 id, u32 timeout_msec)
{	
    if (msg_show_f == 0) {
        ui_show(LAYER_MY_SYS_MSG);
		ui_show(id);
        msg_show_f = 1;
        if (timeout_msec > 0) {
            sys_timeout_add(NULL, __sys_msg_timeout_func, timeout_msec);
        }
    } else {
    	ui_show(id);
        if (timeout_msec > 0) {
            sys_timeout_add(NULL, __sys_msg_timeout_func, timeout_msec);
        }
    }
}
/*
 * (end)
 */
 

u8 meun_id = 0;

/***********************************************************************************/
/*							 ∑÷±Ê¬ ≤Àµ•…Ë÷√										   */
/***********************************************************************************/
//∑÷±Ê¬ 1º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
static int text_sys_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, db_select("res"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TXT_SYS_SET_REC)
.onchange = text_sys_rec_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
//∑÷±Ê¬ 2º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
u32 pic_rec_id[] = 
{
	PIC_REC_1,
	PIC_REC_2,
	PIC_REC_3
};
static int menu_sys_set_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
	struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("res"));
        ui_show(pic_rec_id[db_select("res")]);
        break;
	case ON_CHANGE_RELEASE:
		break;
    default:
        return FALSE;
    }
    return FALSE;
}
//∑÷±Ê¬ 2º∂∞¥º¸≤Ÿ◊˜
static int menu_sys_set_rec_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
    struct ui_grid *grid = (struct ui_grid *)_grid;
	struct intent it;
	int err;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
		init_intent(&it);
        it.name	= "sys_set";
        it.action = ACTION_SYS_SET_CONFIG;
        it.data = "res";
		it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("res onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_SYS_SET_REC);
		ui_text_show_index_by_id(TXT_SYS_SET_REC, sel_item);
        break;
    case KEY_MODE:
		ui_hide(LAYOUT_SYS_SET_REC);
        break;
    default:
        return false;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_1)
	.onkey = menu_sys_set_rec_onkey,
	.onchange = menu_sys_set_rec_onchange,
 	.ontouch = NULL,
};

/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/


/***********************************************************************************/
/*							 —≠ª∑¬º”∞≤Àµ•…Ë÷√									   */
/***********************************************************************************/
//—≠ª∑¬º”∞1º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
static const u8 table_video_cycle[] = {
    0,
    3,
    5,
    10,
};

static int text_sys_cyc_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, index_of_table8(db_select("cyc"), TABLE(table_video_cycle)));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TXT_SYS_SET_CYC)
.onchange = text_sys_cyc_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
//—≠ª∑¬º”∞2º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
u32 pic_cyc_id[] = 
{
	PIC_CYC_1,
	PIC_CYC_2,
	PIC_CYC_3,
	PIC_CYC_4,
};
static int menu_sys_set_cyc_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
	struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, index_of_table8(db_select("cyc"), TABLE(table_video_cycle)));
        ui_show(pic_cyc_id[index_of_table8(db_select("cyc"), TABLE(table_video_cycle))]);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
//—≠ª∑¬º”∞2º∂∞¥º¸≤Ÿ◊˜
static int menu_sys_set_cyc_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
    struct ui_grid *grid = (struct ui_grid *)_grid;
	struct intent it;
	int err;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
		init_intent(&it);
        it.name	= "sys_set";
        it.action = ACTION_SYS_SET_CONFIG;
        it.data = "cyc";
		it.exdata = table_video_cycle[sel_item];
        err = start_app(&it);
        if (err) {
            printf("res onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_SYS_SET_CYC);
		ui_text_show_index_by_id(TXT_SYS_SET_CYC, sel_item);
		
        break;
    case KEY_MODE:
		ui_hide(LAYOUT_SYS_SET_CYC);
        break;
    default:
        return false;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_3)
	.onkey = menu_sys_set_cyc_onkey,
	.onchange = menu_sys_set_cyc_onchange,
 	.ontouch = NULL,
};

/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/


/***********************************************************************************/
/*							 ‘À––ºÏ≤‚≤Àµ•…Ë÷√									   */
/***********************************************************************************/
//‘À––ºÏ≤‚1º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ

static int pic_sys_min_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic, db_select("mot"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(PIC_SYS_SET_MIN)
	.onchange = pic_sys_min_onchange,
 	.onkey = NULL,
  	.ontouch = NULL,
};

/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/

/***********************************************************************************/
/*							 ¬º“Ù≤Àµ•…Ë÷√									   */
/***********************************************************************************/
//¬º“Ù1º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ

static int pic_sys_mic_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic, db_select("mic"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(PIC_SYS_SET_MIC)
	.onchange = pic_sys_mic_onchange,
 	.onkey = NULL,
  	.ontouch = NULL,
};

/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/

/***********************************************************************************/
/*							 »’∆⁄ÀÆ”°≤Àµ•…Ë÷√									   */
/***********************************************************************************/
//»’∆⁄ÀÆ”°1º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ

static int pic_sys_data_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic, db_select("dat"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(PIC_SYS_SET_DATA)
	.onchange = pic_sys_data_onchange,
 	.onkey = NULL,
  	.ontouch = NULL,
};

/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/


/***********************************************************************************/
/*							  ÷ÿ¡¶∏–”¶≤Àµ•…Ë÷√									   */
/***********************************************************************************/
//÷ÿ¡¶∏–”¶1º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
static const u8 table_video_gravity[] = {
    GRA_SEN_OFF,
    GRA_SEN_LO,
    GRA_SEN_MD,
    GRA_SEN_HI,
};


static int text_sys_gsn_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, index_of_table8(db_select("gra"), TABLE(table_video_gravity)));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TXT_SYS_SET_GEN)
	.onchange = text_sys_gsn_onchange,
 	.onkey = NULL,
  	.ontouch = NULL,
};

//—≠ª∑¬º”∞2º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
u32 pic_gsn_id[] = 
{
	PIC_GSN_1,
	PIC_GSN_2,
	PIC_GSN_3,
	PIC_GSN_4,
};
static int menu_sys_set_gsn_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
	struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, index_of_table8(db_select("gra"), TABLE(table_video_gravity)));
        ui_show(pic_gsn_id[index_of_table8(db_select("gra"), TABLE(table_video_gravity))]);
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
//—≠ª∑¬º”∞2º∂∞¥º¸≤Ÿ◊˜
static int menu_sys_set_gsn_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
    struct ui_grid *grid = (struct ui_grid *)_grid;
	struct intent it;
	int err;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
		init_intent(&it);
        it.name	= "sys_set";
        it.action = ACTION_SYS_SET_CONFIG;
        it.data = "gra";
		it.exdata = table_video_gravity[sel_item];
        err = start_app(&it);
        if (err) {
            printf("res onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_SET_PHOTO_GSN);
		ui_text_show_index_by_id(TXT_SYS_SET_GEN, sel_item);
		
        break;
    case KEY_MODE:
		ui_hide(LAYOUT_SET_PHOTO_GSN);
        break;
    default:
        return false;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_16)
	.onkey = menu_sys_set_gsn_onkey,
	.onchange = menu_sys_set_gsn_onchange,
 	.ontouch = NULL,
};

/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/

#if 1

static const u16 province_gb2312[] = {
    0xA9BE, 0xFEC4, 0xA8B4, 0xA6BB, 0xF2BD, //æ©£¨ƒ˛£¨¥®£¨ª¶£¨ΩÚ
    0xE3D5, 0xE5D3, 0xE6CF, 0xC1D4, 0xA5D4, //’„£¨”Â£¨œÊ£¨‘¡£¨‘•
    0xF3B9, 0xD3B8, 0xC9C1, 0xB3C2, 0xDABA, //πÛ£¨∏”£¨¡…£¨¬≥£¨∫⁄
    0xC2D0, 0xD5CB, 0xD8B2, 0xF6C3, 0xFABD, //–¬£¨À’£¨≤ÿ£¨√ˆ£¨Ω˙
    0xEDC7, 0xBDBC, 0xAABC, 0xF0B9, 0xCAB8, //«Ì£¨ºΩ£¨º™£¨π£¨∏ £¨
    0xEECD, 0xC9C3, 0xF5B6, 0xC2C9, 0xE0C7, //ÕÓ£¨√…£¨∂ı£¨…¬£¨«‡£¨
    0xC6D4                                  //‘∆
};

static const u8 num_table[] = {
    'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y',
    'Z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8',
    '9'
};


struct car_num {
    const char *mark;
    u32 text_id;
    u32 text_index;
};

struct car_num_str {
    u8 province;
    u8 town;
    u8 a;
    u8 b;
    u8 c;
    u8 d;
    u8 e;
};

struct car_num text_car_num_table[] = {
    {"province", TEXT_MN_CN_PROVINCE, 0}, /* æ© */
    {"town",     TEXT_MN_CN_TOWN,     0}, /* A */
    {"a",        TEXT_MN_CN_A,        0}, /* 1 */
    {"b",        TEXT_MN_CN_B,        0}, /* 2 */
    {"c",        TEXT_MN_CN_C,        0}, /* 3 */
    {"d",        TEXT_MN_CN_D,        0}, /* 4 */
    {"e",        TEXT_MN_CN_E,        0}, /* 5 */
};

enum sw_dir {
    /*
     * «–ªª∑ΩœÚ
     */
    DIR_NEXT = 1,
    DIR_PREV,
    DIR_SET,
};
enum set_mod {
    /*
     * º”ºı∑ΩœÚ
     */
    MOD_ADD = 1,
    MOD_DEC,
    MOD_SET,
};
static u16 car_num_set_p = 0xff;
#endif


static int text_sys_num_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, db_select("num"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TXT_SYS_SET_CAR)
	.onchange = text_sys_num_onchange,
 	.onkey = NULL,
  	.ontouch = NULL,
};


#if 0
static int video_layout_down_onchange(void *ctr, enum element_change_event e, void *arg)
{
    u32 a, b;

    switch (e) {
    case ON_CHANGE_INIT:
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        /*
         * ‘⁄¥ÀªÒ»°ƒ¨»œ“˛≤ÿµƒÕº±Íµƒ◊¥Ã¨≤¢œ‘ æ
         */
        a = db_select("cna");
        b = db_select("cnb");
        text_car_num_table[0].text_index = index_of_table16(a >> 16, TABLE(province_gb2312));
        text_car_num_table[1].text_index  = index_of_table8((a >> 8) & 0xff, TABLE(num_table));
        text_car_num_table[2].text_index  = index_of_table8((a >> 0) & 0xff, TABLE(num_table));
        text_car_num_table[3].text_index  = index_of_table8((b >> 24) & 0xff, TABLE(num_table));
        text_car_num_table[4].text_index  = index_of_table8((b >> 16) & 0xff, TABLE(num_table));
        text_car_num_table[5].text_index  = index_of_table8((b >> 8) & 0xff, TABLE(num_table));
        text_car_num_table[6].text_index  = index_of_table8((b >> 0) & 0xff, TABLE(num_table));

        if (db_select("num")) {
            ui_text_show_index_by_id(TEXT_CAR_PROVINCE, text_car_num_table[0].text_index);
            ui_text_show_index_by_id(TEXT_CAR_TOWN, text_car_num_table[1].text_index);
            ui_text_show_index_by_id(TEXT_CAR_A,    text_car_num_table[2].text_index);
            ui_text_show_index_by_id(TEXT_CAR_B,    text_car_num_table[3].text_index);
            ui_text_show_index_by_id(TEXT_CAR_C,    text_car_num_table[4].text_index);
            ui_text_show_index_by_id(TEXT_CAR_D,    text_car_num_table[5].text_index);
            ui_text_show_index_by_id(TEXT_CAR_E,    text_car_num_table[6].text_index);
        }
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_DOWN_REC)
.onchange = video_layout_down_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif

#if 1
/***************************** ≥µ≈∆∫≈¬Îø™πÿ…Ë÷√ ************************************/
static int menu_car_num_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, db_select("num"));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

static int menu_car_num_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    const char *str;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);

        init_intent(&it);
        it.name	= "sys_set";
        it.action = ACTION_SYS_SET_CONFIG;
        it.data = "num";
        it.exdata = sel_item;
        err = start_app(&it);
        if (err) {
            printf("res car num err! %d\n", err);
            break;
        }

        ui_hide(LAYOUT_MN_NUM_REC);
        if (sel_item) {
            ui_show(LAYOUT_MN_CARNUM_SET_REC);
        }
		else
		{
			ui_show(LAY_SYS_SET_REC);
		}
        break;
    case KEY_DOWN:
        return FALSE;

        break;
    case KEY_UP:
        return FALSE;

        break;
    case KEY_MODE:
    case KEY_MENU:
        ui_hide(LAYOUT_MN_NUM_REC);
		ui_show(LAY_SYS_SET_REC);
        break;
    default:
        return false;
        break;
    }

    return true;
}


REGISTER_UI_EVENT_HANDLER(VLIST_NUM_REC)
.onchange = menu_car_num_onchange,
 .onkey = menu_car_num_onkey,
  .ontouch = NULL,
};

/***************************** ≥µ≈∆∫≈¬Î…Ë÷√ ************************************/

static u16 __car_num_search_by_mark(const char *mark)
{
    u16 p = 0;
    u16 table_sum = sizeof(text_car_num_table) / sizeof(struct car_num);
    while (p < table_sum) {
        if (!strcmp(mark, text_car_num_table[p].mark)) {
            return p;
        }
        p++;
    }
    return -1;
}
static u8 __car_num_reset(void)
{
    car_num_set_p = 0xff;
    return 0;
}
static u8 __car_num_switch(enum sw_dir dir, const char *mark)
{
    u16 table_sum;
    u16 prev_set_p = car_num_set_p;
    u16 p;

    table_sum = sizeof(text_car_num_table) / sizeof(struct car_num);

    ASSERT(dir == DIR_NEXT || dir == DIR_PREV || dir == DIR_SET);
    switch (dir) {
    case DIR_NEXT:
        if (car_num_set_p >= (table_sum - 1)) {
            car_num_set_p = 0;
        } else {
            car_num_set_p++;
        }
        break;
    case DIR_PREV:
        if (car_num_set_p == 0) {
            car_num_set_p = (table_sum - 1);
        } else {
            car_num_set_p--;
        }
        break;
    case DIR_SET:
        p = __car_num_search_by_mark(mark);
        if (p == (u16) - 1) {
            return -1;
        }
        car_num_set_p = p;

        break;
    }
    if (prev_set_p != 0xff) {
        ui_no_highlight_element_by_id(text_car_num_table[prev_set_p].text_id);
    }
    ui_highlight_element_by_id(text_car_num_table[car_num_set_p].text_id);

    return 0;

}
static u8 __car_num_update_show(u8 pos) /* ∏¸–¬µ•∏ˆ ±º‰øÿº˛µƒ ±º‰ */
{
    ui_text_show_index_by_id(text_car_num_table[pos].text_id, text_car_num_table[pos].text_index);
    return 0;
}
static u8 __car_num_set_value(enum set_mod mod, u16 value)
{

    u8 p;
    ASSERT(mod == MOD_ADD || mod == MOD_DEC || mod == MOD_SET);
    switch (mod) {
    case MOD_ADD:
        switch (car_num_set_p) {
        case 0: /* province */
            value = text_car_num_table[car_num_set_p].text_index + 1;
            if (value >= 31) {
                value = 0;
            }
            break;
        case 1: /* town */
        case 2: /* a */
        case 3: /* b */
        case 4: /* c */
        case 5: /* d */
        case 6: /* e */
            value = text_car_num_table[car_num_set_p].text_index + 1;
            if (value >= (('Z' - 'A' + 1) + ('9' - '0' + 1))) {
                value = 0;
            }
            break;
        default:
            ASSERT(0, "mod_add car_num_set_p:%d err!", car_num_set_p);
            break;
        }
        break;
    case MOD_DEC:
        switch (car_num_set_p) {
        case 0: /* province */
            value = text_car_num_table[car_num_set_p].text_index;
            if (value == 0) {
                value = 30;
            } else {
                value--;
            }
            break;
        case 1: /* town */
        case 2: /* a */
        case 3: /* b */
        case 4: /* c */
        case 5: /* d */
        case 6: /* e */
            value = text_car_num_table[car_num_set_p].text_index;
            if (value == 0) {
                value = ('Z' - 'A' + 1) + ('9' - '0' + 1) - 1;
            } else {
                value--;
            }
            break;
        default:
            ASSERT(0, "mod_dec car_num_set_p:%d err!", car_num_set_p);
            break;
        }
        break;
    case MOD_SET:
        switch (car_num_set_p) {
        case 0: /* province */
            ASSERT(value >= 0 && value <= 30, "car num set value err!\n");
            break;
        case 1: /* town */
        case 2: /* a */
        case 3: /* b */
        case 4: /* c */
        case 5: /* d */
        case 6: /* e */
            ASSERT(value >= 0 && value <= (('Z' - 'A' + 1) + ('9' - '0' + 1) - 1));
            break;
        default:
            ASSERT(0, "mod_set car_num_set_p:%d err!", car_num_set_p);
            break;
        }
        break;
    }
    text_car_num_table[car_num_set_p].text_index = value;

    printf("car_num_set p:%d, value:%d\n", car_num_set_p, value);
    __car_num_update_show(car_num_set_p); /* ∏¸–¬µ±«∞Œªœ‘ æ */

    return 0;
}

static u8 __car_num_get(struct car_num_str *num)
{
    num->province = text_car_num_table[0].text_index;
    num->town = text_car_num_table[1].text_index;
    num->a = text_car_num_table[2].text_index;
    num->b = text_car_num_table[3].text_index;
    num->c = text_car_num_table[4].text_index;
    num->d = text_car_num_table[5].text_index;
    num->e = text_car_num_table[6].text_index;
    return 0;
}

static u8 __car_num_set_by_ascii(const char *str)
{
    u16 i;
    u16 province;
    /* ASSERT(strlen(str) == 8, "car num err!"); */

    memcpy((char *)&province, str, 2);
    text_car_num_table[0].text_index = 0;
    for (i = 0; i < (sizeof(province_gb2312) / sizeof(u16)); i++) {
        if (province_gb2312[i] == province) {
            text_car_num_table[0].text_index = i;
            break;
        }
    }
    text_car_num_table[1].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[2]) {
            text_car_num_table[1].text_index = i;
            break;
        }
    }
    text_car_num_table[2].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[3]) {
            text_car_num_table[2].text_index = i;
            break;
        }
    }
    text_car_num_table[3].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[4]) {
            text_car_num_table[3].text_index = i;
            break;
        }
    }
    text_car_num_table[4].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[5]) {
            text_car_num_table[4].text_index = i;
            break;
        }
    }
    text_car_num_table[5].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[6]) {
            text_car_num_table[5].text_index = i;
            break;
        }
    }
    text_car_num_table[6].text_index = 0;
    for (i = 0; i < sizeof(num_table); i++) {
        if (num_table[i] == str[7]) {
            text_car_num_table[6].text_index = i;
            break;
        }
    }
    return 0;

}
int menu_rec_car_num_set(void)
{
    struct intent it;
    int err;
    struct car_num_str num;

    __car_num_get(&num);
    u32 part_a = (province_gb2312[num.province] << 16) |
                 (num_table[num.town] << 8) | num_table[num.a];
    u32 part_b = (num_table[num.b] << 24) | (num_table[num.c] << 16) |
                 (num_table[num.d] << 8)  |  num_table[num.e];

    init_intent(&it);
    //it.name	= "video_rec";
    //it.action = ACTION_VIDEO_REC_SET_CONFIG;
	it.name	= "sys_set";
    it.action = ACTION_SYS_SET_CONFIG;
    it.data = "cna";
    it.exdata = part_a;
    err = start_app(&it);
    if (err) {
        printf("num set onkey err! %d\n", err);
        return -1;
    }
    it.data = "cnb";
    it.exdata = part_b;
    err = start_app(&it);

    return 0;
}

static int menu_layout_car_num_set_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        layout_on_focus(layout);
        __car_num_reset();
        break;
    case ON_CHANGE_RELEASE:
        layout_lose_focus(layout);
        break;
    case ON_CHANGE_FIRST_SHOW:
        __car_num_switch(DIR_SET, "province");
        break;
    default:
        return false;
    }
    return false;
}
static int menu_layout_car_num_set_onkey(void *ctr, struct element_key_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    struct intent it;
    int sel_item = 0;
    int err;
    const char *str;

    switch (e->value) {
    case KEY_OK:
        __car_num_switch(DIR_NEXT, NULL);

        break;
    case KEY_DOWN:
        __car_num_set_value(MOD_ADD, 0);
        return FALSE;

        break;
    case KEY_UP:
        __car_num_set_value(MOD_DEC, 0);
        return FALSE;

        break;
    case KEY_MODE:
    case KEY_MENU:
        ui_hide(LAYOUT_MN_CARNUM_SET_REC);
        menu_rec_car_num_set();
		ui_show(LAY_SYS_SET_REC);
        break;
    default:
        return false;
        break;
    }

    return true;
}

REGISTER_UI_EVENT_HANDLER(LAYOUT_MN_CARNUM_SET_REC)
.onchange = menu_layout_car_num_set_onchange,
 .onkey = menu_layout_car_num_set_onkey,
  .ontouch = NULL,
};




/*
 * (begin)≤Àµ•÷–≥µ≈∆∫≈¬Îµƒœ‘ æ≥ı ºªØ∂Ø◊˜ ********************************************
 */
static int text_menu_car_num_province_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[0].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_PROVINCE)
.onchange = text_menu_car_num_province_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_town_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[1].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_TOWN)
.onchange = text_menu_car_num_town_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_a_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[2].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_A)
.onchange = text_menu_car_num_a_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_b_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[3].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_B)
.onchange = text_menu_car_num_b_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
static int text_menu_car_num_c_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[4].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_C)
.onchange = text_menu_car_num_c_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int text_menu_car_num_d_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[5].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_D)
.onchange = text_menu_car_num_d_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

static int text_menu_car_num_e_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, text_car_num_table[6].text_index);
        break;
    case ON_CHANGE_RELEASE:
        break;
    case ON_CHANGE_FIRST_SHOW:
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TEXT_MN_CN_E)
.onchange = text_menu_car_num_e_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};
#endif



static const int menu_rec_items[] = {
    LAYOUT_SYS_SET_REC,
	LAYOUT_SYS_SET_CYC,	
};

static int menu_sys_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
	struct intent it;
	int err;
	
    struct ui_grid *grid = (struct ui_grid *)_grid;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
		printf("sys_set_KEY_OK sel_item = %d\n", sel_item);
		#if 0
        ASSERT(sel_item < (sizeof(menu_rec_items) / sizeof(int)));
        if (menu_rec_items[sel_item] == LAYOUT_MN_FORMAT_SYS) {
            if (dev_online(SDX_DEV) == false && storage_device_ready() == 0) {
                printf("format sd offline !\n");
                __sys_set_msg_show(BOX_MSG_INSERT_SD, 2000);

                return TRUE;
            }
        }
		#endif
		switch(sel_item)
		{
			case 0:
			case 1:
				ui_show(menu_rec_items[sel_item]);
				break;
			case 5:
				ui_show(LAYOUT_SET_PHOTO_GSN);
				break;
			case 6:
				
				ui_hide(LAY_SYS_SET_REC);	
				ui_show(LAYOUT_MN_NUM_REC);
				break;
			case 2: //‘À∂ØºÏ≤‚
				init_intent(&it);
		        it.name	= "sys_set";
		        it.action = ACTION_SYS_SET_CONFIG;
				
		        it.data = "mot";
				if(db_select("mot"))
				{
					it.exdata = 0;
				}
				else
				{
					it.exdata = 1;	
				}
		        err = start_app(&it);
		        if (err) {
		            printf("res onkey err! %d\n", err);
		            break;
		        }	
				ui_pic_show_image_by_id( PIC_SYS_SET_MIN,it.exdata);
				break;
		
			case 3: //¬º“Ù
				init_intent(&it);
		        it.name	= "sys_set";
		        it.action = ACTION_SYS_SET_CONFIG;
		        it.data = "mic";
				if(db_select("mic"))
				{
					it.exdata = 0;
				}
				else
				{
					it.exdata = 1;
				}
		        err = start_app(&it);
		        if (err) {
		            printf("res onkey err! %d\n", err);
		            break;
		        }	
				ui_pic_show_image_by_id( PIC_SYS_SET_MIC,it.exdata);
				break;
			case 4: // ±º‰ÀÆ”°
				init_intent(&it);
		        it.name	= "sys_set";
		        it.action = ACTION_SYS_SET_CONFIG;
		        it.data = "dat";
				if(db_select("dat"))
				{
					it.exdata = 0;
				}
				else
				{
					it.exdata = 1;
				}
		        err = start_app(&it);
		        if (err) {
		            printf("res onkey err! %d\n", err);
		            break;
		        }	
				ui_pic_show_image_by_id( PIC_SYS_SET_DATA,it.exdata);
				break;
			default:
				break;
		}
        break;
    case KEY_DOWN:
		puts("sys_set_KEY_DOWN\n");
        return FALSE;

        break;
    case KEY_UP:
		puts("sys_set_KEY_UP\n");
        return FALSE;

        break;
    case KEY_MENU:
        //sys_exit_menu_post();
        break;
    case KEY_MODE:
		printf("=====menu_sys_onkey KEY_MODE===\n");
		if(meun_id == 1)
		{
			meun_id = 2;
			ui_hide(LAY_SYS_SET_REC);
			ui_show(LAY_SYS_SET_PHOTO);
		}
        return true;
    default:
        return false;
        break;
    }

    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_2)
	.onkey = menu_sys_onkey,
 	.ontouch = NULL,
};


/***********************************************************************************/
/*							 ≈ƒ’’…Ë÷√≤Àµ•…Ë÷√										   */
/***********************************************************************************/
static const u8 table_photo_delay_mode_camera0[] = {
    0,
    2,
    5,
    10,
};

//∂® ±≈ƒ’’1º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
static int text_sys_photo_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, index_of_table8(db_select("phm"), TABLE(table_photo_delay_mode_camera0)));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TXT_SYS_SET_PHOTO)
	.onchange = text_sys_photo_onchange,
 	.onkey = NULL,
  	.ontouch = NULL,
};

//∂® ±≈ƒ’’2º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
u32 pic_photo_id[] = 
{
	PIC_PHOTO_1,
	PIC_PHOTO_2,
	PIC_PHOTO_3,
	PIC_PHOTO_4
};
static int menu_sys_set_photo_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
	struct ui_grid *grid = (struct ui_grid *)ctr;

    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, index_of_table8(db_select("phm"), TABLE(table_photo_delay_mode_camera0)));
        ui_show(pic_photo_id[index_of_table8(db_select("phm"), TABLE(table_photo_delay_mode_camera0))]);
        break;
	case ON_CHANGE_RELEASE:
		break;
    default:
        return FALSE;
    }
    return FALSE;
}
//∂® ±≈ƒ’’2º∂∞¥º¸≤Ÿ◊˜
static int menu_sys_set_photo_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
    struct ui_grid *grid = (struct ui_grid *)_grid;
	struct intent it;
	int err;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
		init_intent(&it);
        it.name	= "sys_set";
        it.action = ACTION_SYS_SET_PHOTO_CONFIG;
        it.data = "phm";
		it.exdata = table_photo_delay_mode_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            printf("res onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_SYS_SET_PHOTO);
		ui_text_show_index_by_id(TXT_SYS_SET_PHOTO, sel_item);
        break;
    case KEY_MODE:
		ui_hide(LAYOUT_SYS_SET_PHOTO);
        break;
    default:
        return false;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_5)
	.onkey = menu_sys_set_photo_onkey,
	.onchange = menu_sys_set_photo_onchange,
 	.ontouch = NULL,
};

/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/


/***********************************************************************************/
/*							 ≈ƒ’’…Ë÷√≤Àµ•…Ë÷√										   */
/***********************************************************************************/
static const u8 table_photo_res_camera0[] = {
    PHOTO_RES_VGA, 		/* 640*480 */
    PHOTO_RES_1D3M,  	/* 1280*960 */
    PHOTO_RES_2M,		/* 1920*1080 */
    PHOTO_RES_3M, 		/* 2048*1536 */
    PHOTO_RES_5M, 		/* 2592*1944 */
    PHOTO_RES_8M, 		/* 3264*2448 */
    PHOTO_RES_10M, 		/* 3648*2736 */
    PHOTO_RES_12M, 		/* 4032*3024 */
};


//∑÷±Ê¬ 1º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
static int text_sys_photo_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, index_of_table8(db_select("pres"), TABLE(table_photo_res_camera0)));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TXT_SYS_SET_PHOTO_REC)
	.onchange = text_sys_photo_rec_onchange,
 	.onkey = NULL,
  	.ontouch = NULL,
};
//∑÷±Ê¬ 2º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
u32 pic_photo_rec_id[] = 
{
	PIC_PHOTO_REC_1,
	PIC_PHOTO_REC_2,
	PIC_PHOTO_REC_3,
	PIC_PHOTO_REC_4,
	PIC_PHOTO_REC_5,
	PIC_PHOTO_REC_6,
	PIC_PHOTO_REC_7,
	PIC_PHOTO_REC_8
};
static int menu_sys_set_photo_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
	struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, index_of_table8(db_select("pres"), TABLE(table_photo_res_camera0)));
        ui_show(pic_photo_rec_id[index_of_table8(db_select("pres"), TABLE(table_photo_res_camera0))]);
        break;
	case ON_CHANGE_RELEASE:
		break;
    default:
        return FALSE;
    }
    return FALSE;
}
//∑÷±Ê¬ 2º∂∞¥º¸≤Ÿ◊˜
static int menu_sys_set_photo_rec_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
    struct ui_grid *grid = (struct ui_grid *)_grid;
	struct intent it;
	int err;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
		init_intent(&it);
        it.name	= "sys_set";
        it.action = ACTION_SYS_SET_PHOTO_CONFIG;
        it.data = "pres";
		it.exdata = table_photo_res_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            printf("res onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_SET_PHOTO_REC);
		ui_text_show_index_by_id(TXT_SYS_SET_PHOTO_REC, sel_item);
        break;
    case KEY_MODE:
		ui_hide(LAYOUT_SET_PHOTO_REC);
        break;
    default:
        return false;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_6)
	.onkey = menu_sys_set_photo_rec_onkey,
	.onchange = menu_sys_set_photo_rec_onchange,
 	.ontouch = NULL,
};

/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/

static int menu_sys_set_photo_cyt_onchange(void *ctr, enum element_change_event e, void *arg)
{
	struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic, db_select("cyt"));
        break;
	case ON_CHANGE_RELEASE:
		break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(PIC_SYS_SET_PHOTO_CYT)
	.onchange = menu_sys_set_photo_cyt_onchange,
 	.ontouch = NULL,
};

/***********************************************************************************/
/*							 iso…Ë÷√≤Àµ•…Ë÷√										   */
/***********************************************************************************/
static const u16 table_photo_iso_camera0[] = {
    0,
    100,
    200,
    400,
};



//iso1º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
static int text_sys_photo_iso_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, index_of_table16(db_select("iso"), TABLE(table_photo_iso_camera0)));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TXT_SYS_SET_PHOTO_ISO)
	.onchange = text_sys_photo_iso_onchange,
 	.onkey = NULL,
  	.ontouch = NULL,
};
//iso2º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
u32 pic_photo_iso_id[] = 
{
	PIC_PHOTO_ISO_1,
	PIC_PHOTO_ISO_2,
	PIC_PHOTO_ISO_3,
	PIC_PHOTO_ISO_4,
};
static int menu_sys_set_photo_iso_onchange(void *ctr, enum element_change_event e, void *arg)
{
	struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, index_of_table16(db_select("iso"), TABLE(table_photo_iso_camera0)));
        ui_show(pic_photo_iso_id[index_of_table16(db_select("iso"), TABLE(table_photo_iso_camera0))]);
        break;
	case ON_CHANGE_RELEASE:
		break;
    default:
        return FALSE;
    }
    return FALSE;
}
//iso2º∂∞¥º¸≤Ÿ◊˜
static int menu_sys_set_photo_iso_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
    struct ui_grid *grid = (struct ui_grid *)_grid;
	struct intent it;
	int err;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
		init_intent(&it);
        it.name	= "sys_set";
        it.action = ACTION_SYS_SET_PHOTO_CONFIG;
        it.data = "iso";
		it.exdata = table_photo_iso_camera0[sel_item];
        err = start_app(&it);
        if (err) {
            printf("res onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_SET_PHOTO_IS0);
		ui_text_show_index_by_id(TXT_SYS_SET_PHOTO_ISO, sel_item);
        break;
    case KEY_MODE:
		ui_hide(LAYOUT_SET_PHOTO_IS0);
        break;
    default:
        return false;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_7)
	.onkey = menu_sys_set_photo_iso_onkey,
	.onchange = menu_sys_set_photo_iso_onchange,
 	.ontouch = NULL,
};

/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/

static int menu_sys_set_photo_pdat_onchange(void *ctr, enum element_change_event e, void *arg)
{
	struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic, db_select("pdat"));
        break;
	case ON_CHANGE_RELEASE:
		break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(PIC_SYS_SET_PHOTO_DATA)
	.onchange = menu_sys_set_photo_pdat_onchange,
 	.ontouch = NULL,
};


static const int menu_photo_items[] = 
{
	LAYOUT_SYS_SET_PHOTO,  
	LAYOUT_SET_PHOTO_REC,
};

static int menu_photo_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
	struct intent it;
	int err;
	
    struct ui_grid *grid = (struct ui_grid *)_grid;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
		printf("sys_set_KEY_OK sel_item = %d\n", sel_item);
		#if 0
        ASSERT(sel_item < (sizeof(menu_rec_items) / sizeof(int)));
        if (menu_rec_items[sel_item] == LAYOUT_MN_FORMAT_SYS) {
            if (dev_online(SDX_DEV) == false && storage_device_ready() == 0) {
                printf("format sd offline !\n");
                __sys_set_msg_show(BOX_MSG_INSERT_SD, 2000);

                return TRUE;
            }
        }
		#endif
		switch(sel_item)
		{
			case 0:
			case 1:
				ui_show(menu_photo_items[sel_item]);
				break;
			case 3:	
				ui_show(LAYOUT_SET_PHOTO_IS0);
				break;
			
			case 2: //‘À∂ØºÏ≤‚
				init_intent(&it);
		        it.name	= "sys_set";
		        it.action = ACTION_SYS_SET_PHOTO_CONFIG;
				
		        it.data = "cyt";
				if(db_select("cyt"))
				{
					it.exdata = 0;
				}
				else
				{
					it.exdata = 1;	
				}
		        err = start_app(&it);
		        if (err) {
		            printf("res onkey err! %d\n", err);
		            break;
		        }	
				ui_pic_show_image_by_id( PIC_SYS_SET_PHOTO_CYT,it.exdata);
				break;
			case 4: // ±º‰ÀÆ”°
				init_intent(&it);
		        it.name	= "sys_set";
		        it.action = ACTION_SYS_SET_PHOTO_CONFIG;
		        it.data = "pdat";
				if(db_select("pdat"))
				{
					it.exdata = 0;
				}
				else
				{
					it.exdata = 1;
				}
		        err = start_app(&it);
		        if (err) {
		            printf("res onkey err! %d\n", err);
		            break;
		        }	
				ui_pic_show_image_by_id( PIC_SYS_SET_PHOTO_DATA,it.exdata);
				break;
			default:
				break;
		}
        break;
    case KEY_DOWN:
		puts("sys_set_KEY_DOWN\n");
        return FALSE;

        break;
    case KEY_UP:
		puts("sys_set_KEY_UP\n");
        return FALSE;

        break;
    case KEY_MENU:
        //sys_exit_menu_post();
        break;
    case KEY_MODE:
		printf("=====menu_sys_onkey KEY_MODE===\n");
		if(meun_id == 2)
		{
			meun_id = 3;
			ui_hide(LAY_SYS_SET_PHOTO);
			ui_show(LAY_SYS_SET_SET);
		}
        return true;
    default:
        return false;
        break;
    }

    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_4)
	.onkey = menu_photo_onkey,
 	.ontouch = NULL,
};




static int menu_set_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
	struct intent it;
	int err;

    struct ui_grid *grid = (struct ui_grid *)_grid;

    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
		printf("sys_set_KEY_OK sel_item = %d\n", sel_item);
		#if 0
        ASSERT(sel_item < (sizeof(menu_rec_items) / sizeof(int)));
        if (menu_rec_items[sel_item] == LAYOUT_MN_FORMAT_SYS) {
            if (dev_online(SDX_DEV) == false && storage_device_ready() == 0) {
                printf("format sd offline !\n");
                __sys_set_msg_show(BOX_MSG_INSERT_SD, 2000);

                return TRUE;
            }
        }
		#endif
		switch(sel_item)
		{
			case 0:
				ui_show(LAYOUT_SET_SYS_PRO);
				break;
			case 1:
				ui_show(LAYOUT_SET_SYS_AFF);
				break;
			case 2:	
				ui_show(LAYOUT_SET_SYS_FRE);
				break;
			
			case 3: //∞¥º¸…˘“Ù
				init_intent(&it);
		        it.name	= "sys_set";
		        it.action = ACTION_SYS_SET_SYS_CONFIG;
				
		        it.data = "kvo";
				if(db_select("kvo"))
				{
					it.exdata = 0;
				}
				else
				{
					it.exdata = 1;	
				}
		        err = start_app(&it);
		        if (err) {
		            printf("res onkey err! %d\n", err);
		            break;
		        }	
				ui_pic_show_image_by_id( PIC_SYS_SET_SYS_KVO,it.exdata);
				break;
			case 4: //”Ô—‘…Ë÷√
				ui_show(LAYOUT_SET_SYS_LANG);
				break;
			case 5://”∞œÒ–˝◊™
				init_intent(&it);
		        it.name	= "sys_set";
		        it.action = ACTION_SYS_SET_SYS_CONFIG;
				
		        it.data = "rat";
				if(db_select("rat"))
				{
					it.exdata = 0;
				}
				else
				{
					it.exdata = 1;	
				}
		        err = start_app(&it);
		        if (err) {
		            printf("res onkey err! %d\n", err);
		            break;
		        }	
	
				ui_pic_show_image_by_id( PIC_SYS_SET_SYS_ROTATE,it.exdata);
				break;
			case 6://»’∆⁄…Ë÷√
				ui_show(LAYOUT_SET_SYS_TINE);
				break;
			case 7://∏Ò ΩªØ
				if (dev_online(SDX_DEV) == false && storage_device_ready() == 0)
				{
					printf("no card\n");
					__sys_set_msg_show(TEXT_SYS_INSERT_SD, 1000);
					break;
				}
				ui_show(LAYOUT_SET_SYS_FMT);
				break;	
			case 8:  //ƒ¨»œ…Ë÷√
				ui_show(LAYOUT_SET_SYS_DEF);   
				break;
			case 9:  //∞Ê±æ∫≈
				ui_show(LAYOUT_SET_SYS_VES);   
				break;	
			default:
				break;
		}
        break;
    case KEY_DOWN:
		puts("sys_set_KEY_DOWN\n");
        return FALSE;

        break;
    case KEY_UP:
		puts("sys_set_KEY_UP\n");
        return FALSE;

        break;
    case KEY_MENU:
        //sys_exit_menu_post();
        break;
    case KEY_MODE:
		printf("=====menu_sys_onkey KEY_MODE===\n");
		if(meun_id == 3)
		{
			meun_id = 4;
			ui_hide(LAY_SYS_SET_SET);
			ui_show(LAY_SYS_SET_EXET);
		}
        return true;
    default:
        return false;
        break;
    }

    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_8)
	.onkey = menu_set_onkey,
 	.ontouch = NULL,
};

/***********************************************************************************/
/*							 œµÕ≥…Ë÷√										   */
/***********************************************************************************/
static const u16 table_system_lcd_protect[] = {
    0,
    60,
    180,
    300,
};

//∆¡ƒª±£ª§1º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
static int text_sys_pro_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, index_of_table16(db_select("pro"), TABLE(table_system_lcd_protect)));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TXT_SYS_SET_SYS_CREEN)
	.onchange = text_sys_pro_onchange,
 	.onkey = NULL,
  	.ontouch = NULL,
};

//∑÷±Ê¬ 2º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
u32 pic_pro_id[] = 
{
	PIC_SYS_PRO1,
	PIC_SYS_PRO2,
	PIC_SYS_PRO3,
	PIC_SYS_PRO4,
};
static int menu_sys_set_pro_onchange(void *ctr, enum element_change_event e, void *arg)
{
	struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, index_of_table16(db_select("pro"), TABLE(table_system_lcd_protect)));
        //ui_show(pic_pro_id[index_of_table16(db_select("pro"), TABLE(table_system_lcd_protect))]);
		ui_show(PIC_SYS_PRO1);
		break;
	case ON_CHANGE_RELEASE:
		break;
    default:
        return FALSE;
    }
    return FALSE;
}
//∑÷±Ê¬ 2º∂∞¥º¸≤Ÿ◊˜
static int menu_sys_set_pro_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
    struct ui_grid *grid = (struct ui_grid *)_grid;
	struct intent it;
	int err;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
		init_intent(&it);
        it.name	= "sys_set";
        it.action = ACTION_SYS_SET_SYS_CONFIG;
        it.data = "pro";
		it.exdata = table_system_lcd_protect[sel_item];
        err = start_app(&it);
        if (err) {
            printf("res onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_SET_SYS_PRO);
		ui_text_show_index_by_id(TXT_SYS_SET_SYS_CREEN, sel_item);
        break;
    case KEY_MODE:
		ui_hide(LAYOUT_SET_SYS_PRO);
        break;
    default:
        return false;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_10)
	.onkey = menu_sys_set_pro_onkey,
	.onchange = menu_sys_set_pro_onchange,
 	.ontouch = NULL,
};

/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/


/***********************************************************************************/
/*							 œµÕ≥…Ë÷√										   */
/***********************************************************************************/
static const u8 table_system_auto_close[] = {
    0,
    3,
    5,
    10,
};


//∆¡ƒª±£ª§1º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
static int text_sys_aff_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, index_of_table8(db_select("aff"), TABLE(table_system_auto_close)));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TXT_SYS_SET_SYS_AFF)
	.onchange = text_sys_aff_onchange,
 	.onkey = NULL,
  	.ontouch = NULL,
};

//∑÷±Ê¬ 2º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
u32 pic_aff_id[] = 
{
	PIC_SYS_AFF1,
	PIC_SYS_AFF2,
	PIC_SYS_AFF3,
	PIC_SYS_AFF4,
};
static int menu_sys_set_aff_onchange(void *ctr, enum element_change_event e, void *arg)
{
	struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, index_of_table8(db_select("aff"), TABLE(table_system_auto_close)));
        ui_show(pic_aff_id[index_of_table8(db_select("aff"), TABLE(table_system_auto_close))]);
        break;
	case ON_CHANGE_RELEASE:
		break;
    default:
        return FALSE;
    }
    return FALSE;
}
//∑÷±Ê¬ 2º∂∞¥º¸≤Ÿ◊˜
static int menu_sys_set_aff_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
    struct ui_grid *grid = (struct ui_grid *)_grid;
	struct intent it;
	int err;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
		init_intent(&it);
        it.name	= "sys_set";
        it.action = ACTION_SYS_SET_SYS_CONFIG;
        it.data = "aff";
		it.exdata = table_system_auto_close[sel_item];
        err = start_app(&it);
        if (err) {
            printf("res onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_SET_SYS_AFF);
		ui_text_show_index_by_id(TXT_SYS_SET_SYS_AFF, sel_item);
        break;
    case KEY_MODE:
		ui_hide(LAYOUT_SET_SYS_AFF);
        break;
    default:
        return false;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_11)
	.onkey = menu_sys_set_aff_onkey,
	.onchange = menu_sys_set_aff_onchange,
 	.ontouch = NULL,
};

/***********************************************************************************/
/*							 œµÕ≥…Ë÷√										   */
/***********************************************************************************/
static const u8 table_system_led_fre[] = {
    50,
    60,
};



//π‚‘¥∆µ¬ 1º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
static int text_sys_fre_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, index_of_table8(db_select("fre"), TABLE(table_system_led_fre)));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TXT_SYS_SET_SYS_FRE)
	.onchange = text_sys_fre_onchange,
 	.onkey = NULL,
  	.ontouch = NULL,
};

//π‚‘¥∆µ¬ 2º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
u32 pic_fre_id[] = 
{
	PIC_SYS_FRE1,
	PIC_SYS_FRE2,
};
static int menu_sys_set_fre_onchange(void *ctr, enum element_change_event e, void *arg)
{
	struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, index_of_table8(db_select("fre"), TABLE(table_system_led_fre)));
        ui_show(pic_fre_id[index_of_table8(db_select("fre"), TABLE(table_system_led_fre))]);
        break;
	case ON_CHANGE_RELEASE:
		break;
    default:
        return FALSE;
    }
    return FALSE;
}
//π‚‘¥∆µ¬ 2º∂∞¥º¸≤Ÿ◊˜
static int menu_sys_set_fre_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
    struct ui_grid *grid = (struct ui_grid *)_grid;
	struct intent it;
	int err;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
		init_intent(&it);
        it.name	= "sys_set";
        it.action = ACTION_SYS_SET_SYS_CONFIG;
        it.data = "fre";
		it.exdata = table_system_led_fre[sel_item];
        err = start_app(&it);
        if (err) {
            printf("res onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_SET_SYS_FRE);
		ui_text_show_index_by_id(TXT_SYS_SET_SYS_FRE, sel_item);
        break;
    case KEY_MODE:
		ui_hide(LAYOUT_SET_SYS_FRE);
        break;
    default:
        return false;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_12)
	.onkey = menu_sys_set_fre_onkey,
	.onchange = menu_sys_set_fre_onchange,
 	.ontouch = NULL,
};


/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/

/***********************************************************************************/
/*							 œµÕ≥…Ë÷√										   */
/***********************************************************************************/
static const u8 table_system_language[] = {
    Chinese_Simplified,  /* ÁÆÄ‰Ωì‰∏≠Êñá */
    Chinese_Traditional, /* ÁπÅ‰Ωì‰∏≠Êñá */
    Japanese,            /* Êó•Êñá */
    English,             /* Ëã±Êñá */
    French,              /* Ê≥ïÊñá */
};

//”Ô—‘…Ë÷√1º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
static int text_sys_lang_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_text_set_index(text, index_of_table8(db_select("lag"), TABLE(table_system_language)));
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TXT_SYS_SET_SYS_LANG)
	.onchange = text_sys_lang_onchange,
 	.onkey = NULL,
  	.ontouch = NULL,
};

//”Ô—‘…Ë÷√2º∂≤Àµ•”“±ﬂ–≈œ¢œ‘ æ
u32 pic_lang_id[] = 
{
	PIC_SYS_LANG1,
	PIC_SYS_LANG2,
	PIC_SYS_LANG3,
	PIC_SYS_LANG4,
	PIC_SYS_LANG5,
};
static int menu_sys_set_lang_onchange(void *ctr, enum element_change_event e, void *arg)
{
	struct ui_grid *grid = (struct ui_grid *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_grid_set_item(grid, index_of_table8(db_select("lag"), TABLE(table_system_language)));
        ui_show(pic_lang_id[index_of_table8(db_select("lag"), TABLE(table_system_language))]);
        break;
	case ON_CHANGE_RELEASE:
		break;
    default:
        return FALSE;
    }
    return FALSE;
}
//”Ô—‘…Ë÷√2º∂∞¥º¸≤Ÿ◊˜
static int menu_sys_set_lang_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
    struct ui_grid *grid = (struct ui_grid *)_grid;
	struct intent it;
	int err;
    switch (e->value) {
    case KEY_OK:
        sel_item = ui_grid_cur_item(grid);
		init_intent(&it);
        it.name	= "sys_set";
        it.action = ACTION_SYS_SET_SYS_CONFIG;
        it.data = "lag";
		it.exdata = table_system_language[sel_item];
        err = start_app(&it);
        if (err) {
            printf("res onkey err! %d\n", err);
            break;
        }
        ui_hide(LAYOUT_SET_SYS_LANG);
		ui_text_show_index_by_id(TXT_SYS_SET_SYS_LANG, sel_item);
        break;
    case KEY_MODE:
		ui_hide(LAYOUT_SET_SYS_LANG);
        break;
    default:
        return false;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_13)
	.onkey = menu_sys_set_lang_onkey,
	.onchange = menu_sys_set_lang_onchange,
 	.ontouch = NULL,
};


/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/


/***********************************************************************************/
/*							  ±º‰…Ë÷√										   */
/***********************************************************************************/

static void get_sys_time(struct sys_time *time)
{
    void *fd = dev_open("rtc", NULL);
    if (!fd) {
        memset(time, 0, sizeof(*time));
        return;
    }
    dev_ioctl(fd, IOCTL_GET_SYS_TIME, (u32)time);
    dev_close(fd);
}


struct sys_time set_time;
u8 curr_time = 0, last_time;

u32 time_id[] =
{
	TIMER_1,
	TIMER_2,
	TIMER_3,
	TIMER_4,
	TIMER_5,
	TIMER_6,
};

static int timer_sys_rec_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_time *time = (struct ui_time *)ctr;
    struct sys_time sys_time;

    switch (e) {
    case ON_CHANGE_INIT:
        get_sys_time(&sys_time);
		memcpy(&set_time, &sys_time, sizeof(struct sys_time));
        time->year = sys_time.year;
        time->month = sys_time.month;
        time->day = sys_time.day;
        time->hour = sys_time.hour;
        time->min = sys_time.min;
        time->sec = sys_time.sec;
		ui_highlight_element_by_id(TIMER_1);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(TIMER_1)
	.onchange = timer_sys_rec_onchange,
};
REGISTER_UI_EVENT_HANDLER(TIMER_2)
	.onchange = timer_sys_rec_onchange,
};
REGISTER_UI_EVENT_HANDLER(TIMER_3)
	.onchange = timer_sys_rec_onchange,
};
REGISTER_UI_EVENT_HANDLER(TIMER_4)
	.onchange = timer_sys_rec_onchange,
};
REGISTER_UI_EVENT_HANDLER(TIMER_5)
	.onchange = timer_sys_rec_onchange,
};
REGISTER_UI_EVENT_HANDLER(TIMER_6)
	.onchange = timer_sys_rec_onchange,
};


static int menu_sys_set_time_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("time_layout_onchange onfocus.\n");
        layout_on_focus(layout);
        break;
    case ON_CHANGE_RELEASE:
        puts("time_layout_onchange losefocus.\n");
        layout_lose_focus(layout);
        break;
    default:
        break;
    }
    return FALSE;
}

static u8 __is_leap_year(u16 year)
{
    return (((year % 4) == 0 && (year % 100) != 0) || ((year % 400) == 0));
}
static u16 leap_month_table[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static int menu_sys_set_time_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
	void *fd;
	
	struct intent it;
	int err;
    switch (e->value) {
    case KEY_OK:
		last_time = curr_time;
		if(curr_time >= 5)
			curr_time = 0;
		else
			curr_time++;

		ui_no_highlight_element_by_id(time_id[last_time]);
		ui_highlight_element_by_id(time_id[curr_time]);
        break;
    case KEY_MODE:
		fd = dev_open("rtc", NULL);
	    if (!fd) {
	        return 0;
	    }
	    dev_ioctl(fd, IOCTL_SET_SYS_TIME, (u32)&set_time);
	    dev_close(fd);
		ui_hide(LAYOUT_SET_SYS_TINE);
        break;
	case KEY_UP:
		switch(curr_time)
		{
			case 0:  //ƒÍ
				set_time.year++;
				if(set_time.year >= 2100)
				{
					set_time.year = 2018;	
				}
				break;
			case 1:  //‘¬
				set_time.month++;
				if(set_time.month >= 13)
				{
					set_time.month = 1;	
				}
				break;
			case 2:  //»’
				set_time.day++;
	            if (set_time.month == 2 && !__is_leap_year(set_time.year)) 
				{
					 if(set_time.day > 28)	
		             	set_time.day = 1;
	        	}
				else 
				{
		            if (set_time.day > leap_month_table[set_time.month - 1]) 
					{
		                set_time.day = 1;
		            }
		        }
				break;
			case 3:  // ±
				set_time.hour++;
				if(set_time.hour >= 24)
				{
					set_time.hour = 0;	
				}
				break;
			case 4:  //∑÷
				set_time.min++;
				if(set_time.min >= 60)
				{
					set_time.min = 0;	
				}
				break;
			case 5:  //√Î
				set_time.sec++;
				if(set_time.sec >= 60)
				{
					set_time.sec = 0;	
				}
				break;
		}
		ui_time_update_by_id(time_id[curr_time], &set_time);
		break;
	case KEY_DOWN:
		switch(curr_time)
		{
			case 0:  //ƒÍ
				set_time.year--;
				if(set_time.year <= 2017)
				{
					set_time.year = 2099;	
				}
				break;
			case 1:  //‘¬
				set_time.month--;
				if(set_time.month <= 0)
				{
					set_time.month = 12;	
				}
				break;
			case 2:  //»’
				set_time.day--;
	            if (set_time.month == 2 && !__is_leap_year(set_time.year)) 
				{
					 if(set_time.day == 0)	
		             	set_time.day = 28;
	        	}
				else 
				{
		            if (set_time.day == 0) 
					{
		                set_time.day = leap_month_table[set_time.month - 1];
		            }
		        }
				break;
			case 3:  // ±
				set_time.hour--;
				if(set_time.hour == 0xff)
				{
					set_time.hour = 23;	
				}
				break;
			case 4:  //∑÷
				set_time.min--;
				if(set_time.min == 0xff)
				{
					set_time.min = 59;	
				}
				break;
			case 5:  //√Î
				set_time.sec--;
				if(set_time.sec >= 0xff)
				{
					set_time.sec = 59;	
				}
				break;
		}
		ui_time_update_by_id(time_id[curr_time], &set_time);
		break;
    default:
        return false;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_SET_SYS_TINE)
	.onkey = menu_sys_set_time_onkey,
	.onchange = menu_sys_set_time_layout_onchange,
 	.ontouch = NULL,
};


/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/

/***********************************************************************************/
/*							 ∏Ò ΩªØ										   */
/***********************************************************************************/

static void format_callback(void *priv, int err)
{
    __sys_msg_hide();
    if (err) {
        /*
         * Ê†ºÂºèÂåñÂá∫Èîô
         */
        printf("format_callback err!!\n");
        __sys_set_msg_show(TEXT_SYS_FORMAT_ERR, 2000);
    }
}

static int menu_sys_set_fmt_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;

	struct ui_grid *grid = (struct ui_grid *)_grid;
	struct intent it;
	int err;
    switch (e->value) {
    case KEY_OK:
		sel_item = ui_grid_cur_item(grid);
		if(sel_item)
		{
		
			init_intent(&it);
	        it.name	= "sys_set";
	        it.action = ACTION_SYS_SET_SYS_CONFIG;
	        it.data = "frm";
	        start_app_async(&it, format_callback, NULL);
		
			//ui_show(LAYER_SET_SYS_MSG);
			//ui_show(TEXT_SYS_SET_MSG_T);
			//ui_text_show_index_by_id(TEXT_2, 3);
	        __sys_set_msg_show(TEXT_SYS_FMT_SUC, 1000);
	        //ui_hide(PIC_XXXX);
		}

		ui_hide(LAYOUT_SET_SYS_FMT);
        break;
    case KEY_MODE:

        break;
    default:
        return false;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_14)
	.onkey = menu_sys_set_fmt_onkey,
 	.ontouch = NULL,
};


/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/

/***********************************************************************************/
/*							 ƒ¨»œ≥ˆ≥ß									   */
/***********************************************************************************/

static int menu_sys_set_def_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;

	struct ui_grid *grid = (struct ui_grid *)_grid;
	struct intent it;
	int err;
    switch (e->value) {
    case KEY_OK:
		sel_item = ui_grid_cur_item(grid);
		if(sel_item)
		{
			init_intent(&it);
	        it.name	= "sys_set";
	        it.action = ACTION_SYS_SET_SYS_CONFIG;
	        it.data = "def";
	        start_app_async(&it, NULL, NULL);
			//ui_show(LAYOUT_SET_SYS_MSG);
			__sys_set_msg_show(TEXT_SYS_DEFAULT_SET, 1000);
		}
		printf("menu_sys_set_def_onkey\n");
		ui_hide(LAYOUT_SET_SYS_DEF);
        break;
    case KEY_MODE:

        break;
    default:
        return false;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_15)
	.onkey = menu_sys_set_def_onkey,
 	.ontouch = NULL,
};


/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/

/***********************************************************************************/
/*							 ∞Ê±æ∫≈									   */
/***********************************************************************************/

static int menu_sys_set_ves_layout_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct layout *layout = (struct layout *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        puts("time_layout_onchange onfocus.\n");
        layout_on_focus(layout);
        break;
    case ON_CHANGE_RELEASE:
        puts("time_layout_onchange losefocus.\n");
        layout_lose_focus(layout);
        break;
    default:
        break;
    }
    return FALSE;
}
static int sys_version_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        text->str = "CD10-20180821";
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TEXT_1)
.onchange = sys_version_onchange,
 .onkey = NULL,
  .ontouch = NULL,
};

#if 1
extern char *get_wifi_ssid(void);
extern char *get_wifi_pwd(void);

static int wifi_ssid_onchange(void *ctr, enum element_change_event e, void *arg)
{
	static char buf[64];
	
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%s%s", "SSID: ",get_wifi_ssid());
        text->str = buf;
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TEXT_RES_SSID)
	.onchange = wifi_ssid_onchange,
};
#endif
static int wifi_pwd_onchange(void *ctr, enum element_change_event e, void *arg)
{
	static char buf[64];
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        memset(buf, 0, sizeof(buf));
		sprintf(buf, "%s%s", "PWD: ",get_wifi_pwd());
        text->str = buf;
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TEXT_RES_PWD)
	.onchange = wifi_pwd_onchange,
};

static int wifi_tip_onchange(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_text *text = (struct ui_text *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        text->str = "Press UP to return";
        break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(TEXT_WIFI_TRUEN)
	.onchange = wifi_tip_onchange,
};



static int menu_sys_set_ves_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;

	struct ui_grid *grid = (struct ui_grid *)_grid;
	struct intent it;
	int err;
    switch (e->value) {
    case KEY_OK:
		ui_hide(LAYOUT_SET_SYS_VES);
        break;
    case KEY_MODE:

        break;
    default:
        return false;
    }
    return true;
}
REGISTER_UI_EVENT_HANDLER(LAYOUT_SET_SYS_VES)
	.onkey = menu_sys_set_ves_onkey,
	.onchange = menu_sys_set_ves_layout_onchange,
 	.ontouch = NULL,
};


/****************************************************************************************/
/*                                end                                                   */
/****************************************************************************************/

static int menu_sys_set_kvo_onchange(void *ctr, enum element_change_event e, void *arg)
{
	struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic, db_select("kvo"));
        break;
	case ON_CHANGE_RELEASE:
		break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(PIC_SYS_SET_SYS_KVO)
	.onchange = menu_sys_set_kvo_onchange,
 	.ontouch = NULL,
};

static int menu_sys_set_rotate_onchange(void *ctr, enum element_change_event e, void *arg)
{
	struct ui_pic *pic = (struct ui_pic *)ctr;
    switch (e) {
    case ON_CHANGE_INIT:
        ui_pic_set_image_index(pic, db_select("rat"));
        break;
	case ON_CHANGE_RELEASE:
		break;
    default:
        return FALSE;
    }
    return FALSE;
}
REGISTER_UI_EVENT_HANDLER(PIC_SYS_SET_SYS_ROTATE)
	.onchange = menu_sys_set_rotate_onchange,
 	.ontouch = NULL,
};


//extern sys_set_t sys_hdl;
static int menu_exet_onkey(void *_grid, struct element_key_event *e)
{
    u8 sel_item;
	struct intent it;
	int err;
	//struct sys_event event;
	//struct key_event org = { KEY_EVENT_CLICK, KEY_OK };
    //struct key_event new = { KEY_EVENT_CLICK, KEY_MODE };
		
    switch (e->value) {
    case KEY_OK:
		//sys_key_event_disable();
		//printf("=====menu_exet_onkey====\n");
		//sys_hdl.state = 2;   //Œ™ ≤√¥‘⁄’‚¿ÔÕÀ≤ª≥ˆAPP
		
		//sys_key_event_enable();
		//sys_key_event_map(&org, &new);
	    //event.type = SYS_KEY_EVENT;
	    //event.u.key.event = KEY_EVENT_CLICK;
	    //event.u.key.value = KEY_MODE;
	    //sys_event_notify(&event);
        return true;
    case KEY_MODE:
		if(meun_id == 4)
		{
			meun_id = 1;
			ui_hide(LAY_SYS_SET_EXET);
			ui_show(LAY_SYS_SET_REC);
		}
        return true;
    default:
        return false;
        break;
    }

    return true;
}
REGISTER_UI_EVENT_HANDLER(VLIST_9)
	.onkey = menu_exet_onkey,
 	.ontouch = NULL,
};


static int sys_page_onkey(void *ctr, struct element_key_event *e)
{
    switch (e->event) {
    case KEY_EVENT_LONG:
        if (e->value == KEY_POWER) {
            //sys_exit_menu_post();
            sys_key_event_takeover(false, true);
        }
		break;
	case KEY_MODE:
		printf("=====222====\n");
        break;
			
    }
    return false;
}

static int show_rec_meun_handler(const char *type, u32 arg)
{
	meun_id = 1;
    ui_hide(LAY_SYS_SET_BG);
	ui_show(LAY_SYS_SET_REC);
    return 0;
}

static const struct uimsg_handl sys_set_msg_handler[] = 
{
    { "inrec",        show_rec_meun_handler     }, 
};

static int sys_page_onchange(void *ctr, enum element_change_event e, void *arg)
{
    switch (e) {
    case ON_CHANGE_INIT:
		//sys_key_event_takeover(true, false);
		ui_register_msg_handler(ID_WINDOW_SYS_SET, sys_set_msg_handler);
        break;
    default:
        return false;
    }
    return false;
}
REGISTER_UI_EVENT_HANDLER(ID_WINDOW_SYS_SET)
.onchange = sys_page_onchange,
 .onkey = sys_page_onkey,
  .ontouch = NULL,
};


#endif





