#ifndef STORAGE_DEVICE_H
#define STORAGE_DEVICE_H




extern int storage_device_ready();
extern int mount_sd_to_fs(const char *name);
extern int upgrade_detect(const char *sdcard_name);








#endif

