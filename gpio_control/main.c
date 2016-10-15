/**
 *	file name:	gpio_control.c
 *	author:     JungJaeJoon (rgbi3307@nate.com) on the www.kernel.bz
 *	comments:   GPIO Control Module
 *
 *  Copyright(C) www.kernel.bz
 *  This code is licenced under the GPL.
  *
 *  Editted:
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../lib/wiringPi.h"

#define	LED	         2
#define	SPEAKER      7

#define LED_ON       0
#define LED_OFF      1

int main (int argc, char **argv)
{
    if (wiringPiSetup () == -1) {
        return -1;
    }

    pinMode (LED, OUTPUT);
    pinMode (SPEAKER, OUTPUT);

    if (argc > 1) {
        digitalWrite (LED, LED_OFF);	// Off
        digitalWrite (SPEAKER, LOW);	// Off
    } else {
        digitalWrite (LED, LED_ON);	    // On
        digitalWrite (SPEAKER, HIGH);	// On
    }

    return 0;
}
