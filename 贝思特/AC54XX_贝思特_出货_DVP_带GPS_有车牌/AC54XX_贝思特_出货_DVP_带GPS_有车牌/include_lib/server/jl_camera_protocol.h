/*****************************************************************
>file name : jl_camera_protocol.h
>author : lichao
>create time : Mon 31 Jul 2017 03:10:46 PM HKT
*****************************************************************/
#include "typedef.h"

#if 0
struct jlc_stream_packet {
    unsigned int offset : 32;
    unsigned int len : 24;
    unsigned int payload_info : 8;// 
    u8 data[2048];   
};


struct jlc_packet_header {
    unsigned int id : 8;
    unsigned int attr : 8;
    unsigned int dir : 1;
    unsigned int reserved : 15;
    int len : 24;
};

#define JLC_PID     0x4A
//#define PID_ACK         0x4B
//#define PID_STREAM      0x4C
//#define PID_OTHER       0x4D


enum jlc_packet_attribute {
    JLC_STREAM = 0x4B,
    JLC_CTL,
    JLC_REQUEST,
    JLC_RESPONSE,
    JLC_ACK,
};
#endif

#define JLC_CTL_PID             0x82
struct jlc_control_packet {
    u8 pid;
    u8 cmd; 
    u16 len; 
    u8 data[0];
};

#define     FRAME_START_PID     0x80
#define     FRAME_SUB_PID       0x81
struct v_frame_header {
    u8 pid;  
    u8 fmt;
    u16 pkt_len;
    int len;
    u32 frame_id;
    u32 offset;
};

#if 1
enum jl_camera_rec_state {
    JLC_REC_START = 0x20,
    JLC_REC_STOP, 
    JLC_SAVE_FILE,
    JLC_SET_REC_INFO,
    JLC_SET_INFO,
    JLC_GET_INFO,
    JLC_SET_OSD,
    JLC_SET_MUTE,
};
#endif
