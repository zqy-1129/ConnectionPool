#pragma once
#include <string>
#include <mysql.h>
#include <ctime>
using namespace std;

/**
 * 实现MySQL数据库的增删改查
 */

class Connection
{
public:
    Connection();
    ~Connection();

    bool connect(string ip,
        unsigned short port,
        string username,
        string password,
        string dbname);

    bool update(string sql);

    MYSQL_RES* query(string sql);

    // 刷新连接的起始空闲时间点
    void refreshAliveTime() { _alivetime = clock(); }
    // 返回存活时间
    clock_t getAliveTime() const { return clock() - _alivetime; }

private:
    MYSQL *_conn;           // MySQL Server的一条连接
    clock_t _alivetime;     // 记录进入空闲状态后的存活时间
};
