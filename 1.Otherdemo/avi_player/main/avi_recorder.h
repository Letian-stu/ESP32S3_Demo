#ifndef _AVI_PRECESS_H_
#define _AVI_PRECESS_H_

#include "esp_log.h"
#include "esp_camera.h"

#ifdef __cplusplus 
extern "C" {
#endif

void avi_recorder_start(const char *fname, framesize_t rec_size, uint32_t rec_time);

void avi_recorder_stop(void);


#ifdef __cplusplus 
}
#endif

#endif