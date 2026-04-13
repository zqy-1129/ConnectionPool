#pragma once
#include <string>
#include <queue>
#include <Connection.h>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>
using namespace std;

/**
 * 实现连接池模块
 */
class ConnectionPool {
public:
    static ConnectionPool* getConnectionPool();

    // 给外部的接口，从连接池中获取一个可用的空闲连接
    shared_ptr<Connection> getConnection();

private:
    ConnectionPool();  

    bool loadConfigFile();                  // 从配置文件中加载配置项

    void produceConnectionTask();           // 在独立的线程中，生产新连接
    void scannerConnectionTask();           // 在独立的线程中，扫描回收连接

    string _ip;                             // mysql的ip地址
    unsigned short _port;                   // mysql的端口号
    string _dbname;                         // mysql的数据库名称
    string _username;                       // mysql登录用户名
    string _password;                       // mysql登录密码
    int _initSize;                          // 连接池初始连接量
    int _maxSize;                           // 连接池最大连接量
    int _maxIdleTime;                       // 连接池最大空闲时间
    int _connectionTimeout;                 // 连接池获取连接的超时时间

    atomic_int _connectionCnt;              // 创建连接的总数
    queue<Connection*> _connectionQueue;    // 存储mysql连接的队列
    mutex _queueMutex;                      // 维护连接队列的线程安全互斥锁
    condition_variable cv;                  // 条件变量，用于连接生产线程和连接消费线程的通信
};