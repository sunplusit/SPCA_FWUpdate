/*
 * Sunplus IT Camera Firmware Update tool.
 *
 * Copyright (c) 2014-2015 Frank Chang, Sunplus IT
 * 
 */

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <iostream>
#include <fstream>

#include "include/libspca.h"
#include "include/libusb.h"

using namespace std;

int logLevel = 0;

unsigned long long ahextoui64(const char* String)
{
    unsigned long length=(unsigned long)strlen(String);
    unsigned long long ret=0;
    if (length>8)
    {	
	return 0x10000;
    }
    else
    {
	unsigned long i=0;
	for (i=0;i<length;i++)
	{
	    switch (String[i])
	    {
		case '0':
		    ret+=((0)<<((length-i-1)*4));
		    break;
		case '1':
		    ret+=((1)<<((length-i-1)*4));
		    break;
		case '2':
		    ret+=((2)<<((length-i-1)*4));
		    break;
		case '3':
		    ret+=((3)<<((length-i-1)*4));
		    break;
		case '4':
		    ret+=((4)<<((length-i-1)*4));
		    break;
		case '5':
		    ret+=((5)<<((length-i-1)*4));
		    break;
		case '6':
		    ret+=((6)<<((length-i-1)*4));
		    break;
		case '7':
		    ret+=((7)<<((length-i-1)*4));
		    break;
		case '8':
		    ret+=((8)<<((length-i-1)*4));
		    break;
		case '9':
		    ret+=((9)<<((length-i-1)*4));
		    break;
		case 'a':
		    ret+=((10)<<((length-i-1)*4));
		    break;
		case 'b':
		    ret+=((11)<<((length-i-1)*4));
		    break;
		case 'c':
		    ret+=((12)<<((length-i-1)*4));
		    break;
		case 'd':
		    ret+=((13)<<((length-i-1)*4));
		    break;
		case 'e':
		    ret+=((14)<<((length-i-1)*4));
		    break;
		case 'f':
		    ret+=((15)<<((length-i-1)*4));
		    break;
		case 'A':
		    ret+=((10)<<((length-i-1)*4));
		    break;
		case 'B':
		    ret+=((11)<<((length-i-1)*4));
		    break;
		case 'C':
		    ret+=((12)<<((length-i-1)*4));
		    break;
		case 'D':
		    ret+=((13)<<((length-i-1)*4));
		    break;
		case 'E':
		    ret+=((14)<<((length-i-1)*4));
		    break;
		case 'F':
		    ret+=((15)<<((length-i-1)*4));
		    break;
		case ' ':
		    break;
		case 0:
		    break;
		default:
		    return 0x10001;
	    }
	}
    }
    return ret;
}

libusb_device *libusb_find_device(libusb_context *ctx, const int vid,
		const int pid, uint16_t* bcdD, int fd) {

    libusb_device **devs;
    
    // get list of devices
    int cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) 
    {
	loge(logLevel, "failed to get device list");
	return NULL ;
    }

    int r, i;
    libusb_device *device = NULL;
    struct libusb_device_descriptor desc;
    for (i = 0; i < cnt; i++) 
    {
	r = libusb_get_device_descriptor(devs[i], &desc);
	if (r < 0) 
	{
	    loge(logLevel, "failed to get device descriptor");
	    continue;
	}
	//loge(logLevel, "find device!! %d,  %.4X, %.4X",i, desc.idVendor, desc.idProduct);
	if ((!vid || (desc.idVendor == vid)) && (!pid || (desc.idProduct == pid))) 
	{
	    // found
	    device = devs[i];
	    libusb_ref_device(device);
	    //memcpy(bcdD, &desc.bcdDevice, 4);
	    break;
	}
	else
	{
	    logd(logLevel, "find USB device(%d), VID_0x%.4X PID_0x%.4X not match", i, desc.idVendor, desc.idProduct);
	}
    }

    libusb_free_device_list(devs, 1);
    return device;
}

int ReleaseInterface(libusb_device_handle *devh, int idx)
{
    int ret;
    
    ret = libusb_release_interface(devh, idx);
    //logi(logLevel, "ret = %d", ret);

    //ret = libusb_attach_kernel_driver(devh, idx);
    //logi(logLevel, "ret = %d", ret);
    
    return ret;
}


void usage(const char *argv0)
{
    cout << "usage : "
	<< endl
	<< "\t sudo " << argv0 
	<< " -v <vid> -p <pid> -f <file>"
	<< endl 
	<< "example : " 
	<< endl
	<< "\t sudo " << argv0
	<< " -v 04f2 -p b39a -f ./sf.bin" 
	<< endl;
}

int main(int argc, char *argv[])
{
    SunplusCamera *pCamera = new SunplusCamera();
    
    libusb_device_handle *device_handle; //handle to USB device
    libusb_device *usb_dev;
    libusb_context *ctx = NULL; //a libusb context for library intialization

    
    int fd=0;
    const char* szPid=NULL;
    const char* szVid=NULL;
    //const char* szBcdD=NULL;
    const char* szFile=NULL;
    
    ifstream ifile;
    unsigned char* buffer = NULL; // firmware buffer
    int pf_length = 0;		// firmware length
    
    unsigned long long pid;
    unsigned long long vid;
    uint16_t	bcdD = 0;
    
    int retCode;
    int opt;
    int helpflag = 0;
    
    char device_ready = 0;  // device ready to download flag
    char device_open = 0;   // device is open flag
    
    //int downloadSuccess = 0;
    
    uint8_t firstInterfaceNumber = 0;
   

    struct option longopts[] = 
    {
	//const char *name, int has_arg, *flag, int val
	{"vid",		 1, 0, 'v'},
	{"pid",		 1, 0, 'p'},
	{"fw",		 1, 0, 'f'},
	{"log",		 1, 0, 'l'},
	{"help", 	 0, &helpflag, 1},
	{0, 0, 0, 0}
    };
    
    if (argc < 4) 
    {
	usage(argv[0]);
	retCode = 1;
	goto bye;
    }
    
    // parse arguments
    while ((opt = getopt_long(argc, argv, "v:p:f:l:", longopts, NULL)) != EOF)
    {
	switch(opt)
	{
	    case 'v': // vid
		szVid = optarg;
		break;
	    case 'p': // pid
		szPid = optarg;
		break;
	    case 'f': // fw file
		szFile = optarg;
		break;
	    case 'l': // log level
		logLevel = atoi(optarg);
		break;
	    default:
		break;
	}
    }
    
    // check arugments
    pid = ahextoui64(szPid);
    if(pid > 0xFFFF)
    {
	loge(logLevel, "Incorrect Pid, its character can only be 0~9, a~f(A~F), its length can only <= 4");
	retCode = 1;
	goto bye;
    }
    
    vid = ahextoui64(szVid);
    if(pid > 0xFFFF)
    {
	loge(logLevel, "Incorrect Vid, its character can only be 0~9, a~f(A~F), its length can only <= 4");
	retCode = 1;
	goto bye;
    }
    
    // read file
    ifile.open(szFile);
    if(!ifile)
    {
	loge(logLevel, "Cannot open file: %s", szFile);
	retCode = 1;
	goto bye;
    }
    
    ifile.seekg(0, ifile.end);
    pf_length = ifile.tellg();
    ifile.seekg(0, ifile.beg);
    
    logd(logLevel, "buffer length is %d", pf_length);
    
    buffer = new unsigned char[MAX_CODE_SIZE];
    memset(buffer, 0, MAX_CODE_SIZE);
    ifile.read((char*)buffer, pf_length);
    ifile.close();
    
    ///////////////////////////////////
    
    // initial libusb
    retCode = libusb_init(&ctx);
    if(retCode<0)
    {
	loge(logLevel, "libusb init fail.");
	goto unInit;
    }
    //logd("libusb init ok");
    
    
    // find target device
    usb_dev = libusb_find_device(ctx, vid, pid, &bcdD, fd);
    
    if(usb_dev)
    {
	// open device
	retCode = libusb_open(usb_dev, &device_handle);
	if(retCode < 0)
	{
	    loge(logLevel, "opening USB device VID_0x%.4X/PID_0x%.4X fail, please try run as \"sudo\"", (int)vid, (int)pid);
	    device_open = 0;
	}
	else
	{
	    device_open = 1;
	    logi(logLevel, "device Opened!");
	    
	    const struct libusb_interface_descriptor *if_desc;
	    libusb_config_descriptor *device_config;
	    
	    // get config descriptor
	    retCode = libusb_get_config_descriptor(usb_dev, 0, 
						    &device_config);
	    if (retCode) 
	    {
		loge(logLevel, "get config fail");
	    }
	    else
	    {
		// get config ok
	    }
	    
	    int interface_idx = 0;
	    if(device_config)
	    {
		for(interface_idx = 0; interface_idx < device_config->bNumInterfaces; ++interface_idx)
		{
		    if_desc = &device_config->interface[interface_idx].altsetting[0];
		    // select first found video control
		    if( if_desc->bInterfaceClass == LIBUSB_CLASS_VIDEO/*14*/ && 
			if_desc->bInterfaceSubClass==1) // Video, Control
			break;
		    if_desc = NULL;
		}
	    }
	    firstInterfaceNumber = interface_idx;
	    
	    //libusb_set_auto_detach_kernel_driver(device_handle, 1);
	    //logi(logLevel, "active = %d", libusb_kernel_driver_active(device_handle, firstInterfaceNumber));
	    // detach kernel driver
	    retCode = libusb_detach_kernel_driver(device_handle, firstInterfaceNumber);
	    //logd(logLevel, "detach ret %x", retCode);
	    //logi(logLevel, "active = %d", libusb_kernel_driver_active(device_handle, firstInterfaceNumber));
	    
	    if(!retCode ||
		retCode == LIBUSB_ERROR_NOT_FOUND ||
		retCode == LIBUSB_ERROR_NOT_SUPPORTED)
	    {
		retCode = libusb_claim_interface(device_handle, firstInterfaceNumber);
		if(retCode)
		{
		    loge(logLevel, "claim interface fail (%x), you must run as \"sudo\"", retCode);
		    goto unInit;
		}
	    }
	    else
	    {
		loge(logLevel, "detach kernel driver (%x), you must run as \"sudo\"", retCode);
		goto unInit;
	    }
	    logd(logLevel, "ready");
	    device_ready = 1;
	    
	} 
    }
    else
    {
	//cout << "device not found" << endl;
	loge(logLevel, "device not found");
    }
    
    // device ready?
    if(device_ready && device_handle)
    {
	
	// check Camera
	if(pCamera->InitialCamera(device_handle, logLevel))
	{
	    loge(logLevel, "SunplutIT Camera not found.");
	    goto unInit;
	}
	
	/*
	switch(pCamera->Get_ICType(device_handle))
	{
	    case SPCA_2080 : loge(logLevel, "SPCA_2080"); break;
	    case SPCA_2088 : loge(logLevel, "SPCA_2088"); break;
	    case SPCA_2082 : loge(logLevel, "SPCA_2082"); break;
	    case SPCA_2085 : loge(logLevel, "SPCA_2085"); break;
	    default: loge(logLevel, "DEVICE_UNKNOWN"); break;
	}
	
	switch(pCamera->Get_FlashType(device_handle))
	{
	    case SF_MX25L512 : loge(logLevel, "SF_MX25L512"); break;
	    case SF_SST25VF512 : loge(logLevel, "SF_SST25VF512"); break;
	    case SF_PM25LD512 : loge(logLevel, "SF_PM25LD512"); break;
	    case SF_PM25LD256 : loge(logLevel, "SF_PM25LD256"); break;
	    case SF_PM25LV512 : loge(logLevel, "SF_PM25LV512"); break;
	    case SF_AT25F512B : loge(logLevel, "SF_AT25F512B"); break;
	    case SF_EN25F05 : loge(logLevel, "SF_EN25F05"); break;
	    case SF_AMICA25LS512 : loge(logLevel, "SF_AMICA25LS512"); break;
	    case SF_MX25L5121E : loge(logLevel, "SF_MX25L5121E"); break;
	    case SF_GD25Q512 : loge(logLevel, "SF_GD25Q512"); break;
	    case SF_FM25F005 : loge(logLevel, "SF_FM25F005"); break;
	    case SF_MD25D512 : loge(logLevel, "SF_MD25D512"); break;
	    case SF_MD25D10 : loge(logLevel, "SF_MD25D10"); break;
	    case SF_MX25L1006E : loge(logLevel, "SF_MX25L1006E"); break;
	    case SF_GT25F512 : loge(logLevel, "SF_GT25F512"); break;
	    case SF_EN25F40 : loge(logLevel, "SF_EN25F40"); break;
	    case SF_PM25LQ512 : loge(logLevel, "SF_PM25LQ512"); break;
	    case SF_W25X05CL : loge(logLevel, "SF_W25X05CL"); break;
	    default: loge(logLevel, "SF_UNDEFINED"); break;
	}
	
	*/
	
	// start to download firmware
	logi(logLevel, "Start Download Firmware");
	if(pCamera->buffer_download(device_handle, buffer, pf_length))
	{
	    loge(logLevel, "Download fail!");
	}
	else
	{
	    //ok
	}
	
	//downloadSuccess = 1;
	// reset camera device
	pCamera->softReset(device_handle);
    }
    
unInit:

    if(device_open==1)
    {
	// release and attach kernel driver
	ReleaseInterface(device_handle, firstInterfaceNumber);
	
	// close
	libusb_close(device_handle);
	device_open = 0;
    }
    
bye:
    if(buffer)
    {
	//logd(logLevel, "delete buffer");
	delete [] buffer;
	buffer = NULL;
    }
    if(pCamera)
    {
	//logd(logLevel, "delete camera");
	delete pCamera;
	pCamera = NULL;
    }
    
    //logd(logLevel, "bye");
    return retCode;
}

