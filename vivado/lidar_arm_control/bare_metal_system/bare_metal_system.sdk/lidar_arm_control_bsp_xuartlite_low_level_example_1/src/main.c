/******************************************************************************
*
* Copyright (C) 2002 - 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xuartlite_low_level_example.c
*
* This file contains a design example using the low-level driver functions
* and macros of the UartLite driver (XUartLite).
*
* @note
*
* The user must provide a physical loopback such that data which is
* transmitted will be received.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who  Date	 Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00b rpm  04/25/02 First release
* 1.00b sv   06/13/05 Minor changes to comply to Doxygen and coding guidelines
* 2.00a ktn  10/20/09 Updated to use HAL processor APIs and minor changes
*		      for coding guidelines.
* 3.2   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xuartlite_l.h"
#include "xil_printf.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

/************************** Constant Definitions *****************************/


/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define UARTLITE_BASEADDR	   XPAR_UARTLITE_0_BASEADDR

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the UartLite, this constant must be 16 bytes or less so the
 * entire buffer will fit into the transmit and receive FIFOs of the UartLite.
 */
#define TEST_BUFFER_SIZE 16

#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0])) //macro to get array length
#endif

// Commands that can be sent to RPLIDARs.
// class Command:
#define Stop 0x25
#define Reset 0x40
#define Scan 0x20
#define ExpressScan 0x82
#define ForceScan 0x21
#define GetInfo 0x50
#define GetHealth 0x52
#define GetSampleRate 0x59

// data types returned in data response descriptors
// class ResponseType:
#define ScanData 0x81
#define ExpressScanData 0x82
#define DeviceInfo 0x04
#define HealthInfo 0x06
#define SamplingRate 0x15

// two-bit send modes returned in data response descriptors
// SendMode:
#define SingleResponse 0x00
#define MultipleResponse 0x01

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int UartLiteLowLevelExample(u32 UartliteBaseAddress);
void get_device_info(u32 UartliteBaseAddress, int* devinfo);
void get_health_info(u32 UartliteBaseAddress, int* healthinfo);
void print_response_data(int* data, int len);
void read_response_descriptor(u32 UartliteBaseAddress);
void start_scan(u32 UartliteBaseAddress);
void stop_scan(u32 UartliteBaseAddress);
void poll_scan_samples(u32 UartliteBaseAddress, int* packet, int len);

void print_response_data(int* data, int len){
    for(int i=0; i<len; i++){
        printf("%i ", data[i]);
    }
}

void get_response_data(u32 UartliteBaseAddress, int* data, int len){
    for(int i=0; i<len; i++){
        data[i] = XUartLite_RecvByte(UartliteBaseAddress);
    }
}

/************************** Variable Definitions *****************************/

/*
 * The following buffers are used in this example to send and receive data
 * with the UartLite.
 */
u8 SendBuffer[TEST_BUFFER_SIZE]; /* Buffer for Transmitting Data */
u8 RecvBuffer[TEST_BUFFER_SIZE]; /* Buffer for Receiving Data */


/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
******************************************************************************/
int main(void)
{

    int devinfo[4];
    get_device_info(UARTLITE_BASEADDR, devinfo);
    int model       = devinfo[0];
    int fw          = devinfo[1];
    int hw          = devinfo[2];
    int serial_no   = devinfo[3];

    int healthinfo[2];
    get_health_info(UARTLITE_BASEADDR, healthinfo);
    int health_status   = healthinfo[0];
    int err_code        = healthinfo[1];

    printf("\n\n===\nOpened LIDAR on UARTLITE_BASEADDR: 0x%x\nModel ID: %i\nFirmware: %i\nHardware: %i\nSerial Number: %i etc...\nDevice Health Status: %i (Error Code: 0x%x)\n===\n", UARTLITE_BASEADDR, model, fw, hw, serial_no, health_status, err_code);

    start_scan(UARTLITE_BASEADDR);

    int packet[5];
    int packet_size = 5;

//    while(1){
//        poll_scan_samples(UARTLITE_BASEADDR, packet, packet_size);
//
//        int scan_start      = packet[0] & 1;
////        int inv_scan_start  = packet[0] & 2;
////        int check_bit       = packet[1] & 1;
//
//        float angle_q6 = (((packet[1] >> 1) & 0x7F) | (packet[2] << 7)) / 64.0;
//        float distance_q2 = (packet[3] | (packet[4] << 8)) / 4;
//
//        // printf("0x%x 0x%x 0x%x 0x%x 0x%x\n", packet[0], packet[1], packet[2], packet[3], packet[4]);
//        printf("ScanStart: %i - Angle: %f - Distance: %f\n", scan_start, angle_q6, distance_q2);
//    }
    stop_scan(UARTLITE_BASEADDR);
}


void get_device_info(u32 UartliteBaseAddress, int* devinfo){

    // Retrieve hardware information from the RPLIDAR.
    // Returns:
    //     A tuple containing:
    //     - The RPLIDAR model ID, as an integer
    //     - The firmware version number, as a ``float``
    //     - The hardware version number, as an integer
    //     - The 128-bit serial number, as ``bytes``.

    printf("\nSent Device Info command...\n");
    XUartLite_SendByte(UartliteBaseAddress, 0xA5);
    XUartLite_SendByte(UartliteBaseAddress, GetInfo);

    read_response_descriptor(UartliteBaseAddress);

    int len = 20;
    int data[len];
    get_response_data(UartliteBaseAddress, data, len);

    printf("~~~ Device Info ~~~\n");
    print_response_data(data, len);

    devinfo[0] = data[0];
    devinfo[1] = (data[1] | (data[2] << 8)) / 256.0;
    devinfo[2] = data[3];
    devinfo[3] = data[4];
}

void get_health_info(u32 UartliteBaseAddress, int* healthinfo){

    // Retrieve the health status of the RPLIDAR.
    // Returns:
    //     A tuple containing:
    //     - The RPLIDAR's status
    //         - 0 indicates a healthy device.
    //         - 1 indicates a warning status: the device works, but may fail
    //           soon.
    //         - 2 indicates an error status.
    //     - An error code

    printf("\nSent Health Info command...\n");
	XUartLite_SendByte(UartliteBaseAddress, 0xA5);
	XUartLite_SendByte(UartliteBaseAddress, GetHealth);

    read_response_descriptor(UartliteBaseAddress);

    int len = 3;
    int data[len];
    get_response_data(UartliteBaseAddress, data, len);

    printf("\n~~~ Device Health Info ~~~\n");
    print_response_data(data, len);

    healthinfo[0] = data[0];
    healthinfo[1] = data[1] | (data[2] << 8);
}

void read_response_descriptor(u32 UartliteBaseAddress){
    while(1){
        int checked_byte = XUartLite_RecvByte(UartliteBaseAddress);
        if(checked_byte == 0xA5){
            break;
        }
    }

    int raw_desc[7];
    raw_desc[0] = 0xA5;
    raw_desc[1] = XUartLite_RecvByte(UartliteBaseAddress);
    raw_desc[2] = XUartLite_RecvByte(UartliteBaseAddress);
    raw_desc[3] = XUartLite_RecvByte(UartliteBaseAddress);
    raw_desc[4] = XUartLite_RecvByte(UartliteBaseAddress);
    raw_desc[5] = XUartLite_RecvByte(UartliteBaseAddress);
    raw_desc[6] = XUartLite_RecvByte(UartliteBaseAddress);

    int t = (raw_desc[2] | (raw_desc[3] << 8) | (raw_desc[4] << 16) | (raw_desc[5] << 24));

    int data_type = raw_desc[6];

    int response_length = t & ~(3 << 30);
    int send_mode = (t & (3 << 30)) >> 30;

    printf("Got response descriptor...\n");
    printf("Len: %i, Mode: %i, Data Type: %i\n", response_length, send_mode, data_type);
}

void start_scan(u32 UartliteBaseAddress){

    // Command the RPLIDAR to begin standard scanning.
    // To retrieve scan measurements, call :method:`poll_scan_samples`.
    // Note:
    //     The RPLIDAR will only begin to return samples after the sensor's
    //     motor rotation has stabilized.

	XUartLite_SendByte(UartliteBaseAddress, 0xA5);
	XUartLite_SendByte(UartliteBaseAddress, Scan);

    read_response_descriptor(UartliteBaseAddress);
}

void stop_scan(u32 UartliteBaseAddress){

    // Command the RPLIDAR to begin standard scanning.
    // To retrieve scan measurements, call :method:`poll_scan_samples`.
    // Note:
    //     The RPLIDAR will only begin to return samples after the sensor's
    //     motor rotation has stabilized.

	XUartLite_SendByte(UartliteBaseAddress, 0xA5);
	XUartLite_SendByte(UartliteBaseAddress, Stop);
}

void poll_scan_samples(u32 UartliteBaseAddress, int* packet, int len){

    get_response_data(UartliteBaseAddress, packet, len);
//    packet[0] = XUartLite_RecvByte(UartliteBaseAddress);
//    packet[1] = XUartLite_RecvByte(UartliteBaseAddress);
//    packet[2] = XUartLite_RecvByte(UartliteBaseAddress);
//    packet[3] = XUartLite_RecvByte(UartliteBaseAddress);
//    packet[4] = XUartLite_RecvByte(UartliteBaseAddress);
}
