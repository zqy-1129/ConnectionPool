#include "pch.h"
#include "public.h"
#include "ConnectionPool.h"

ConnectionPool::ConnectionPool()
{
    // 加载配置项
    if (!loadConfigFile()) 
    {
        return;
    }
    // 创建初始数量连接
    for (int i = 0; i < _initSize; ++i) 
    {
        Connection *p = new Connection();
        p->connect(_ip, _port, _username, _password, _dbname);
        p->refreshAliveTime();
        _connectionQueue.push(p);
        _connectionCnt++;
    }

    // 启动一个线程作为连接生产者
    thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
    produce.detach();

    // 启动一个新的定时线程，扫描超过maxIdleTime的空闲连接，进行对于连接的回收
    thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach();
}

// 线程安全的懒汉单例函数接口
ConnectionPool* ConnectionPool::getConnectionPool()
{
    static ConnectionPool pool;
    return &pool;
}

// 从配置文件中加载配置项
bool ConnectionPool::loadConfigFile()
{
    FILE *pf = fopen("mysql.ini", "r");
    if (pf == nullptr)
    {
        LOG("mysql.ini file is not exist!");
        return false;
    }

    while (!feof(pf)) 
    {
        char line[1024] = { 0 };
        fgets(line, 1024, pf);
        string str = line;
        int idx = str.find('=', 0);
        if (idx == -1) // 无效配置项
        {
            continue;
        }
        int endidx = str.find('\n', idx);
        string key = str.substr(0, idx);
        string value = str.substr(idx + 1, endidx - idx - 1);
        
        if (key == "ip")
        {
            _ip = value;
        }
        else if (key == "port")
        {
            _port = atoi(value.c_str());
        }
        else if (key == "dbname")
        {
            _dbname = value;
        }
        else if (key == "username")
        {
            _username = value;
        }
        else if (key == "password")
        {
            _password = value;
        }
        else if (key == "initSize")
        {
            _initSize = atoi(value.c_str());
        }
        else if (key == "maxIdleTime")
        {
            _maxIdleTime = atoi(value.c_str());
        }
        else if (key == "connectionTimeOut")
        {
            _connectionTimeout = atoi(value.c_str());
        }
    }
    fclose(pf);
    return true;
}

// 在独立线程中生产新连接
void ConnectionPool::produceConnectionTask()
{   
    for (;;)
    {   
        unique_lock<mutex> lock(_queueMutex);
        while (!_connectionQueue.empty())
        {
            cv.wait(lock);
        }
        // 连接数量未达上限继续创建
        if (_connectionCnt < _maxSize)
        {
            Connection *p = new Connection();
            p->connect(_ip, _port, _username, _password, _dbname);
            p->refreshAliveTime();
            _connectionQueue.push(p);
            _connectionCnt++;
        }

        // 通知消费者消费连接
        cv.notify_all();
    }
}

// 在独立的线程中扫描回收连接
void ConnectionPool::scannerConnectionTask() 
{
    for (;;) 
    {
        this_thread::sleep_for(chrono::seconds(_maxIdleTime));
        // 扫描整个队列，释放多余连接
        unique_lock<mutex> lock(_queueMutex);
        while (_connectionCnt > _initSize)
        {
            Connection *p = _connectionQueue.front();
            if (p->getAliveTime() > _maxIdleTime * 1000)
            {
                _connectionQueue.pop();
                // 释放连接
                delete p; 
            }
            else 
            {
                // 对头连接没有超时则之后不可能超时
                break;
            }
        }
    }
}

// 给外部的接口，从连接池中获取一个可用的空闲连接
shared_ptr<Connection> ConnectionPool::getConnection()
{
    unique_lock<mutex> lock(_queueMutex);
    while (_connectionQueue.empty())
    {
        if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
        {
            if (_connectionQueue.empty())
            {
                LOG("get connection timeout!");
                return nullptr;
            }
        }        
    } 
    /**
     * 智能指针析构会将连接资源释放，因此需要自定义智能指针释放方式
     * 将连接重新放入队列中
     */
    shared_ptr<Connection> sp(_connectionQueue.front(),
        [&](Connection *conn) {
            // 对于队列的操作需要保证线程安全
            unique_lock<mutex> lock(_queueMutex);
            conn->refreshAliveTime();
            _connectionQueue.push(conn);
        });
    _connectionQueue.pop();
    // 通知生产者线程生产
    cv.notify_all();    
    return sp;
}