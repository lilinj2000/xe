/*!
 * @file MarketProcessModel.h
 *
 * 
 * @author shuaiw
 */

#ifndef MARKETPROCESSMODEL_H_
#define MARKETPROCESSMODEL_H_
#include <pthread.h>
#include "DemoUtils.h"

#include <fstream>

#include "XeleFtdcMduserApi.h"
/*
 * 辅助函数: 打印marketData域
 */

class MarketModelBase
{
public:
    /*
     * 模型A: 如何获取最新的行情
     * @param m_thread 线程标识变量
     * @param m_api    mdapi对象指针
     * @param running  用来控制work线程是否循环运行
     */
    pthread_t m_thread;
    CXeleMdApi* m_api;
    int running;
    
    MarketModelBase(CXeleMdApi* api)
            : m_api(api), running(0)
    {
    }
    /*
     * @param
     */
    virtual void start() = 0;
    virtual void stop()
    {
        running = 0;
        pthread_join(m_thread, NULL);
    }
};

/*
 * 演示直接获取最新行情
 */
class MarketModel: public MarketModelBase
{
    const char* m_logName;
public:
    MarketModel(CXeleMdApi* api, const char* logname)
            : MarketModelBase(api), m_logName(logname)
    {
    }

    virtual void start()
    {
        /*
         *  Base类的纯虚方法start, 子类的实现, 替换相应的work_a就可以了
         */
        running = 1;
        pthread_create(&m_thread, NULL, MarketModel::work_a, this);
    }
    static void* work_a(void* arg)
    {
        //创建线程时传入的是MarketModel_A的this指针, 这里从void*还原其类型
        MarketModel *model = (MarketModel*) arg;
        //获取到api对象
        CXeleMdApi *api = model->m_api;
        //例子中获取行情, 并打印到title为名字的文件中的相关代码
        std::cerr << model->m_logName << std::endl;
        ofstream log(model->m_logName);
        /*
         * 获取行情的代码主体
         */
        MarketDataTick tick;
        while (model->running)
        {
           /*
            * 轮询检查行情tick
            */
            if(api->RecvMarketDataTick(&tick))
            {
                printDepthMarketData(log, model->m_logName, &tick.data);
            }
        }
        pthread_exit(NULL);
    }
};

#endif /* MARKETPROCESSMODEL_H_ */
