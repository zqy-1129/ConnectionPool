#include "Connection.h"
#include "ConnectionPool.h"
#include <iostream>
#include <mysql.h>
#include <thread>
using namespace std;

// 每个线程要执行的插入任务
void insertTask(int threadId, int insertCount)
{
    ConnectionPool* cp = ConnectionPool::getConnectionPool();

    for (int i = 0; i < insertCount; ++i)
    {
        // 使用多线程不使用连接池 1000组: 2915ms 5000组: 13352ms
        Connection conn;
        char sql[1024] = { 0 };
        sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')",
            "zhang san", 20, "male");
        conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
        conn.update(sql);

        // 使用多线程和连接池 1000组: 2430ms 5000组: 9716ms
        // shared_ptr<Connection> sp = cp->getConnection();
        // if (!sp) continue;

        // char sql[1024] = { 0 };
        // snprintf(sql, sizeof(sql) - 1,
        //     "insert into user(name, age, sex) values('%s', %d, '%s')",
        //     "zhang san", 20, "male");

        // sp->update(sql);
    }
}

int main()
{
    // 总插入次数
    const int TOTAL_COUNT = 5000;
    // 线程数量
    const int THREAD_NUM = 4;
    // 每个线程插入次数
    const int PER_THREAD_COUNT = TOTAL_COUNT / THREAD_NUM;

    clock_t begin = clock();

    // 创建 4 个线程
    vector<thread> threads;
    for (int i = 0; i < THREAD_NUM; ++i)
    {
        threads.emplace_back(insertTask, i, PER_THREAD_COUNT);
    }

    // 等待所有线程结束
    for (auto& t : threads)
    {
        if (t.joinable())
            t.join();
    }

    clock_t end = clock();
    cout << "use:" << end - begin << "ms" << endl;
    return 0;
}


#if 0
int main() 
{
    // Connection conn;
    // char sql[1024] = { 0 };
    // sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')",
    //     "zhang san", 20, "male");
    // conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
    // conn.update(sql);
    
    clock_t begin = clock();
    ConnectionPool *cp = ConnectionPool::getConnectionPool();
    if (cp == nullptr) {
        cerr << "ConnectionPool init failed!" << endl;
        return -1;
    }
    for (int i = 0; i < 5000; i++) 
    {   
        // 不使用连接池 1000组: 7110ms 5000组: 35714ms
        // Connection conn;
        // char sql[1024] = { 0 };
        // sprintf(sql, "insert into user(name, age, sex) values('%s', %d, '%s')",
        //     "zhang san", 20, "male");
        // conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
        // conn.update(sql);

        // 使用连接池 1000组: 3590ms 5000组: 19192ms
        shared_ptr<Connection> sp = cp->getConnection();
        char sql[1024] = { 0 };
        snprintf(sql, sizeof(sql) - 1, 
                 "insert into user(name, age, sex) values('%s', %d, '%s')",
                 "zhang san", 20, "male");
        sp->update(sql);
    }
    clock_t end = clock();
    cout << "use: " << end - begin << "ms" << endl;
    return 0;   
}
#endif