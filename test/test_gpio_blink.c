/*
 *  file: gpio_test.c
 *  edit by JungJaeJoon(rgbi3307@nate.com) on the www.kernel.bz
  *
 *  Copyright(C) www.kernel.bz
 *  This code is licenced under the GPL.
 */

#include <stdio.h>
#include <wiringPi.h>

#define GPIO_P1         30
#define GPIO_P2         31
#define GPIO_P3         21
#define GPIO_P4         26
#define GPIO_P5          6
#define GPIO_INTR1      22
#define GPIO_INTR2      23
#define GPIO_INTR3      24
#define GPIO_INTR4      25
#define GPIO_INTR5      27
#define GPIO_PWM1        1
#define GPIO_MD1         4
#define GPIO_MD2         5
#define GPIO_MD3        28
#define GPIO_MD4        29
#define GPIO_SPI1       12
#define GPIO_SPI2       13
#define GPIO_SPI3       14
#define GPIO_SPI4       10
#define GPIO_SPI5       11
#define GPIO_SPK         7
#define GPIO_LED         2
#define GPIO_UART_SW     0


int main (void)
{
    ///sensor board
    int pin_array1[] = {GPIO_P1, GPIO_P3, GPIO_P5, GPIO_INTR2, GPIO_INTR4, GPIO_PWM1
                , GPIO_MD2, GPIO_MD4, GPIO_SPI2, GPIO_SPI4, GPIO_LED, GPIO_UART_SW };
    int pin_array2[] = {GPIO_P2, GPIO_P4, GPIO_INTR1, GPIO_INTR3, GPIO_INTR5, GPIO_MD1
                , GPIO_MD3, GPIO_SPI1, GPIO_SPI3, GPIO_SPI5, GPIO_SPK, GPIO_UART_SW };
    ///motor board
    int pin_array3[] = {GPIO_P5, GPIO_PWM1, GPIO_INTR1, GPIO_INTR2, GPIO_INTR3, GPIO_INTR4
                , GPIO_INTR5, GPIO_SPI1, GPIO_SPI2, GPIO_SPI3, GPIO_SPI4, GPIO_SPI5 };
    int i;

    printf ("RaZig GPIO Testing...\n") ;

    wiringPiSetup () ;

    for (i=0; i < 12; i++) {
        pinMode (pin_array1[i], OUTPUT);
        pinMode (pin_array2[i], OUTPUT);
        pinMode (pin_array3[i], OUTPUT);
    }
/*
    for (i=0; i < 12; i++) {
        digitalWrite (pin_array1[i], LOW);
        digitalWrite (pin_array2[i], LOW);
        digitalWrite (pin_array3[i], LOW);
    }
*/

/*
    for (i=0; i < 12; i++) {
        digitalWrite (pin_array1[i], HIGH);
        digitalWrite (pin_array2[i], HIGH);
        digitalWrite (pin_array3[i], HIGH);
        delay (1000);   ///1s
    }

    for (i=0; i < 12; i++) {
        digitalWrite (pin_array1[i], LOW);
        digitalWrite (pin_array2[i], LOW);
        digitalWrite (pin_array3[i], LOW);
    }
*/

    while (1) {
        ///digitalWrite (GPIO_LED, LOW);       ///on
        digitalWrite (GPIO_UART_SW, HIGH);  ///TxD(console=tty1)
        digitalWrite (GPIO_LED, HIGH);      ///on
        sleep(1);
        digitalWrite (GPIO_LED, LOW);      ///off
        sleep(1);
    }

    return 0 ;
}
