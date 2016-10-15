/**----------------------------------------------------------------------------
 * Name:    user_mysql.c
 * Purpose: mysql module
 * Author:	JungJaeJoon on the www.kernel.bz
 *-----------------------------------------------------------------------------
 * Notes:
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include "/usr/include/mysql/mysql.h"

MYSQL *user_mysql_connect(char *server, char *user, char *passwd)
{
    MYSQL *conn;

    if (!(conn=mysql_init((MYSQL*)NULL))) {
        printf("mysql_init() fail.\r\n");
        return NULL;
    }

    if (!mysql_real_connect(conn, server, user, passwd, NULL, 3306, NULL, 0)) {
        printf("mysql_real_connect() error.\r\n");
        mysql_close(conn);
        return NULL;
    }

    return conn;
}

void user_mysql_close(MYSQL *conn)
{
    if (conn) mysql_close(conn);
}

int user_mysql_select_db(MYSQL *conn, char *dbname)
{
    if (!conn) return -1;

    if (mysql_select_db(conn, dbname) != 0) {
        printf("mysql_select_db(%s) error.\r\n", dbname);
        return -2;
    }
    return 0;
}

int user_mysql_query(MYSQL *conn, char *query)
{
    if (!conn) return -1;

    if (mysql_query(conn, query) != 0) {
        printf("mysql_query() error.\r\n");
        return -2;
    }

    return 0;
}

int user_mysql_query_select(MYSQL *conn, const char *query, char (*buf)[60])
{
    MYSQL_RES *res;
    MYSQL_ROW row;

    if (!conn) return -1;

    if (mysql_query(conn, query) != 0) {
        printf("mysql_query() error.\r\n");
        return -2;
    }

    res = mysql_store_result(conn);
    row = mysql_fetch_row(res);
    if (row != NULL) {
        strcpy(buf[0], row[0]);
        strcpy(buf[1], row[1]);
    }

    /**
    while ((row=mysql_fetch_row(res)) != NULL) {
        buf = row[0];
    }
    */

    return 0;
}

MYSQL_RES *user_mysql_query_result(MYSQL *conn, const char *query)
{
    MYSQL_RES *res;

    if (!conn) return NULL;

    if (mysql_query(conn, query) != 0) {
        printf("mysql_query() error.\r\n");
        return NULL;
    }

    res = mysql_store_result(conn);
    return res;
}

