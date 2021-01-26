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
* @file ultrasonic_sensor_driver.c
*
* This file contains a design for an Ultrasonic Sensor (HC-SR04) driver using the AXI GPIO driver (XGpio) and
* hardware device.  It only uses channel 1 of a GPIO device and assumes that
* the bit 0 of the GPIO is connected to the sensor Trigger pin and bit 1 to the Echo pin on the HW board.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <stdio.h>
#include "xparameters.h"
#include "xgpio.h"
#include "xil_printf.h"
#include "xtime_l.h"

/************************** Constant Definitions *****************************/

#define TRIG 0x1   /* Assumes bit 0 of GPIO is connected to an TRIG pin  */
#define ECHO 0x2   /* Assumes bit 1 of GPIO is connected to an ECHO pin  */
#define BOTH 0x3   /* Mask for both TRIG and ECHO GPIO pins	*/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define GPIO_EXAMPLE_DEVICE_ID  XPAR_GPIO_0_DEVICE_ID

/*
 * The following constant is used to specify the amount of
 * distance measurements performed per output reading
 */
#define TOTAL_CALC	100

/*
 * The following constant is used to determine which channel of the GPIO is
 * used for the sensor pins if there are 2 channels supported.
 */
#define CHANNEL 1

/************************** Variable Definitions *****************************/
/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger
 */

XGpio Gpio; /* The Instance of the GPIO Driver */

/************************** Function Definitions *****************************/
float get_distance();
float mode();
void test_delay();
void _delay_();


/*****************************************************************************/
int main(void){

	int Status;

	/* Initialize the GPIO driver */
	Status = XGpio_Initialize(&Gpio, GPIO_EXAMPLE_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Gpio Initialization Failed\r\n");
		return XST_FAILURE;
	}

//	/* test for the delay function */
//	test_delay(100);

	/* Set the direction for all signals as inputs except the TRIG output */
	XGpio_SetDataDirection(&Gpio, CHANNEL, ~TRIG);

	/* Loop forever measuring distance */
	while (1) {

		float dist_list[TOTAL_CALC];
		float mode_dist;

		for(int i = 0; i < TOTAL_CALC; i++){
			/* add reading to list */
			dist_list[i] = get_distance();

			/* sensor hardware requires 2ms delay between measurements */
			_delay_(2000);
		}

		/* find the model value from all readings */
		mode_dist = mode(dist_list);

		printf("Distance: %f mm\n\n", mode_dist);
	}

	xil_printf("Successfully ran Gpio Example\r\n");
	return XST_SUCCESS;
}

float mode(float list[]) {

   int modeValue = 0, modeCount = 0, i, j;

   /* loop through list of values */
   for (i = 0; i < TOTAL_CALC; ++i) {
      int count = 0;

      /* count number of occurrences */
      for (j = 0; j < TOTAL_CALC; ++j) {
         if (list[j] == list[i])
         count++;
      }

      /* check if value is the mode */
      if (count > modeCount) {
         modeCount = count;
         modeValue = list[i];
      }
   }
   /* return the mode value */
   return modeValue;
}

float get_distance(){

	XTime tStart, tEnd;

	/* Clear the TRIG bit */
	XGpio_DiscreteClear(&Gpio, CHANNEL, BOTH);

	/* Wait 2us so TRIG is low */
	_delay_(2);

	/* Set the TRIG to High */
	XGpio_DiscreteWrite(&Gpio, CHANNEL, TRIG);

//	xil_printf("Triggering: 0x%x\n", XGpio_DiscreteRead(&Gpio, CHANNEL));

	/* 10us TRIG high pulse */
	_delay_(10);

	/* Clear the TRIG bit */
	XGpio_DiscreteClear(&Gpio, CHANNEL, TRIG);

//	xil_printf("Wait for Echo pulse: 0x%x\n", XGpio_DiscreteRead(&Gpio, CHANNEL));

	/* wait for the echo pulse to begin (rising edge) */
	while(!(XGpio_DiscreteRead(&Gpio, CHANNEL) & ECHO)){}
	/* record time at rising edge */
	XTime_GetTime(&tStart);

//	xil_printf("Measuring Echo pulse: 0x%x\n", XGpio_DiscreteRead(&Gpio, CHANNEL));

	/* wait for the echo pulse to end (falling edge) */
	while(XGpio_DiscreteRead(&Gpio, CHANNEL) & ECHO){}
	/* record time at falling edge */
	XTime_GetTime(&tEnd);

//	xil_printf("Echo pulse ended: 0x%x\n", XGpio_DiscreteRead(&Gpio, CHANNEL));

//	printf("Start %f us\n", (1.0 * tStart));
//	printf("End %f us\n", (1.0 * tEnd));

	/* calculate pulse length in micro-seconds */
	float pulse = 1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000);

	/* speed of signal is 343000 mm/sec */
	/* pulse length time to obstacle and back (x2 distance) */
	/* calculate distance in millimeters */
	float distance = (pulse/1000000)/2 * 343000;

//	printf("Pulse Length: %f us\n", delay);
//	printf("Distance: %f mm\n\n", distance);

	/* clear all pins */
	XGpio_DiscreteClear(&Gpio, CHANNEL, BOTH);

	/* return measured distance */
	return distance;
}

void test_delay(int delay_size){

	printf("Testing Delay %i us\n", delay_size);

	XTime tStart, tEnd;

	/* get time before delay */
	XTime_GetTime(&tStart);

	/* run the delay */
	_delay_(delay_size);

	/* get time after the delay */
	XTime_GetTime(&tEnd);

	printf("Start %f us\n", (1.0 * tStart));
	printf("End %f us\n", (1.0 * tEnd));

	/* calculate delay length in useconds */
	float delay = 1.0 * (tEnd - tStart) / (COUNTS_PER_SECOND/1000000);
	printf("Delay took %f us\n\n", delay);
}

void _delay_(int useconds){

	float delay;
	XTime tStart, tNext;

	/* get time at start of delay */
	XTime_GetTime(&tStart);

	while(1){
		/* check time for each loop */
		XTime_GetTime(&tNext);

		/* calculate the delay in micro-seconds */
		delay = 1.0 * (tNext - tStart) / (COUNTS_PER_SECOND/1000000);

		/* if the delay is greater than or equal to the specified time, end delay */
		if(delay >= useconds){
			return;
		}
	}
}

