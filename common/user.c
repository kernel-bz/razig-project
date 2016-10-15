/**----------------------------------------------------------------------------
 * Name:    user.c
 * Purpose: user common module
 * Author:	JungJaeJoon on the www.kernel.bz
 *-----------------------------------------------------------------------------
 * Notes:
 *-----------------------------------------------------------------------------
 */

#include <string.h>

void user_str_char_replace(char *s, char t, char r)
{
    int len = strlen(s);
    int i;

    for (i=0; i<len; i++) {
        if (s[i] == t) s[i] = r;
    }
}

