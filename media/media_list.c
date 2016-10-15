/**
 * Name:    media_list.c
 * Purpose: media list collection module
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
#include <sys/inotify.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#include "../common/usr_types.h"
#include "../lib/user_mysql.h"

#define MEDIA_DIR   "/media/pi/"
#define RAZIG       "/razig"

#define CAMERA      "/camera/"
#define RECORD      "/record/"
#define ACTION      "/action/"
#define MOTION      "/motion/"
#define MOTION_ACT  "/home/pi"
#define MUSIC       "/music/"
#define VIDEO       "/video/"
#define RADIO       "/radio/"
#define MOTION_DIR  "/home/pi/motion/"

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))

typedef struct _dir_tag {
    char path[80];
    char subdir[20];
} DIR_T;


char Path[80];
uint32_t DirMediaFound=0;

void str_char_replace(char *s, char t, char r)
{
    int len = strlen(s);
    int i;

    for (i=0; i<len; i++) {
        if (s[i] == t) s[i] = r;
    }
}

int dir_get(char *arg, char *buf)
{
    FILE *fp;

    int ret;

    sprintf(buf, "df -h | awk '/dev\\/%s/ {print $6}'", arg);
    fp = popen(buf, "r");

    if (fp) {
        fgets(buf, 30, fp);
        if (strstr(buf, MEDIA_DIR)) ret = 1;
        else ret = 0;
    } else {
        ret = -1;
    }

	pclose(fp);

    return ret;
}

int dir_media_check(char *path, char *subdir)
{
    char *arg[] = { "sda1", "sda2", "sda3", "sda4"
                  , "sdb1", "sdb2", "sdb3", "sdb4"
                  , "sdc1", "sdc2", "sdc3", "sdc4"
                  , "sdd1", "sdd2", "sdd3", "sdd4" };
    DIR *dir;
    int i, found, cnt;

    cnt = ARRAY_CNT(arg);
    for (i=0; i < cnt; i++) {
        found = dir_get(arg[i], path);
        if (found > 0) break;
    }

    if (found > 0) {
        i = strlen(path) - 1;
        path[i] = '\0';
        strcat(path, subdir);
        ///printf("path=%s\r\n", path);
    } else {
        return -1;
    }

    dir = opendir(path);
    if (dir) {
        closedir(dir);
        return 1;
    } else {
        printf("Do not find media directory(%s)\r\n", path);
        mkdir(path, 0755);
        return -2;
    }
}

int dir_sub_check(char *path, char *subdir)
{
    DIR *dir;
    MYSQL *conn;

    strcat(path, subdir);
    dir = opendir(path);
    if (dir)
    {
        closedir(dir);
        if (strstr(subdir, "record") || strstr(subdir, "motion")) {
            char query[128];
            conn = user_mysql_connect("localhost", "root", "kernel.bz");
            user_mysql_select_db(conn, "db_razig");
            sprintf(query, "UPDATE `tbl_media` SET path='%s' WHERE sub='%s'", path, subdir);
            user_mysql_query(conn, query);
            user_mysql_close(conn);
        }
        return 1;
    } else {
        printf("Do not find media directory(%s)\r\n", path);
        mkdir(path, 0755);
        return -1;
    }
}


void dir_motion_files_move(char *fname)
{
    struct dirent *entry;
	DIR *dir;
	char path_trg[80];
	char fullpath[80];
	char buf[160];
	struct stat sb;
	struct timespec ts;

	strcpy(path_trg, Path);
	strcat(path_trg, MOTION);
	///if (!strcmp(path_src, path_trg)) return;

	strcpy(fullpath, MOTION_DIR);
    strcat(fullpath, fname);
	stat(fullpath, &sb);
	///ts = sb.st_atim;
	ts = sb.st_mtim;

    dir = opendir (MOTION_DIR);
    while ((entry = readdir (dir)) != NULL) {
        if (strlen(entry->d_name) > 5) {
            ///memset(fullpath, 0, sizeof(fullpath));
            strcpy(fullpath, MOTION_DIR);
            strcat(fullpath, entry->d_name);
            stat(fullpath, &sb);

            if ((int)sb.st_size > 0 && sb.st_mtim.tv_sec < ts.tv_sec-60) {
                sprintf(buf, "mv %s %s", fullpath, path_trg);
                system(buf);
            };
        }
    } //while
    closedir(dir);
}

int dir_files_insert(char *path, char *subdir, char *fname)
{
    char fullpath[160] = {0};
	char dtime[20], utime[20];
	int ret=0;
	time_t t = time(NULL);
    struct stat sb;
	struct tm *tms;
	char qbuf[256];
	MYSQL *conn;
	const char *query = "INSERT INTO `tbl_files` " \
        "(`path`, `sub`, `fname`, `dtime`, `size`, `utime` " \
        ") VALUES ( '%s', '%s', '%s', '%s', %d, '%s' );";

    if (!strcmp(path, MOTION_DIR)) goto _end;

    conn = user_mysql_connect("localhost", "root", "kernel.bz");
    if (!conn) goto _exit;
    if (user_mysql_select_db(conn, "db_razig") < 0) goto _exit;

    strcpy(fullpath, path);
    strcat(fullpath, fname);
    stat(fullpath, &sb);
    tms = localtime(&sb.st_mtime);
    sprintf(dtime, "%04d-%02d-%02d %02d:%02d:%02d"
            , 2000+(tms->tm_year%100), tms->tm_mon+1, tms->tm_mday
            , tms->tm_hour, tms->tm_min, tms->tm_sec);
    tms = localtime(&t); //current time
    sprintf(utime, "%04d-%02d-%02d %02d:%02d:%02d"
            , 2000+(tms->tm_year%100), tms->tm_mon+1, tms->tm_mday
            , tms->tm_hour, tms->tm_min, tms->tm_sec);

    printf ("INSERT: %s, %s, %s, %s, %d, %s\r\n"
            , path, subdir, fname, dtime, (int)sb.st_size, utime);

    memset(qbuf, 0, sizeof(qbuf));
    str_char_replace(fname, '\'', '`');
    sprintf(qbuf, query, path, subdir, fname, dtime, (int)sb.st_size, utime);
    ret = user_mysql_query(conn, qbuf);

_exit:
    user_mysql_close(conn);

_end:
    if (!strcmp(subdir, MOTION)) {
        dir_motion_files_move(fname);
    }

    return ret;
}

int dir_files_update(char *path, char *subdir, char *fname)
{
    char fullpath[160] = {0};
	char dtime[20], utime[20];
	int ret=0;
    struct stat sb;
    time_t t = time(NULL);
	struct tm *tms;
	char qbuf[256];
	MYSQL *conn;
	/*
	const char *query = "UPDATE `tbl_files` SET " \
        " `dtime`='%s', `size`=%d " \
        " WHERE `path`='%s' and `sub`='%s' and `fname`='%s'";
    */
    const char *query = "INSERT INTO `tbl_files` " \
        "(`path`, `sub`, `fname`, `dtime`, `size`, `utime` " \
        ") VALUES ( '%s', '%s', '%s', '%s', %d, '%s' );";

    if (!strcmp(path, MOTION_DIR)) goto _end;

    conn = user_mysql_connect("localhost", "root", "kernel.bz");
    if (!conn) goto _exit;
    if (user_mysql_select_db(conn, "db_razig") < 0) goto _exit;

    strcpy(fullpath, path);
    strcat(fullpath, fname);
    stat(fullpath, &sb);
    tms = localtime(&sb.st_mtime);
    sprintf(dtime, "%04d-%02d-%02d %02d:%02d:%02d"
            , 2000+(tms->tm_year%100), tms->tm_mon+1, tms->tm_mday
            , tms->tm_hour, tms->tm_min, tms->tm_sec
    );
    tms = localtime(&t); //current time
    sprintf(utime, "%04d-%02d-%02d %02d:%02d:%02d"
            , 2000+(tms->tm_year%100), tms->tm_mon+1, tms->tm_mday
            , tms->tm_hour, tms->tm_min, tms->tm_sec);

    printf ("INSERT: %s, %s, %s, %s, %d, %s\r\n", path, subdir, fname
            , dtime, (int)sb.st_size, utime);

    memset(qbuf, 0, sizeof(qbuf));
    str_char_replace(fname, '\'', '`');
    //sprintf(qbuf, query, dtime, fsize, path, subdir, fname);
    sprintf(qbuf, query, path, subdir, fname, dtime, (int)sb.st_size, utime);
    ret = user_mysql_query(conn, qbuf);

_exit:
    user_mysql_close(conn);

_end:
    return ret;
}

int dir_files_delete(char *path, char *subdir, char *fname)
{
	int ret=0;
	char qbuf[256];
	MYSQL *conn;
	const char *query = "DELETE FROM `tbl_files` " \
        " WHERE `path`='%s' and `sub`='%s' and `fname`='%s'";

    if (!strcmp(path, MOTION_DIR)) goto _end;

    conn = user_mysql_connect("localhost", "root", "kernel.bz");
    if (!conn) goto _exit;
    if (user_mysql_select_db(conn, "db_razig") < 0) goto _exit;

    printf ("DELETE: %s, %s, %s\r\n", path, subdir, fname);

    memset(qbuf, 0, sizeof(qbuf));
    str_char_replace(fname, '\'', '`');
    sprintf(qbuf, query, path, subdir, fname);
    ret = user_mysql_query(conn, qbuf);
    ret = user_mysql_query(conn, "OPTIMIZE TABLE `tbl_files`");

_exit:
    user_mysql_close(conn);

_end:
    return ret;
}

int dir_files_delete_all(void)
{
	int ret=0;
	MYSQL *conn;

    conn = user_mysql_connect("localhost", "root", "kernel.bz");
    if (!conn) goto _exit;
    if (user_mysql_select_db(conn, "db_razig") < 0) goto _exit;

    ret = user_mysql_query(conn, "DELETE FROM `tbl_files`");
    ret = user_mysql_query(conn, "ALTER TABLE `tbl_files` auto_increment=1");
    ret = user_mysql_query(conn, "OPTIMIZE TABLE `tbl_files`");

_exit:
    user_mysql_close(conn);

    return ret;
}

int dir_files_collect(char *path, char *subdir)
{
	struct dirent *entry;
	DIR *dir;
	char buf[80] = {0};
	char fname[160] = {0};
	char dtime[20];
	int cnt=0;
	struct stat sb;
	struct tm *tms;
	char qbuf[256];
	MYSQL *conn;
	const char *query = "INSERT INTO `tbl_files` " \
        "(`path`, `sub`, `fname`, `dtime`, `size` " \
        ") VALUES ( '%s', '%s', '%s', '%s', %d );";

    strcpy(buf, path);
    if (dir_sub_check(buf, subdir) <= 0) return -1;

    conn = user_mysql_connect("localhost", "root", "kernel.bz");
    if (!conn) goto _exit;
    if (user_mysql_select_db(conn, "db_razig") < 0) goto _exit;

    dir = opendir (buf);
    while ((entry = readdir (dir)) != NULL)
    {
        if (strlen(entry->d_name) > 5) {
            strcpy(fname, buf);
            strcat(fname, entry->d_name);
            stat(fname, &sb);
            tms = localtime(&sb.st_mtime);
            sprintf(dtime, "%04d-%02d-%02d %02d:%02d:%02d"
                    , 2000+(tms->tm_year%100), tms->tm_mon+1, tms->tm_mday
                    , tms->tm_hour, tms->tm_min, tms->tm_sec
            );
            ///printf ("INSERT: %s, %s, %s, %s, %d\r\n", buf, subdir, entry->d_name
            ///        , dtime, (int)sb.st_size);

            memset(qbuf, 0, sizeof(qbuf));
            str_char_replace(entry->d_name, '\'', '`');
            sprintf(qbuf, query, buf, subdir, entry->d_name, dtime, (int)sb.st_size);
            user_mysql_query(conn, qbuf);
            cnt++;
        } //if
    } //while
    closedir(dir);

_exit:
    user_mysql_close(conn);

	return cnt;
}

void *thread_dir_media_check(void *arg)
{
    int ret;

    while (1) {
        memset(Path, 0, sizeof(Path));
        ret = dir_media_check(Path, RAZIG);
        if (ret > 0) {
            if (DirMediaFound == 0) {
                dir_files_delete_all();
                ret = dir_files_collect(Path, CAMERA);
                ret += dir_files_collect(Path, RECORD);
                ret += dir_files_collect(Path, ACTION);
                ///ret += dir_files_collect(MOTION_ACT, MOTION);
                ret += dir_files_collect(Path, MOTION);
                ret += dir_files_collect(Path, MUSIC);
                ret += dir_files_collect(Path, VIDEO);
            }
            if (ret >= 0) DirMediaFound++;
            if (DirMediaFound > 10) DirMediaFound = 10;

        } else {
            DirMediaFound = 0;
        }
        sleep(4);
    } //while

    return (void*)0;
}

void *thread_dir_notify(void *arg)
{
    int fd, wd;
    char buf[BUF_LEN]__attribute__((aligned(4)));
    ssize_t len, i=0;
    DIR_T *dirt = (DIR_T *)arg;

    fd = inotify_init();
    if (fd == -1) {
        printf("inotify_init() error in the dir_notify.\r\n");
        goto _exit;
    }
    wd = inotify_add_watch(fd, dirt->path, IN_MOVE | IN_CREATE | IN_DELETE);
    if (wd == -1) {
        printf("inotify_add_watch() error in the dir_notify.\r\n");
        goto _exit;
    }

_retry:
    i = 0;
    len = read(fd, buf, BUF_LEN);   ///block, nonblock in thread
    if (len < 0) {
        printf("read() error in the dir_notify.\r\n");
        sleep(1);
        goto _retry;
    }
    while ( i < len ) {
        struct inotify_event *event = ( struct inotify_event * ) &buf[i];

        if ( event->len ) {
          if ( event->mask & IN_CREATE ) {
            if (!(event->mask & IN_ISDIR)) {
              printf( "The file %s was created.\n", event->name );
              dir_files_insert(dirt->path, dirt->subdir, event->name);
            }
          }
          else if ( event->mask & IN_DELETE ) {
            if (!(event->mask & IN_ISDIR)) {
              printf( "The file %s was deleted.\n", event->name );
              dir_files_delete(dirt->path, dirt->subdir, event->name);
            }
          }
          else if ( event->mask & IN_MOVED_FROM ) {
            if (!(event->mask & IN_ISDIR)) {
              printf( "The file %s was moved from.\n", event->name );
              dir_files_delete(dirt->path, dirt->subdir, event->name);
            }
          }
          else if ( event->mask & IN_MOVED_TO ) {
            if (!(event->mask & IN_ISDIR)) {
              printf( "The file %s was moved to.\n", event->name );
              dir_files_update(dirt->path, dirt->subdir, event->name);
            }
          }
        } //if
        i += EVENT_SIZE + event->len;
    } //while

    sleep(2);
    goto _retry;

_exit:
    inotify_rm_watch(fd, wd);
    close(fd);
    return (void*)0;
}

int thread_create(DIR_T **dir, char *path, char *subdir, pthread_t *tid)
{
    int ret;

    *dir = malloc(sizeof(DIR_T));
    strcpy((*dir)->path, path);
    strcat((*dir)->path, subdir);
    strcpy((*dir)->subdir, subdir);
    ret = pthread_create(tid, NULL, thread_dir_notify, *dir);
    if (ret) {
        printf("thread_dir_notify(%s) error in the media_list.\r\n", subdir);
        return 0;
    } else {
        printf("thread_dir_notify(%s%s) created in the media_list.\r\n", path, subdir);
        return 1;
    }
}

void thread_delete(DIR_T *dir, pthread_t tid)
{
    void *tret = (void*)0;

    pthread_cancel(tid);
    pthread_join(tid, &tret);
    printf("thread_dir_notify(%s%s) delete in the media_list.\r\n", dir->path, dir->subdir);
    if (dir) free(dir);
}

int main(int argc, char **argv)
{
    int ret=0;
    uint32_t thread_cnt=0;
    pthread_t tid0, tid1, tid2, tid3, tid4, tid5, tid6, tid7;
    DIR_T *dir_music, *dir_video, *dir_camera, *dir_record
        , *dir_action, *dir_motion, *dir_motion_act;

    ///#define RAZIG       "/razig"
    ret = pthread_create(&tid0, NULL, thread_dir_media_check, NULL);
    if (ret) {
        printf("thread_dir_media_check() error in the media_list.\r\n");
        return -1;
    }

    while (1)
    {
        if (DirMediaFound==1 && thread_cnt==0) {
            ret = thread_create(&dir_music, Path, MUSIC, &tid1);
            thread_cnt += ret;
            ret = thread_create(&dir_video, Path, VIDEO, &tid2);
            thread_cnt += ret;
            ret = thread_create(&dir_camera, Path, CAMERA, &tid3);
            thread_cnt += ret;
            ret = thread_create(&dir_record, Path, RECORD, &tid4);
            thread_cnt += ret;
            ret = thread_create(&dir_action, Path, ACTION, &tid5);
            thread_cnt += ret;
            ret = thread_create(&dir_motion, Path, MOTION, &tid6);
            thread_cnt += ret;
            ret = thread_create(&dir_motion_act, MOTION_ACT, MOTION, &tid7);
            thread_cnt += ret;

        } else if (DirMediaFound==0 && thread_cnt > 0) {
            thread_cnt = 0;
            thread_delete(dir_music, tid1);
            thread_delete(dir_video, tid2);
            thread_delete(dir_camera, tid3);
            thread_delete(dir_record, tid4);
            thread_delete(dir_action, tid5);
            thread_delete(dir_motion, tid6);
            thread_delete(dir_motion_act, tid7);

            dir_files_delete_all();
        }
        sleep(2);
    } //while

    return 0;
}
