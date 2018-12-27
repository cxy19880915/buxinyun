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
extern unsigned char acc_is_charging();

#define pmsg(fmt,args...) do{printf("L%d(%s)"fmt, __LINE__,__FILE__,  ##args);}while(0)










#endif
