/*
 * pwm.c:
 *	This tests the hardware PWM channel.
 *
 * Copyright (c) 2012-2013 Gordon Henderson. <projects@drogon.net>
 ***********************************************************************
 * This file is part of wiringPi:
 *	https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringPi.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../lib/wiringPi.h"
#include "../lib/softPwm.h"

#define	LED	         2
#define	SPEAKER      7
#define	PWM1        30
#define	PWM2        31
#define	PWM3        21
#define	PWM4        26
#define	MD1          4
#define	MD2          5
#define	MD3         28
#define	MD4         29

#define LED_ON       0
#define LED_OFF      1

int main (void)
{
  int speed, i, step;

  printf ("Raspberry Pi wiringPi PWM test program\n") ;

  if (wiringPiSetup () == -1) {
    exit (1) ;
  }

  pinMode (LED, OUTPUT);
  pinMode (SPEAKER, OUTPUT);
  digitalWrite (LED, LED_ON);	// On
  digitalWrite (SPEAKER, HIGH);	// On

  pinMode (MD1, OUTPUT);
  pinMode (MD2, OUTPUT);
  pinMode (MD3, OUTPUT);
  pinMode (MD4, OUTPUT);
  digitalWrite(MD1, 0);
  digitalWrite(MD2, 0);
  digitalWrite(MD3, 1);
  digitalWrite(MD4, 1);

  //pinMode (1, PWM_OUTPUT);
  //pinMode (6, PWM_OUTPUT);
  pinMode (PWM1, PWM_OUTPUT);
  pinMode (PWM2, PWM_OUTPUT);
  pinMode (PWM3, PWM_OUTPUT);
  pinMode (PWM4, PWM_OUTPUT);

  ///OFF
  softPwmCreate (PWM1, 0, 100, MODE_DC_MOTOR); //pin, duty, interval(Hz=1/f)
  softPwmCreate (PWM2, 0, 100, MODE_DC_MOTOR); //pin, duty, interval(Hz=1/f)
  softPwmCreate (PWM3, 0, 100, MODE_DC_MOTOR); //pin, duty, interval(Hz=1/f)
  softPwmCreate (PWM4, 0, 100, MODE_DC_MOTOR); //pin, duty, interval(Hz=1/f)

#if 1
  //Soft PWM test
  //-------------------------------------------------
  // 3:  6: 2092Hz
  // 6: 12: 1065Hz
  //12: 24:  560Hz
  //25: 50:  240Hz
  //50:100   120Hz

  speed = 5;
  step = 5;
  softPwmCreate (PWM1, speed, 100, MODE_DC_MOTOR); //pin, duty, interval(Hz=1/f)
  ///softPwmCreate (PWM2, speed, 100, MODE_DC_MOTOR); //pin, duty, interval(Hz=1/f)
  ///softPwmCreate (PWM3, speed, 100, MODE_DC_MOTOR); //pin, duty, interval(Hz=1/f)
  ///softPwmCreate (PWM4, speed, 100, MODE_DC_MOTOR); //pin, duty, interval(Hz=1/f)

  while (1)
  ///for (i=0; i < 4; i++)
  {

      digitalWrite(MD1, 0);
      digitalWrite(MD2, 0);
      digitalWrite(MD3, 1);
      digitalWrite(MD4, 1);

      digitalWrite (LED, LED_ON);	// On

      delay(2000);
      digitalWrite(MD1, 1);
      digitalWrite(MD2, 1);
      digitalWrite(MD3, 0);
      digitalWrite(MD4, 0);
      digitalWrite (LED, LED_OFF);	// On

      delay(2000);

        speed += step;
        if (speed > 100) {
            speed = 100;
            step = -5;
        } else if (speed < 5) {
            speed = 5;
            step = 5;
        }

        softPwmWrite(PWM1, speed);
        softPwmWrite(PWM2, speed);
        softPwmWrite(PWM3, speed);
        softPwmWrite(PWM4, speed);

    }

    softPwmStop(PWM1);
    softPwmStop(PWM2);
    softPwmStop(PWM3);
    softPwmStop(PWM4);

#endif


  return 0 ;
}
