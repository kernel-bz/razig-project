/*
 *  file name:  test_i2c.c
  *	comments:   i2c test program
 *  editted by: JungJaeJoon on the www.kernel.bz
  *
 *  Copyright(C) www.kernel.bz
 *  This code is licenced under the GPL.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
//#include <fcntl.h>
//#include <sys/ioctl.h>
//#include <linux/spi/spidev.h>

#include "../lib/wiringPi.h"
#include "../lib/wiringPiI2C.h"

int main (void)
{
	int i, data;

    //I2C Clock: 100KHz
	//sudo i2cdetect -y 1 (/dev/i2c-1)
	//I2C Device: 0x1e, 0x5d, 0x5f, 0x6b    //0x01 ~ 0x77

	//I2C Addr: 0x1e: 0x3C(0x3D), LIS3MDL
	//I2C Addr: 0x5d: 0xBA(0xBD), LPS25HB
	//I2C Addr: 0x5f: 0xBE(0xBC), HTS221
	//I2C Addr: 0x6b: 0xD6(0x68), LSM6DS0
	const int i2c_dev[4] = { 0x1e, 0x5d, 0x5f, 0x6b };
	int fd[4];

    for (i=0; i<4; i++) {
        fd[i] = wiringPiI2CSetup (i2c_dev[i]);
        if (fd[i] < 0) {
            printf("I2C(0x%02X) setup error!\n", i2c_dev[i]);
            exit(-1);
        } else {
            printf("I2C(0x%02X) setup succeeded.(fd=%d)\n", i2c_dev[i], fd[i]);
        }
        data = wiringPiI2CReadReg8(fd[i], 0x0F);    //Who are you?
        printf("data=%02X\n", data);    //I am ID
    }

	return 0 ;
}
