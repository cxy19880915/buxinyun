#include "system/includes.h"

#include "res.h"
#include "app_database.h"


#define CN_PA   ((0xA9BE << 16) | ('A' << 8)  | ('B' << 0))
#define CN_PB   (('C'    << 24) | ('D' << 16) | ('E' << 8) | ('F' << 0))
#define UI_PARA_REFRESH   0x1a

/*
 * app配置项表
 * 参数1: 配置项名字
 * 参数2: 配置项需要多少个bit存储
 * 参数3: 配置项的默认值
 */


static const struct db_table app_config_table[] = {
    /*
     *  录像模式配置项
     */
    {"mic",     1,      1},                         // 录音开关
    {"mot",     1,      0},                         // 移动侦测开关
    {"par",     1,      0},                         // 停车守卫开关
    {"wdr",     1,      1},                         // 夜视增强开关
    {"num",     1,      0},                         // 车牌开关
    {"dat",     1,      1},                         // 时间标签开关
    {"two",     1,      1},                         // 双路开关
    {"gra",     2,      GRA_SEN_OFF},                // 重力感应灵敏度
    {"res",     3,      VIDEO_RES_1080P},           // 录像分辨率
    {"bu",     2,      guan},           // 补光灯
    {"cyc",     4,      3},                         // 循环录像时间，单位分钟
    {"exp",     8,      0},                         // 曝光, 范围-3到+3
    {"gap",     16,     0},                         // 间隔录影设置
    {"cna",     32,     CN_PA},                     // 车牌号码前3个字符
    {"cnb",     32,     CN_PB},                     // 车牌号码后4个字符
   
    /*
     *  系统模式配置项
     */
    {"kvo",     1,      1},                         // 按键音开关
    {"lag",     5,      Chinese_Simplified},        // 语言设置
    {"fre",     8,      50},                        // 灯光频率
    {"aff",     8,      0},                         // 自动关机时间, 单位分钟
    {"pro",     8,      0},                         // 屏幕保护时间, 单位秒
    {"tvm",     8,      TVM_PAL},                   // 电视制式
    {"lan",     32,     0},                         // 轨道偏移
    {"hlw",     1,      0},                         // 前照灯提醒开关
    {"paraflag",     8,     UI_PARA_REFRESH},
    /*
     *  拍照模式配置项
     */
    {"sok",     1,      0},                         // 防手抖开关
    {"pdat",    1,      1},                         // 图片日期标签开关
    {"cyt",     1,      0},                         // 连拍开关
    {"qua",     3,      PHOTO_QUA_HI},              // 图片质量
    {"acu",     3,      PHOTO_ACU_LO},              // 图片锐度
    {"phm",     4,      0},                         // 延时拍照， 单位秒
    {"pres",    4,      PHOTO_RES_1D3M},            // 图片分辨率
    {"wbl",     4,      PHOTO_WBL_AUTO},            // 白平衡
    {"col",     4,      PHOTO_COLOR_NORMAL},        // 颜色模式
    {"sca",     4,      0},                         // 快速预览时间，单位s
    {"pexp",    8,      0},                         // 曝光设置，范围-3到+3
    {"iso",     16,     0},                         // iso


    {"dac",     16,     0x55aa},                         // dac_trim
};



int app_set_config(struct intent *it, const struct app_cfg *cfg, int size)
{
    int i;

    printf("app_set_config: %s, %d\n", it->data, it->exdata);

    for (i = 0; i < size; i++) {
        if (!strcmp(it->data, cfg[i].table)) {
            if (cfg[i].set && cfg[i].set(it->exdata)) {
                return -EFAULT;
            }
            return db_update(cfg[i].table, it->exdata);
        }
    }

    return -EINVAL;
}





static int app_config_init()
{
    int err;
    int para;
	
    err = db_create("rtc");
    if (err) {
        return err;
    }
      err = db_create_table(app_config_table, ARRAY_SIZE(app_config_table));
			    para = db_select("paraflag");
			    if(para != UI_PARA_REFRESH)
			    {
			        db_reset();
			    }
			    return err;
  //  return db_create_table(app_config_table, ARRAY_SIZE(app_config_table));
}
__initcall(app_config_init);


