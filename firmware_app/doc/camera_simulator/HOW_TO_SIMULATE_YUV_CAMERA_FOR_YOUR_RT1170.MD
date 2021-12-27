
## How to use YUV camera simulator

1. ### load frames bin file to flash ( make sure the address is valid on your platform)

Connect the J-Link and execute the below commands:

```shell
connect           // initiate connection to device
MIMXRT1776XXXA_M7 // set device to "MIMXRT1176XXXA_M7"
s                 // Specify the "SWD" interface
<Enter>           // use default interface speed
loadbin /PATH/TO/FRAMES/BIN/YUV1P444_5frames_0x30800000.bin 0x30800000 // flash fw into address 0x30800000
```

2. ### Aligh flash address for loaded frames in project

The following Macros should be align with frames bin file in Hal_camera_rgb_sim.c
eg: 
    define SIMULATOR_FRAMES_FLASH_ADDRESS 0x30800000
    define SIM_FRAME_COUNT 5 

3. ### register camera simulator in project

Add the following line in main.cpp

SLN_CameraDevRGBSimRegister();



