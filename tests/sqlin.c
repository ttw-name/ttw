#include <mysql/mysql.h>
#include <stdio.h>

int main() {
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    MYSQL_RES *result;
    MYSQL_ROW row;

    // 连接到数据库
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        return 1;
    }

    if (mysql_real_connect(conn, "localhost", "username", "password", "database", 0, NULL, 0) == NULL) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        mysql_close(conn);
        return 1;
    }

    // 准备 SQL 查询
    stmt = mysql_stmt_init(conn);
    if (stmt == NULL) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        mysql_close(conn);
        return 1;
    }

    if (mysql_stmt_prepare(stmt, "SELECT * FROM users WHERE username = ? AND password = ?", -1)) {
        fprintf(stderr, "%s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return 1;
    }

    // 绑定参数值
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = malloc(256);
    bind[0].buffer_length = 256;
    bind[0].is_null = 0;
    bind[0].length = 0;
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = malloc(256);
    bind[1].buffer_length = 256;
    bind[1].is_null = 0;
    bind[1].length = 0;

    strcpy((char *)bind[0].buffer, "john");
    strcpy((char *)bind[1].buffer, "secret");

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "%s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return 1;
    }

    // 执行查询
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "%s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return 1;
    }

    // 获取结果集
    result = mysql_stmt_result_metadata(stmt);
    while ((row = mysql_fetch_row(result))) {
        printf("ID: %s\n", row[0]);
        printf("Username: %s\n", row[1]);
        printf("Email: %s\n", row[2]);
    }

    // 释放资源
    mysql_free_result(result);
    mysql_stmt_close(stmt);
    mysql_close(conn);

    return 0;
}