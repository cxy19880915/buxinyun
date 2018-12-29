#ifndef BOARD_H
#define BOARD_H


/*-------------avin det and avin parking det----- */
extern void AV_PCTL();
extern void av_parking_det_init();
extern unsigned char av_parking_det_status();
extern unsigned char PWR_CTL(unsigned char on_off);
extern unsigned char read_power_key();
extern unsigned char usb_is_charging();
extern unsigned int get_usb_wkup_gpio();
extern void key_voice_enable(int enable);




extern void s_led_ctrl(unsigned char on_off);
extern void wifi_led_ctrl(unsigned char on_off);

unsigned short DUMP_PORT();
unsigned short FORWARD_PORT();
unsigned short BEHIND_PORT();
const char *get_rec_path_1();
const char *get_rec_path_2();
const char *get_root_path();






#endif
