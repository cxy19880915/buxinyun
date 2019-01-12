#include "system/includes.h"
#include "app_config.h"

#define FILE_SIZE  (400 * 1024 * 1024)

struct sd_test_hdl {
    int id;
    int test_time;
    u32 speed;
    OS_SEM sem;
    OS_SEM sem_end;
};

static void sd_speed_task(void *p)
{
    FILE *file;
    u8 buf[1024];
    u32 start = jiffies;
    struct vfs_partition *part;
    struct sd_test_hdl *hdl = (struct sd_test_hdl *)p;

    part = fget_partition(CONFIG_ROOT_PATH);

    if (hdl->id == 0) {
        file = fopen(CONFIG_STORAGE_PATH"/C/speed0.bin", "w+");
    } else {
        file = fopen(CONFIG_STORAGE_PATH"/C/speed1.bin", "w+");
    }
    if (!file) {
        goto __end;
    }
    int len = fseek(file, FILE_SIZE, SEEK_SET);
    if (len <= 0) {
        goto __end;
    }
    fseek(file, 0, SEEK_SET);

    hdl->speed = FILE_SIZE / 1024 / part->clust_size / 128 / 2;

    while (1) {
        int len = fwrite(file, buf, 1024);
        if (len != 1024) {
            hdl->speed = 0;
            break;
        }
        hdl->speed += 1;
        if (jiffies_to_msecs(jiffies - start) >= hdl->test_time) {
            break;
        }
    }
    fclose(file);
    if (hdl->id == 0) {
        fdelete_by_name(CONFIG_STORAGE_PATH"/C/speed0.bin");
    } else {
        fdelete_by_name(CONFIG_STORAGE_PATH"/C/speed1.bin");
    }

__end:
    os_sem_post(&hdl->sem);
    os_sem_pend(&hdl->sem_end, 0);

}



int sd_speed_test(int file_num, int test_sec)
{
    int i;
    struct sd_test_hdl hdl[2];

    if (file_num == 0 || file_num > 2) {
        return -EINVAL;
    }

    if (test_sec < 0 || test_sec > 30) {
        test_sec = 15;
    }

    for (i = 0; i < 2; i++) {
        hdl[i].id = i;
        hdl[i].speed = 0;
        hdl[i].test_time = test_sec * 1000;
        os_sem_create(&hdl[i].sem, 0);
        os_sem_create(&hdl[i].sem_end, 0);
    }

    if (file_num >= 1) {
        os_task_create(sd_speed_task, (void *)&hdl[0], 10, 1024, 0, "sd_speed_0");
    }
    if (file_num >= 2) {
        os_task_create(sd_speed_task, (void *)&hdl[1], 10, 1024, 0, "sd_speed_1");
    }

    for (i = 0; i < file_num; i++) {
        os_sem_pend(&hdl[i].sem, 0);
    }
    if (file_num >= 1) {
        task_kill("sd_speed_0");
    }
    if (file_num >= 2) {
        task_kill("sd_speed_1");
    }

    int s = (hdl[0].speed + hdl[1].speed) / test_sec;

    printf("speed: %dKB/s\n", s);

    return s;
}


