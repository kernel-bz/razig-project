/**----------------------------------------------------------------------------
 * Name:    video_play.c
 * Purpose: video play module
 * Author:	JungJaeJoon on the www.kernel.bz
 *-----------------------------------------------------------------------------
 * Notes:
 *-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>

#include "../common/usr_types.h"
#include "../lib/user_mysql.h"

#define STOP    0
#define END     1
#define REPEAT  2

int PStatus;
int RunningPid;
char Path[80];

int process_run(char *arg)
{
    int pid;

    pid = fork();
    if (pid < 0) {
        printf("fork() error in music_play.\r\n");
        return -1;
    } else if (pid == 0) {
        ///RunningPid = getpid();
        if (execl("/usr/bin/mplayer", "/usr/bin/mplayer"
                    , "-noconsolecontrols", "-quiet", arg, NULL) == -1) {
            printf("execl() error in music_play.\r\n");
            return -2;
        }
        close(0);
        close(1);
        close(2);
        chdir("/");
        setsid();

        ///RunningPid = getpid();
        ///printf("RunningPid = %d\r\n", RunningPid);
        exit(0);
    } else {
        int status;
        RunningPid = pid;   ///child id
        //printf("RunningPid = %d\r\n", RunningPid);
        waitpid(RunningPid, &status, WUNTRACED);
    }
    return 0;
}

int dir_check(char *dir1, char *dir2, char *dir3, char *path)
{
    char buf[80]={0};
    DIR *dir;

    dir = opendir(dir1);
    if (!dir) {
        printf("Do not find media directory(%s)\r\n", dir1);
        return -1;
    }

    strcpy(buf, dir1);
    strcat(buf, dir2);
    dir = opendir(buf);
    if (!dir) {
        if (mkdir(buf, 0755) < 0) {
            printf("Do not create directory(%s)\r\n", buf);
            return -2;
        }
        //chmod(buf, 0755);
    }

    strcat(buf, dir3);
    dir = opendir(buf);
    if (!dir) {
        if (mkdir(buf, 0755) < 0) {
            printf("Do not create directory(%s)\r\n", buf);
            return -3;
        }
        //chmod(buf, 0755);
    }

    strcpy(path, buf);
    return 0;
}

int dir_files_play(char *path)
{
	struct dirent *entry;
	DIR *dir;
	char fname[100] = {0};
	int cnt;

	while (1)
	{
        cnt = 0;
		dir = opendir (path);
		while ((entry = readdir (dir)) != NULL) {
			if (strlen(entry->d_name) > 5) {
				strcat(fname, path);
				strcat(fname, entry->d_name);

				//printf ("fname = %s\n", fname);
				process_run(fname);
				cnt++;

				///sleep(1);
				if (PStatus == STOP || PStatus == END) break;

				memset(fname, 0, sizeof(fname));
			} //if
		} //while
		closedir(dir);

		///sleep(1);
		if (cnt <= 0 || PStatus == STOP || PStatus == END) break;
	}

	return 0;
}

void process_stop(int signo)
{
    char buf[20] = {0};
    ///sprintf(buf, "kill -9 %d", RunningPid);
    sprintf(buf, "kill 15 %d", RunningPid); ///SIGTERM(15) Termination
    system(buf);

    sleep(1);
    RunningPid = 0;

    PStatus = STOP;
}

void process_end(int signo)
{
    PStatus = END;  //one time
    dir_files_play(Path);
}

void process_repeat(int signo)
{
    PStatus = REPEAT;   //loop
    dir_files_play(Path);
}


int main()
{
    char buf[2][60] = {{0,0}};
    const char *query = "SELECT media_dir, base_dir FROM tbl_config WHERE pno=2";

    user_mysql_init();
    user_mysql_connect("localhost", "root", "kernel.bz");
    user_mysql_select_db("db_razig");

    if (user_mysql_query_select(query, buf) < 0) {
        strcpy(buf[0], "/media/pi/WORKS/");
        strcpy(buf[1], "razig");
    }

    //printf("buf=%s, %s\r\n", buf[0], buf[1]);
    dir_check(buf[0], buf[1], "/video/", Path);

    //printf("path=%s\r\n", path);
    if (signal(SIGUSR1, (void *)process_stop) == SIG_ERR) {
        printf("signal(SIGUSR1) error in music_play.\r\n");
    }

    if (signal(SIGUSR2, (void *)process_end) == SIG_ERR) {
        printf("signal(SIGUSR2) error in music_play.\r\n");
    }

    if (signal(SIGCONT, (void *)process_repeat) == SIG_ERR) {
        printf("signal(SIGCONT) error in music_play.\r\n");
    }

    user_mysql_close();

    while(1) pause();

    return 0;
}
