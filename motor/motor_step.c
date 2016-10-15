/**
 *	file name:	motor_step.c
 *	author:		JungJaeJoon (rgbi3307@nate.com) on the www.kernel.bz
 *	comments:   Stepping Motor Control
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
#define MOTOR_P1    21  //Left Wheel
#define MOTOR_P2    26  //Right Wheel
#define MOTOR_P3    30
#define MOTOR_P4    31

//Motor Direction GPIO
#define MOTOR_D1     4  //Left Direction
#define MOTOR_D2     5  //Right Direction
#define MOTOR_D3    28
#define MOTOR_D4    29

#define  BUFF_SIZE   160

#define SPEED_FIRST     12
#define SPEED_GOOD       8
#define SPEED_MAX        4
#define SPEED_MIN       24
#define SPEED_STEP       2
#define SPEED_RANGE     60

int MotorRunning = 0;
unsigned int MotorSpeed = SPEED_FIRST;   //fast=4 ~ slow=24


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

    digitalWrite (MOTOR_D1, LOW);     //Left Wheel Go
    digitalWrite (MOTOR_D2, HIGH);    //Right Wheel Go

    return 0;
}

static void motor_setup(void)
{
    MotorSpeed = SPEED_FIRST;

    //thread create
    softPwmCreate (MOTOR_P1, MotorSpeed, SPEED_RANGE, MODE_STEP_MOTOR); //Left Wheel
    softPwmCreate (MOTOR_P2, MotorSpeed, SPEED_RANGE, MODE_STEP_MOTOR); //Right Wheel

    MotorRunning = 1;
}

static void motor_close(void)
{
    //thread cancel
    softPwmStop(MOTOR_P1);
    softPwmStop(MOTOR_P2);

    MotorRunning = 0;
}

static void motor_stop(void)
{
    softPwmWrite(MOTOR_P1, MotorSpeed+SPEED_STEP);
    softPwmWrite(MOTOR_P2, MotorSpeed+SPEED_STEP);
    usleep(400);
    softPwmWrite(MOTOR_P1, MotorSpeed+SPEED_STEP*2);
    softPwmWrite(MOTOR_P2, MotorSpeed+SPEED_STEP*2);
    usleep(400);

    if (MotorRunning) motor_close();
}

static void motor_go(void)
{
    int speed;

    if (!MotorRunning) motor_setup();

    digitalWrite(MOTOR_D1, LOW);    //Left Wheel Go
    digitalWrite(MOTOR_D2, HIGH);   //Right Wheel Go

    //speed up
    MotorSpeed -= SPEED_STEP;
    if (MotorSpeed < SPEED_MAX) {
        MotorSpeed = SPEED_MAX;
    } else {
        speed = MotorSpeed;
        while (speed > SPEED_GOOD) {
            speed -= SPEED_STEP;
            softPwmWrite(MOTOR_P1, speed);
            softPwmWrite(MOTOR_P2, speed);
            usleep(800);
        }
    }
}

static void motor_back(void)
{
    int speed;

    if (!MotorRunning) motor_setup();

    digitalWrite(MOTOR_D1, HIGH);   //Left Wheel Back
    digitalWrite(MOTOR_D2, LOW);    //Right Wheel Back

    //speed down
    MotorSpeed += SPEED_STEP;
    if (MotorSpeed > SPEED_MIN) {
        MotorSpeed = SPEED_MIN;
    }

    //speed up going
    speed = MotorSpeed;
    while (speed > SPEED_GOOD) {
        speed -= SPEED_STEP;
        softPwmWrite(MOTOR_P1, speed);
        softPwmWrite(MOTOR_P2, speed);
        usleep(800);
    }
}

static void motor_left(void)
{
    if (!MotorRunning) motor_setup();

    softPwmWrite(MOTOR_P1, MotorSpeed);
    softPwmWrite(MOTOR_P2, MotorSpeed);
    digitalWrite(MOTOR_D1, HIGH);   //Left Wheel back
    digitalWrite(MOTOR_D2, HIGH);   //Right Wheel Go
}

static void motor_right(void)
{
    if (!MotorRunning) motor_setup();

    softPwmWrite(MOTOR_P1, MotorSpeed);
    softPwmWrite(MOTOR_P2, MotorSpeed);
    digitalWrite(MOTOR_D1, LOW);   //Left Wheel go
    digitalWrite(MOTOR_D2, LOW);   //Right Wheel back
}

static void motor_left_go(void)
{
    if (!MotorRunning) motor_setup();

    MotorSpeed = SPEED_FIRST;
    softPwmWrite(MOTOR_P1, MotorSpeed+6);
    softPwmWrite(MOTOR_P2, MotorSpeed);
    digitalWrite(MOTOR_D1, LOW);    //Left Wheel Go(slow)
    digitalWrite(MOTOR_D2, HIGH);   //Right Wheel Go
}

static void motor_right_go(void)
{
    if (!MotorRunning) motor_setup();

    MotorSpeed = SPEED_FIRST;
    softPwmWrite(MOTOR_P1, MotorSpeed);
    softPwmWrite(MOTOR_P2, MotorSpeed+6);
    digitalWrite(MOTOR_D1, LOW);    //Left Wheel Go
    digitalWrite(MOTOR_D2, HIGH);   //Right Wheel Go(slow)
}

static void motor_left_back(void)
{
    if (!MotorRunning) motor_setup();

    MotorSpeed = SPEED_FIRST;
    softPwmWrite(MOTOR_P1, MotorSpeed+6);
    softPwmWrite(MOTOR_P2, MotorSpeed);
    digitalWrite(MOTOR_D1, HIGH);   //Left Wheel Back(slow)
    digitalWrite(MOTOR_D2, LOW);    //Right Wheel Back
}

static void motor_right_back(void)
{
    if (!MotorRunning) motor_setup();

    MotorSpeed = SPEED_FIRST;
    softPwmWrite(MOTOR_P1, MotorSpeed);
    softPwmWrite(MOTOR_P2, MotorSpeed+6);
    digitalWrite(MOTOR_D1, HIGH);   //Left Wheel Back
    digitalWrite(MOTOR_D2, LOW);    //Right Wheel Back(slow)
}


static void motor_faster(void)
{
    //speed up
    while (MotorSpeed > SPEED_MAX) {
        MotorSpeed -= SPEED_STEP;
        softPwmWrite(MOTOR_P1, MotorSpeed);
        softPwmWrite(MOTOR_P2, MotorSpeed);
        usleep(800);
    }
}

static void motor_slower(void)
{
    //speed down
    while (MotorSpeed < SPEED_MIN) {
        MotorSpeed += SPEED_STEP;
        softPwmWrite(MOTOR_P1, MotorSpeed);
        softPwmWrite(MOTOR_P2, MotorSpeed);
        usleep(800);
    }
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

	do
    {
        pb = fgets(buf, BUFF_SIZE, fp);
		if (!pb) break;

		len = strlen(buf);
		buf[len-1] = '\0';
        printf( "read: %s\n", buf);

		motor_action(buf);
		memset(buf, 0, sizeof(buf));

    } while (!feof(fp) );


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

		usleep(200);
	}

    if (MotorRunning) motor_close();
	return 0;
}

