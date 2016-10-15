/**
 *	file name:	media_play.c
 *	author:		JungJaeJoon (rgbi3307@nate.com) on the www.kernel.bz
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
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>

#define  BUFF_SIZE   160

static void media_play_run(char *run)
{
    strcat(run, ".sh &");
    system(run);
}

#if 0
static void media_play(char *run)
{
	FILE *fp;
	char buffer[BUFF_SIZE] = {0, };
	const char *kill_act[] = { "killall mplayer", "killall raspistill" };
	const char *check_act[] = { "mplayer -noconsolecontrols", "raspistill -v" };
	int idx = 0;

	while (1)
	{
        if (strstr(run, "camera")) {
            fp = popen("ps aux | grep raspistill -v", "r");
            idx = 1;
        } else {
            fp = popen("ps aux | grep mplayer -noconsolecontrols", "r");
            idx = 0;
        }

		if (fp) {
			fread(buffer, sizeof(char), BUFF_SIZE, fp);

			///printf("%s\n", buffer);

			system(kill_act[idx]);
			sleep(1);

			if (!strstr(buffer, check_act[idx])) {
				media_play_run(run);
				goto _exit;
			}
		} else {
			media_play_run(run);
			goto _exit;
		}

		pclose(fp);
		memset(buffer, 0, sizeof(buffer));
		sleep(1);
	}

_exit:
	pclose(fp);
}
#endif

static int file_read_play(const char *fname)
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
        ///printf( "read: %s\n", buf);

		media_play_run(buf);

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
	char *path = "../../data/media/";
	char fname[32] = {0};

	while (1)
	{
		dir = opendir (path);
		while ((entry = readdir (dir)) != NULL) {
			if (strlen(entry->d_name) > 5) {
				strcat(fname, path);
				strcat(fname, entry->d_name);

				///printf ("fname = %s\n", fname);

				if (strstr(fname, "media_")) file_read_play(fname);

				///sleep(1);
				memset(fname, 0, sizeof(fname));
			}
		} //while
		closedir(dir);

		sleep(1);
	}

	return 0;
}
