/**----------------------------------------------------------------------------
 * Name:    user_mysql.h
 * Purpose: mysql module
 * Author:	JungJaeJoon on the www.kernel.bz
 *-----------------------------------------------------------------------------
 * Notes:
 *-----------------------------------------------------------------------------
 */

#ifndef USER_MYSQL_H_INCLUDED
#define USER_MYSQL_H_INCLUDED

#include "/usr/include/mysql/mysql.h"

MYSQL *user_mysql_connect(char *server, char *user, char *passwd);
void user_mysql_close(MYSQL *conn);
int user_mysql_select_db(MYSQL *conn, char *dbname);
int user_mysql_query(MYSQL *conn, char *query);
int user_mysql_query_select(MYSQL *conn, const char *query, char (*buf)[60]);
MYSQL_RES *user_mysql_query_result(MYSQL *conn, const char *query);

#endif // USER_MYSQL_H_INCLUDED
