/**
 *  file name:  motor_dc.c
 *  author:     JungJaeJoon (rgbi3307@nate.com) on the www.kernel.bz
 *  comments:   DC Motor Control
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

//Motor Speed GPIO
#define MOTOR_P1    30
#define MOTOR_P2    31
#define MOTOR_P3    21
#define MOTOR_P4    26

//Motor Direction GPIO
#define MOTOR_D1     4
#define MOTOR_D2     5
#define MOTOR_D3    28
#define MOTOR_D4    29

#define MOTOR_D1_GO     0
#define MOTOR_D2_GO     0
#define MOTOR_D3_GO     1
#define MOTOR_D4_GO     1

#define MOTOR_D1_BACK   1
#define MOTOR_D2_BACK   1
#define MOTOR_D3_BACK   0
#define MOTOR_D4_BACK   0

#define  BUFF_SIZE   160

#define SPEED_FIRST     20
#define SPEED_GOOD      30
#define SPEED_MAX       90
#define SPEED_MAX2      80
#define SPEED_MIN       15
#define SPEED_MIN2       5
#define SPEED_STEP       5
#define SPEED_RANGE    100


int MotorRunning = 0;
unsigned int MotorSpeed = SPEED_FIRST;   //fast=4 ~ slow=24

static void motor_set_dir(int d1, int d2, int d3, int d4)
{
    digitalWrite(MOTOR_D1, d1);     ///Right Front Wheel
    digitalWrite(MOTOR_D2, d2);     ///Left Front Wheel
    digitalWrite(MOTOR_D3, d3);     ///Left Rear Wheel
    digitalWrite(MOTOR_D4, d4);     ///Right Rear Wheel
}

static void motor_set_speed(int s1, int s2, int s3, int s4)
{
    softPwmWrite(MOTOR_P1, s1);     ///Right Front Wheel
    softPwmWrite(MOTOR_P2, s2);     ///Left Front Wheel
    softPwmWrite(MOTOR_P3, s3);     ///Left Rear Wheel
    softPwmWrite(MOTOR_P4, s4);     ///Right Rear Wheel
}

static void motor_set_dec(void)
{
    while (MotorSpeed > SPEED_MIN2) {
        MotorSpeed -= SPEED_STEP;
        motor_set_speed(MotorSpeed, MotorSpeed, MotorSpeed, MotorSpeed);
        usleep(200);
    }
}

static void motor_set_inc(void)
{
    while (MotorSpeed < SPEED_GOOD) {
        MotorSpeed += SPEED_STEP;
        motor_set_speed(MotorSpeed, MotorSpeed, MotorSpeed, MotorSpeed);
        usleep(200);
    }
}

/**
//speed
//Soft PWM test(pin, duty, interval(Hz=1/f))
//-------------------------------------------------
// 3:  6: 2092Hz
// 6: 12: 1065Hz
//12: 24:  560Hz
//25: 50:  240Hz
//50:100   120Hz
*/
static int motor_init(void)
{
    if (wiringPiSetup () == -1) return -1;

    pinMode(MOTOR_D1, OUTPUT);
    pinMode(MOTOR_D2, OUTPUT);
    pinMode(MOTOR_D3, OUTPUT);
    pinMode(MOTOR_D4, OUTPUT);

    motor_set_dir(MOTOR_D1_GO, MOTOR_D2_GO, MOTOR_D3_GO, MOTOR_D4_GO);

    pinMode(MOTOR_P1, OUTPUT);
    pinMode(MOTOR_P2, OUTPUT);
    pinMode(MOTOR_P3, OUTPUT);
    pinMode(MOTOR_P4, OUTPUT);

    digitalWrite(MOTOR_P1, 0);
    digitalWrite(MOTOR_P2, 0);
    digitalWrite(MOTOR_P3, 0);
    digitalWrite(MOTOR_P4, 0);

    pinMode(MOTOR_P1, PWM_OUTPUT);
    pinMode(MOTOR_P2, PWM_OUTPUT);
    pinMode(MOTOR_P3, PWM_OUTPUT);
    pinMode(MOTOR_P4, PWM_OUTPUT);

    return 0;
}

static void motor_setup(void)
{
    MotorSpeed = SPEED_FIRST;

    ///thread create
    softPwmCreate (MOTOR_P1, MotorSpeed, SPEED_RANGE, MODE_DC_MOTOR);
    softPwmCreate (MOTOR_P2, MotorSpeed, SPEED_RANGE, MODE_DC_MOTOR);
    softPwmCreate (MOTOR_P3, MotorSpeed, SPEED_RANGE, MODE_DC_MOTOR);
    softPwmCreate (MOTOR_P4, MotorSpeed, SPEED_RANGE, MODE_DC_MOTOR);

    MotorRunning = 1;
}

static void motor_close(void)
{
    ///thread cancel
    softPwmStop(MOTOR_P1);
    softPwmStop(MOTOR_P2);
    softPwmStop(MOTOR_P3);
    softPwmStop(MOTOR_P4);

    MotorRunning = 0;
}

static void motor_stop(void)
{
    motor_set_dec();

    if (MotorRunning) motor_close();
}

static void motor_go(void)
{
    if (!MotorRunning) motor_setup();

    motor_set_dec();

    motor_set_dir(MOTOR_D1_GO, MOTOR_D2_GO, MOTOR_D3_GO, MOTOR_D4_GO);

    ///speed up to SPEED_GOOD
    while (MotorSpeed < SPEED_GOOD) {
        MotorSpeed += SPEED_STEP;
        motor_set_speed(MotorSpeed, MotorSpeed, MotorSpeed, MotorSpeed);
        usleep(400);
    }
}

static void motor_back(void)
{
    if (!MotorRunning) motor_setup();

    motor_set_dec();

    motor_set_dir(MOTOR_D1_BACK, MOTOR_D2_BACK, MOTOR_D3_BACK, MOTOR_D4_BACK);

    ///speed up to SPEED_GOOD
    while (MotorSpeed < SPEED_GOOD) {
        MotorSpeed += SPEED_STEP;
        motor_set_speed(MotorSpeed, MotorSpeed, MotorSpeed, MotorSpeed);
        usleep(400);
    }
}

static void motor_left(void)
{
    if (!MotorRunning) motor_setup();

    motor_set_dec();
    motor_set_dir(MOTOR_D1_GO, MOTOR_D2_BACK, MOTOR_D3_BACK, MOTOR_D4_GO);

    ///speed up to SPEED_MAX2
    while (MotorSpeed < SPEED_MAX2) {
        MotorSpeed += SPEED_STEP;
        motor_set_speed(MotorSpeed, MotorSpeed, MotorSpeed, MotorSpeed);
        usleep(200);
    }
}

static void motor_right(void)
{
    if (!MotorRunning) motor_setup();

    motor_set_dec();
    motor_set_dir(MOTOR_D1_BACK, MOTOR_D2_GO, MOTOR_D3_GO, MOTOR_D4_BACK);

    ///speed up to SPEED_MAX2
    while (MotorSpeed < SPEED_MAX2) {
        MotorSpeed += SPEED_STEP;
        motor_set_speed(MotorSpeed, MotorSpeed, MotorSpeed, MotorSpeed);
        usleep(200);
    }
}

static void motor_left_go(void)
{
    if (!MotorRunning) motor_setup();

    motor_set_dec();
    motor_set_dir(MOTOR_D1_GO, MOTOR_D2_GO, MOTOR_D3_GO, MOTOR_D4_GO);
    motor_set_inc();

    MotorSpeed = SPEED_FIRST;
    motor_set_speed(SPEED_MAX2, MotorSpeed, MotorSpeed, SPEED_MAX2);
}

static void motor_right_go(void)
{
    if (!MotorRunning) motor_setup();

    motor_set_dec();
    motor_set_dir(MOTOR_D1_GO, MOTOR_D2_GO, MOTOR_D3_GO, MOTOR_D4_GO);
    motor_set_inc();

    MotorSpeed = SPEED_FIRST;
    motor_set_speed(MotorSpeed, SPEED_MAX2, SPEED_MAX2, MotorSpeed);
}

static void motor_left_back(void)
{
    if (!MotorRunning) motor_setup();

    motor_set_dec();
    motor_set_dir(MOTOR_D1_BACK, MOTOR_D2_BACK, MOTOR_D3_BACK, MOTOR_D4_BACK);
    motor_set_inc();

    MotorSpeed = SPEED_FIRST;
    motor_set_speed(SPEED_MAX2, MotorSpeed, MotorSpeed, SPEED_MAX2);
}

static void motor_right_back(void)
{
    if (!MotorRunning) motor_setup();

    motor_set_dec();
    motor_set_dir(MOTOR_D1_BACK, MOTOR_D2_BACK, MOTOR_D3_BACK, MOTOR_D4_BACK);
    motor_set_inc();

    MotorSpeed = SPEED_FIRST;
    motor_set_speed(MotorSpeed, SPEED_MAX2, SPEED_MAX2, MotorSpeed);
}

static void motor_faster(void)
{
    ///speed up to SPEED_MAX
    ///while (MotorSpeed < SPEED_MAX) {
        if (MotorSpeed >= SPEED_MAX) MotorSpeed = SPEED_MAX2;
        else MotorSpeed += SPEED_STEP;
        motor_set_speed(MotorSpeed, MotorSpeed, MotorSpeed, MotorSpeed);
        usleep(400);
    ///}
}

static void motor_slower(void)
{
    ///speed down to SPEED_MIN
    ///while (MotorSpeed > SPEED_MIN) {
        if (MotorSpeed <= SPEED_MIN) MotorSpeed = SPEED_MIN;
        else MotorSpeed -= SPEED_STEP;
        motor_set_speed(MotorSpeed, MotorSpeed, MotorSpeed, MotorSpeed);
        usleep(400);
    ///}
}


static void motor_action(char *mode)
{
    int i;
    char *action[] = { "stop", "go", "back", "left", "right"
                    , "left_go", "left_back", "right_go", "right_back"
                    , "faster", "slower" };
    void (*fn_action[])(void) = {
              motor_stop, motor_go, motor_back, motor_left, motor_right
            , motor_left_go, motor_left_back, motor_right_go, motor_right_back
            , motor_faster, motor_slower
            , motor_stop };

    for (i=0; i<11; i++) {
        if (!strcmp(mode, action[i])) break;
    }

    fn_action[i]();
}

static int motor_file_read(const char *fname)
{
	FILE *fp;
	char buf[BUFF_SIZE] = {0,};
	char *pb;
	int len;

	fp = fopen(fname, "r");
	if (!fp) {
		printf("Can't open the file(%s) for read.\n", fname);
		return -1;
	}

	///do
    ///{
        pb = fgets(buf, BUFF_SIZE, fp);
		if (!pb) goto _end;

		len = strlen(buf);
		if (len < 2) goto _end;
		buf[len-1] = '\0';
        printf( "read: %s\n", buf);

		motor_action(buf);
		///memset(buf, 0, sizeof(buf));

    ///} while (!feof(fp) );

_end:
	fclose(fp);

	if (remove(fname)) {
		printf("Can't remove the file(%s).\n", fname);
		return -2;
	} else {
		///printf("Removed the file(%s).\n", fname);
	}

	return 0;
}

int main(void)
{
	struct dirent *entry;
	DIR *dir;
	char *path = "../../data/motor/";
	char fname[32] = {0};

	if (motor_init() < 0) {
        printf("motor_init() error!\r\n");
        return -1;
	}

	while (1)
	{
        dir = opendir (path);
		while ((entry = readdir (dir)) != NULL) {
			if (strlen(entry->d_name) > 5) {
				strcat(fname, path);
				strcat(fname, entry->d_name);

				//printf ("fname = %s\n", fname);
				if (strstr(fname, "motor_")) motor_file_read(fname);

				memset(fname, 0, sizeof(fname));
			}
		} //while
		closedir(dir);
		usleep(300);
	}

    if (MotorRunning) motor_close();
	return 0;
}

