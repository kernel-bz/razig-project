/**
 * Name:    media_run.c
 * Purpose: media running module
 * Author:	JungJaeJoon on the www.kernel.bz
 *
 *  Copyright(C) www.kernel.bz
 *  This code is licenced under the GPL.
  *
 *  Editted:
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
#include <time.h>

#include "../common/usr_types.h"
#include "../common/user.h"
#include "../lib/user_mysql.h"

#define STOP        0
#define ONETIME     1
#define REPEAT      2

int PStatus;
int RunningPid=0;
char MediaType[32];
char Options[64];
char FullPath[100];
char Path[60];

int process_run(char *fname)
{
    int pid;

    pid = fork();
    if (pid < 0) {
        printf("fork() error in media_run.\r\n");
        return -1;
    } else if (pid == 0) {
        ///RunningPid = getpid();
        ///if (execl("/usr/bin/mplayer", "/usr/bin/mplayer", "-noconsolecontrols", "-quiet", fname, NULL) == -1)
        if (execl("/usr/bin/mplayer", "/usr/bin/mplayer", fname, "<", "/dev/null", "&", NULL) == -1)
        {
            printf("execl() error in media_run.\r\n");
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

void media_run(char *path, char *subdir, char *fname, char *timeout)
{
    if (strstr(subdir, "record"))
    {
        char cmd[160], rname[40];
        struct tm *tms;
        time_t t = time(NULL);
        tms = localtime(&t);

        sprintf(rname, "record_%04d%02d%02d_%02d%02d%02d_h264"
            , 2000+(tms->tm_year%100), tms->tm_mon+1, tms->tm_mday
            , tms->tm_hour, tms->tm_min, tms->tm_sec);
        strcpy(FullPath, path);
        strcat(FullPath, rname);
        sprintf(cmd, "raspivid -t %s -o - > %s %s &", timeout, FullPath, Options);
        system(cmd);
        while(1) pause();

    } else if (strstr(subdir, "motion")) {
        system("service motion restart");
        while(1) pause();

    } else if (strstr(subdir, "radio")) {
        process_run(fname); ///radio
    }
}

int dir_check(char *path)
{
    DIR *dir;
    dir = opendir(path);
    if (dir) {
        closedir(dir);
        strcpy(Path, path);
        return 1;
    } else {
        printf("Do not find media directory(%s)\r\n", path);
        strcpy(Path, "\0\0");
        return -1;
    }
}

void dir_motion_files_move(char *path_trg)
{
    struct dirent *entry;
	DIR *dir;
	char *path_src = "/home/pi/motion/";
	char fullpath[80];
	char buf[160];
	struct stat sb;

	if (strlen(path_trg) < 5) return;

    dir = opendir (path_src);
    while ((entry = readdir (dir)) != NULL) {
        if (strlen(entry->d_name) > 5) {
            ///memset(fullpath, 0, sizeof(fullpath));
            strcpy(fullpath, path_src);
            strcat(fullpath, entry->d_name);
            stat(fullpath, &sb);
            if ((int)sb.st_size > 0) {
                sprintf(buf, "mv %s %s", fullpath, path_trg);
                system(buf);
            };
        }
    } //while
    closedir(dir);
}

void media_files_play(char *subdir)
{
    uint32_t cnt=0, is_media=0;
	char path[60], fname[120], tbl[20], timeout[20];
	char qbuf[256];
	MYSQL *conn;
	const char *query = "SELECT path, fname, size FROM `%s` " \
        " WHERE `sub`='%s' and `enable` < 999999999 order by enable, utime desc";
    MYSQL_RES *res;
    MYSQL_ROW row;

_retry:
    conn = user_mysql_connect("localhost", "root", "kernel.bz");
    if (!conn) goto _exit;
    if (user_mysql_select_db(conn, "db_razig") < 0) goto _exit;

    if (strcmp(subdir, "/radio/") && strcmp(subdir, "/record/") && strcmp(subdir, "/motion/"))
    {
        strcpy(tbl, "tbl_files");
        is_media = 0;
    } else {
        strcpy(tbl, "tbl_media");
        is_media = 1;
    }
    sprintf(qbuf, query, tbl, subdir);
    res = user_mysql_query_result(conn, qbuf);
    if (!res) goto _exit;

    while ((row=mysql_fetch_row(res)) != NULL) {
        strcpy(path, row[0]);
        strcpy(fname, row[1]);
        strcpy(timeout, row[2]);
        if (cnt == 0) {
            if (dir_check(path) <= 0) goto _exit;
        }
        if (is_media) {
            media_run(path, subdir, fname, timeout);
        } else {
            user_str_char_replace(fname, '`', '\'');
            strcpy(qbuf, path);
            strcat(qbuf, fname);
            process_run(qbuf);
        }
        if (PStatus == STOP || PStatus == ONETIME) break;
        cnt++;
    }

_exit:
    user_mysql_close(conn);

    sleep(1);
    if (PStatus == REPEAT) goto _retry;
}

void process_stop(int signo)
{
    char buf[256];

    if (!strcmp(MediaType, "/record/")) {
        system("killall raspivid");
        ///sleep(1);
        sprintf(buf, "ffmpeg -r 30 -i %s -vcodec copy %s.mp4", FullPath, FullPath);
        system(buf);
        ///sleep(1);
        sprintf(buf, "rm %s", FullPath);
        system(buf);

    } else if (!strcmp(MediaType, "/motion/")) {
        system("service motion stop");
        ///sleep(1);
        dir_motion_files_move(Path);

    } else {
        if (RunningPid==0) goto _exit;

        sprintf(buf, "kill -9 %d", RunningPid);  ///SIGKILL(9)
        ///sprintf(buf, "kill 15 %d", RunningPid); ///SIGTERM(15) Termination
        system(buf);
        RunningPid = 0;
    }
    sleep(1);

_exit:
    PStatus = STOP;

    exit(0);
}

void process_onetime(int signo)
{
    PStatus = ONETIME;  //one time
    if (RunningPid == 0) media_files_play(MediaType);
    exit(0);
}

void process_repeat(int signo)
{
    PStatus = REPEAT;   //loop
    if (RunningPid == 0) media_files_play(MediaType);
}


int main(int argc, char **argv)
{
    int i;

    if (argc > 3) {
        strcpy(MediaType, argv[1]);
        strcpy(Options, argv[2]);
        for (i=3; i<argc; i++) {
            strcat(Options, " ");
            strcat(Options, argv[i]);
        }
    } else if (argc > 1) {
        strcpy(MediaType, argv[1]);
    } else {
        strcpy(MediaType, "/music/");
    }

    //media_files_play(MediaType);

    if (signal(SIGUSR1, (void *)process_stop) == SIG_ERR) {
        printf("signal(SIGUSR1) error in media_play.\r\n");
    }

    if (signal(SIGUSR2, (void *)process_onetime) == SIG_ERR) {
        printf("signal(SIGUSR2) error in media_play.\r\n");
    }

    if (signal(SIGCONT, (void *)process_repeat) == SIG_ERR) {
        printf("signal(SIGCONT) error in media_play.\r\n");
    }

    while(1) pause();

    return 0;
}
