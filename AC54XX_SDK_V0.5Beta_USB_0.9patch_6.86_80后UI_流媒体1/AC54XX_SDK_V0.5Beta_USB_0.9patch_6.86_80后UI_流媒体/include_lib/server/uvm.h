
#ifndef __UVM_H__
#define __UVM_H__

#include "server/server_core.h"
#include "server/video_server.h"

enum uvm_rec_status {
	UVM_REC_IDLE,
	UVM_REC_START,
	UVM_REC_STOP,
	UVM_REC_STOPING,
	UVM_REC_PAUSE,
	UVM_REC_ERR,
};

#define UVM_CMD_SET_INFO		1
#define UVM_CMD_GET_INFO		2
#define UVM_CMD_START			3
#define UVM_CMD_STOP			4
#define UVM_CMD_SAVE_FILE  		5
#define UVM_CMD_GET_DATA		6
#define UVM_CMD_SET_OSD_STR		7
#define UVM_CMD_PKG_MUTE		8


struct uvm_rec {
	void * fd;
    void *private_data;
    const char *dev_name;
    OS_SEM sem;
    OS_SEM pause;
	char task_name[16];
	void * file;
    u8 fname_buf[64];
	
	
	u8 status;


};

struct uvm_rec_data {
	u8 *buf;
	u32 len;
	u32 addr;
	u32 end_flag;
};


int uvm_rec_start(struct uvm_rec * uvm, struct vs_video_rec *rec);

int uvm_rec_save_file(struct uvm_rec * uvm, struct vs_video_rec *rec);

int uvm_rec_stop(struct uvm_rec * uvm, struct vs_video_rec *rec);

int uvm_rec_pause(struct uvm_rec * uvm, struct vs_video_rec *rec);

int uvm_rec_resume(struct uvm_rec * uvm, struct vs_video_rec *rec);

int uvm_rec_set_osd_str(struct uvm_rec * uvm, struct vs_video_rec *rec);

int uvm_rec_get_info(struct uvm_rec * uvm, struct vpkg_get_info * get_info);

int uvm_rec_pkg_mute(struct uvm_rec * uvm, struct vs_video_rec *rec);












#endif





