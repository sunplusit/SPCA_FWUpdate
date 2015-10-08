/*
 * libspca.h -- Serial Flash Write of Sunplus IT camera.
 *
 * Copyright (c) 2014-2015 Frank Chang, Sunplus IT
 * 
 */

#ifndef LIBSPCA_H
#define LIBSPCA_H

#include <stdio.h> // FILE
#include <stdlib.h>
#include "libusb.h"

#define PACKET_SIZE	64
#define XFER_TIMEOUT	5000
#define MAX_CODE_SIZE	65536

#define Mem2Word(input) (*(input)+(*((input)+1)<<8))
#define Mem2Word_Hi_Low(input) (((*(input))<<8) +*((input)+1))

#define MAX_RETRY	50

//#define ENABLE_DBG 1
// 0 = none, 1 = error, 2 = debug, 3 = info

//#if ENABLE_DBG

#define logi(level, ...) {\
    char str[100];\
    sprintf(str, __VA_ARGS__);\
    if(level > 2)\
    std::cout << "INFO  [" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << str << std::endl;\
    }
    
#define logd(level, ...) {\
    char str[100];\
    sprintf(str, __VA_ARGS__);\
    if(level > 1)\
    std::cout << "DEBUG [" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << str << std::endl;\
    }
    
#define loge(level, ...) {\
    char str[100];\
    sprintf(str, __VA_ARGS__);\
    if(level > 0)\
    std::cerr << "ERROR [" << __FILE__ << "][" << __FUNCTION__ << "][Line " << __LINE__ << "] " << str << std::endl;\
    }
    
//#else
//#define logi(level, ...)
//#define logd(level, ...)
//#define loge(level, ...)
//#endif

enum{
    DEVICE_UNKNOWN,
    
    SPCA_2080 = 10,
    SPCA_2088,
    SPCA_2082,
    SPCA_2085,

    SF_UNDEFINED = 120,
    SF_MX25L512,
    SF_SST25VF512,
    SF_PM25LD512,
    SF_PM25LD256,
    SF_PM25LV512,
    SF_AT25F512B,
    SF_EN25F05,
    SF_AMICA25LS512,
    SF_MX25L5121E,
    SF_GD25Q512,

    SF_FM25F005,
    SF_MD25D512,
    SF_MD25D10,
    SF_MX25L1006E,
    SF_GT25F512,
    SF_EN25F40,
    SF_PM25LQ512,
    SF_W25X05CL,

};

struct SF_DOWNDATA {
	unsigned char	Addr[3];
	unsigned char	Data[32];
	unsigned short 	Count;
} ;

using namespace std;

class SunplusCamera {
    
public:
    SunplusCamera();
    ~SunplusCamera();

    int InitialCamera(libusb_device_handle* dev, int logLvl = 0);
    int Get_ICType(libusb_device_handle* dev);
    unsigned char Get_FlashType(libusb_device_handle* dev);
    void softReset(libusb_device_handle* dev);
    int buffer_download(libusb_device_handle *dev, unsigned char *buf, int buf_size);
};



#endif