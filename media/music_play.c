/**----------------------------------------------------------------------------
 * Name:    music_play.c
 * Purpose: music play module
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

//#include "../common/usr_types.h"

#define STOP    0
#define END     1
#define REPEAT  2

int PStatus;
int RunningPid=0;
char MediaType[32];

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

int get_media_dir(char *arg, char *buf)
{
    FILE *fp;

    int ret;

    sprintf(buf, "df -h | awk '/dev\/%s/ {print $6}'", arg);
    fp = popen(buf, "r");

    if (fp) {
        fgets(buf, sizeof(buf), fp);
        if (strstr(buf, "/media/pi/WORKS")) ret = 1;
        else ret = 0;
    } else {
        ret = -1;
    }

	pclose(fp);

    return ret;
}

int dir_check(char *path, char *subdir)
{
    char *arg[] = { "sda1", "sda2", "sda3", "sda4"
                  , "sdb1", "sdb2", "sdb3", "sdb4"
                  , "sdc1", "sdc2", "sdc3", "sdc4"
                  , "sdd1", "sdd2", "sdd3", "sdd4" };
    DIR *dir;
    int i, found;

    for (i=0; i<16; i++) {
        found = get_media_dir(arg[i], path);
        if (found > 0) break;
    }

    if (found > 0) {
        i = strlen(path) - 1;
        path[i] = '\0';
        strcat(path, subdir);
        printf("path=%s\r\n", path);
    } else {
        return -1;
    }

    dir = opendir(path);
    if (dir) {
        closedir(dir);
        return 1;
    } else {
        printf("Do not find media directory(%s)\r\n", path);
        ///mkdir(path, 0755);
        return -2;
    }
}

int dir_files_play(char *subdir)
{
    char path[80] = {0};
	struct dirent *entry;
	DIR *dir;
	char fname[100] = {0};
	int cnt;

    if (dir_check(path, subdir) <= 0) return -1;

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

    if (RunningPid==0) goto _exit;

    ///sprintf(buf, "kill -9 %d", RunningPid);
    sprintf(buf, "kill 15 %d", RunningPid); ///SIGTERM(15) Termination
    system(buf);

    sleep(1);
    RunningPid = 0;

_exit:
    PStatus = STOP;
}

void process_end(int signo)
{
    PStatus = END;  //one time
    if (RunningPid == 0) dir_files_play(MediaType);
    RunningPid = 0;
}

void process_repeat(int signo)
{
    PStatus = REPEAT;   //loop
    if (RunningPid == 0) dir_files_play(MediaType);
    RunningPid = 0;
}


int main(int argc, char **argv)
{
    if (argc > 1) {
        strcpy(MediaType, argv[1]);
    } else {
        strcpy(MediaType, "/razig/music/");
    }

    if (signal(SIGUSR1, (void *)process_stop) == SIG_ERR) {
        printf("signal(SIGUSR1) error in music_play.\r\n");
    }

    if (signal(SIGUSR2, (void *)process_end) == SIG_ERR) {
        printf("signal(SIGUSR2) error in music_play.\r\n");
    }

    if (signal(SIGCONT, (void *)process_repeat) == SIG_ERR) {
        printf("signal(SIGCONT) error in music_play.\r\n");
    }

    while(1) pause();

    return 0;
}
