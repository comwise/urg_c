/*!
  \example sync_time_stamp.c センサと PC のタイムスタンプを同期する

  \author Satofumi KAMIMURA

  $Id$
*/

#include "urg_sensor.h"
#include "urg_utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>


static int pc_msec_time(void)
{
    static bool is_initialized = false;
    static struct timespec first_timespec;
    struct timespec timespec;
    long msec_time;

    if (!is_initialized) {
        clock_gettime(CLOCK_REALTIME, &first_timespec);
        is_initialized = true;
    }

    clock_gettime(CLOCK_REALTIME, &timespec);

    msec_time =
        1000 * (timespec.tv_sec - first_timespec.tv_sec) +
        ((timespec.tv_nsec - first_timespec.tv_nsec) / 1000000);

    return msec_time;
}


// PC のタイムスタンプに補正するための値を返す
static long print_time_stamp(urg_t *urg, long time_stamp_offset)
{
    long sensor_time_stamp;
    long pc_time_stamp;
    long before_pc_time_stamp;
    long after_pc_time_stamp;
    long delay;

    urg_start_time_stamp_mode(urg);

    before_pc_time_stamp = pc_msec_time();
    sensor_time_stamp = urg_time_stamp(urg);
    after_pc_time_stamp = pc_msec_time();
    delay = (after_pc_time_stamp - before_pc_time_stamp) / 2;

    if (sensor_time_stamp < 0) {
        printf("urg_time_stamp: %s\n", urg_error(urg));
        return -1;
    }
    sensor_time_stamp -= time_stamp_offset;

    pc_time_stamp = pc_msec_time();
    urg_stop_time_stamp_mode(urg);

    printf("%ld,\t%ld\n", pc_time_stamp, sensor_time_stamp);

    return sensor_time_stamp - (pc_time_stamp - delay);
}


int main(int argc, char *argv[])
{
    enum {
        TIME_STAMP_PRINT_TIMES = 5,
    };

    urg_t urg;
    connection_type_t connection_type = URG_SERIAL;
    long time_stamp_offset;
    int i;

#if defined(URG_WINDOWS_OS)
    const char device[] = "COM3";
#elif defined(URG_LINUX_OS)
    const char device[] = "/dev/ttyACM0";
#else
#endif

    // 接続タイプの切替え
    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-e")) {
            connection_type = URG_ETHERNET;
        }
    }

    // 接続
    if (urg_open(&urg, connection_type, device, 115200) < 0) {
        printf("urg_open: %s\n", urg_error(&urg));
        return 1;
    }

    printf("# pc,\tsensor\n");

    // URG のタイムスタンプと PC のタイムスタンプを表示
    time_stamp_offset = print_time_stamp(&urg, 0);

    // URG の補正後のタイムスタンプと PC タイムスタンプを表示
    for (i = 0; i < TIME_STAMP_PRINT_TIMES; ++i) {
        print_time_stamp(&urg, time_stamp_offset);
    }

    urg_close(&urg);

#if defined(URG_MSC)
    getchar();
#endif
    return 0;
}