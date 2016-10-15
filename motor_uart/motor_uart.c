/**
 *	file name:	motor_uart.c
 *	author:		JungJaeJoon (rgbi3307@nate.com) on the www.kernel.bz
 *	comments:   Uart Motor(XL320) Control
 *
 *  Copyright(C) www.kernel.bz
 *  This code is licenced under the GPL.
  *
 *  Editted:
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <dirent.h>

#include "../lib/wiringPi.h"
#include "../lib/softPwm.h"
#include "../common/gpio.h"

#include "dxl_hal.h"
#include "dynamixel.h"

static void gpio_init(void)
{
    pinMode (GPIO_LED, OUTPUT);
    pinMode (GPIO_UART_SW, OUTPUT);
    pinMode (GPIO_SPK, OUTPUT);

    digitalWrite (GPIO_LED, LOW);       ///on
    digitalWrite (GPIO_UART_SW, HIGH);  ///TxD(console=tty1)
    digitalWrite (GPIO_SPK, HIGH);      ///on
}

static void motor_uart_read_write_test(void)
{
	unsigned char packet[32] = {0};
	int size = 32;
	unsigned short crc;

	printf( "\n\nRead/Write example for Linux\n\n" );
	///////// Open USB2Dynamixel ////////////
	if( dxl_initialize(0, 1) == 0 )
	{
		printf( "Failed to open UART Dynamixel!\n" );
		printf( "Press Enter key to terminate...\n" );
		getchar();
		return 0;
	}
	else
		printf( "Succeed to open UART Dynamixel!\n" );

    while(1) {

    digitalWrite (GPIO_UART_SW, HIGH);  ///TxD
    packet[0] = 0xFF;
    packet[1] = 0xFF;
    packet[2] = 0xFD;
    packet[3] = 0x00;
    packet[4] = 0xFE;   ///ID
    packet[5] = 0x06;   ///LEN_L
    packet[6] = 0x00;   ///LEN_H

    packet[7] = 0x03;   ///Instruction
    packet[8] = 0x19;   ///P_Addr_L
    packet[9] = 0x00;   ///P_Addr_H
    packet[10]= 0x02;   ///P_Data_0

    crc = dxl_crc(0, packet, 11);

    packet[11] = crc & 0x00FF;          ///CRC_L
    packet[12] = (crc >> 8) & 0x00FF;   ///CRC_H

    size = 13;

    dxl_hal_tx(packet, size);

    //digitalWrite (GPIO_UART_SW, LOW);  ///RxD
    //memset(packet, 0, size);
    //dxl_hal_rx(packet, size);

    //digitalWrite (GPIO_UART_SW, HIGH);  ///TxD

    sleep(1);

    }

    dxl_hal_close();
}

int main(void)
{
    printf ("RaZig Motor_Uart Testing...\n") ;

    wiringPiSetup() ;

    gpio_init();

    motor_uart_read_write_test();


    return 0;
}
