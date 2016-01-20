/**
 *	@file reference.cpp
 *  @author shuaiw
 */
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <assert.h>
using namespace std;

#include <CXeleTraderApi.hpp>
#include <DemoUtils.h>
/*
 * 辅助函数: 读取配置文件refer.ini
 */

static char UserID_str[MEMB_SIZEOF(CXeleFtdcReqUserLoginField, UserID)];
static char Password_str[MEMB_SIZEOF(CXeleFtdcReqUserLoginField, Password)];
static char ParticipantID_str[MEMB_SIZEOF(CXeleFtdcReqUserLoginField,
        ParticipantID)];
static char FrontAddress_str[80];

static int iniLoad(char* buf, const char* item, const char* iniName)
{
    static const char sed_fmt[] = "sed 's/%s=\\(.*\\)/\\1/g;tx;d;:x' %s";
    char shell_cmd[160];
    FILE* ini;

    sprintf(shell_cmd, sed_fmt, item, iniName);
    ini = popen(shell_cmd, "r");
    fscanf(ini, "%s\n", buf);
    pclose(ini);
}
static void loadConfigFile(char* iniName)
{
    if (iniName == NULL || iniName[0] == 0)
    {
    }

    char config_linebuf[160];
    memset(UserID_str, 0, sizeof(UserID_str));
    iniLoad(UserID_str, "USERID", iniName);
    fprintf(stdout, "USERID:%s\n", UserID_str);

    memset(Password_str, 0, sizeof(Password_str));
    iniLoad(Password_str, "PASSWD", iniName);
    fprintf(stdout, "PASSWD:%s\n", Password_str);

    memset(ParticipantID_str, 0, sizeof(ParticipantID_str));
    iniLoad(ParticipantID_str, "PARTID", iniName);
    fprintf(stdout, "PARTID:%s\n", ParticipantID_str);

    memset(FrontAddress_str, 0, sizeof(FrontAddress_str));
    iniLoad(FrontAddress_str, "FRONTADDRESS", iniName);
    fprintf(stdout, "FRONTADDRESS:%s\n", FrontAddress_str);
}

/*
 *
 * 辅助函数: 填写login域
 */
static void example_ReqUserLogin(CXeleFtdcReqUserLoginField *req)
{
	memset(req, 0, sizeof(*req));

    snprintf(req->UserID, sizeof(req->UserID), "%s", UserID_str);
    snprintf(req->Password, sizeof(req->Password), "%s", Password_str);
    snprintf(req->ParticipantID, sizeof(req->ParticipantID), "%s", ParticipantID_str);
    req->DataCenterID = 25;
}

static void example_ReqOrderInsert(CXeleFtdcInputOrderField *order)
{
    snprintf(order->ParticipantID, sizeof(order->ParticipantID), "%s", "your_ParticipantID");
    snprintf(order->ClientID, sizeof(order->ClientID), "%s", "your_clientID");
    snprintf(order->UserID, sizeof(order->UserID), "%s", "your_UserID");
    snprintf(order->InstrumentID, sizeof(order->InstrumentID), "%s", "your_InstrumentID");
    order->OrderPriceType = XELE_FTDC_OPT_AnyPrice;
    order->Direction = XELE_FTDC_D_Buy;
    sprintf(order->CombOffsetFlag, "%c", '0');
    sprintf(order->CombHedgeFlag, "%c", '1');
    order->LimitPrice = 100.1234;
    order->VolumeTotalOriginal = 1;
    order->TimeCondition = XELE_FTDC_TC_IOC;
    order->VolumeCondition = XELE_FTDC_VC_AV;
    order->MinVolume = 0;
    order->ContingentCondition = XELE_FTDC_CC_Immediately;
    order->StopPrice = 0.0;
    order->ForceCloseReason = XELE_FTDC_FCC_NotForceClose;
    order->IsAutoSuspend = 0;
}

/*
 * 示例Spi
 */
struct ExampleSpi: public CXeleTraderSpi
{
public:
    CXeleTraderApi* &papi;
    CXeleFtdcReqUserLoginField login_info;

	ExampleSpi(CXeleTraderApi* &api):papi(api)
	{}

    virtual void OnFrontConnected()
    {
	    fprintf(stdout, "{%s}\n", __FUNCTION__);
    	example_ReqUserLogin(&login_info);
    	papi->ReqUserLogin(&login_info, 0xdeadbeef);
    }

    virtual void OnFrontDisconnected(int nReason)
    {
        fprintf(stdout, "OnFrontDisconnected:%d\n", nReason);
    }

    virtual void OnRspUserLogin(CXeleFtdcRspUserLoginField *pRspUserLogin,
                                             CXeleFtdcRspInfoField *pRspInfo,
                                             int nRequestID,
                                             bool bIsLast)
    {
        fprintf(stdout, "<RspUserLogin> ErrorID:%d, ErrorMsg:%s\n",pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }


};

/*
 * 主函数:
 */
int main(int argc, char* argv[])
{
    /*
     * 读取refer.ini
     */
    loadConfigFile("config_userdemo.ini");

    std::string msg;

    /*
     * 创建对象
     */
    CXeleTraderApi* api = CXeleTraderApi::CreateTraderApi();
	assert(api);
    ExampleSpi spi(api);
    api->RegisterSpi(&spi);

    api->RegisterFront(FrontAddress_str);

    /*
     * 准备login的结构体
     */
    /*
     * 订阅相应的流
     */
    api->SubscribePrivateTopic(XELE_TERT_RESTART);
    /*
     * 开始登录
     */
    api->Init();

    /*
     * 打印API信息
     */
    fprintf(stdout, "%s\n", api->GetVersion());
    /*
     * 主线程打印
     */
    do
    {
        cout << "Input 'q' to disconnect API:";
        getline(cin, msg);
    } while (msg != "q");
    api->Release();
    fprintf(stdout, "API release done. Exit Demo.\n");
}
