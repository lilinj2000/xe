/**
 * Copyright 2017 ACCELECOM LTD.  All Rights Reserved.
 *  @file userdemo.cpp
 *  @company 南京艾科朗克信息科技有限公司
 *  @author shuaiw
 *  @modify changfengc
 *  @source encoding UTF-8
 *  @console encoding GB2312
 *  @Formatter GNU Built-in
 *  @brief 一个简单的例子，介绍CXeleTraderApi和CXeleTraderSpi接口的使用。
 *	本例将演示一个报单录入操作的过程,以及以下查询的结构体构建：
 *   	1. 持仓查询(Position)
 *  	2. 报单查询(Order)
 *  	3. 成交单查询(Trade)
 *   	4. 资金查询(Account)
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
#include <sched.h>
/*
 * 辅助函数: 读取配置文件refer.ini
 */

static char g_UserID_str[MEMB_SIZEOF(CXeleFtdcReqUserLoginField, UserID)];
static char g_Password_str[MEMB_SIZEOF(CXeleFtdcReqUserLoginField, Password)];
static char g_ParticipantID_str[MEMB_SIZEOF(CXeleFtdcReqUserLoginField,
					    ParticipantID)];
static char g_ClientID_str[MEMB_SIZEOF(CXeleFtdcInputOrderField, ClientID)];
static char g_AccountID_str[MEMB_SIZEOF(CXeleFtdcQryClientAccountField,
					AccountID)];
static char g_FrontAddress_str[80];

static int
iniLoad (char* buf, const char* item, const char* iniName)
{
  static const char sed_fmt[] = "sed 's/%s=\\(.*\\)/\\1/g;tx;d;:x' %s";
  char shell_cmd[160];
  FILE* ini;

  sprintf (shell_cmd, sed_fmt, item, iniName);
  ini = popen (shell_cmd, "r");
  fscanf (ini, "%s\n", buf);
  pclose (ini);
  return 0;
}
static void
loadConfigFile (char* iniName)
{
  if (iniName == NULL || iniName[0] == 0)
    {
      return;
    }

  memset (g_UserID_str, 0, sizeof(g_UserID_str));
  iniLoad (g_UserID_str, "USERID", iniName);
  fprintf (stdout, "USERID:%s\n", g_UserID_str);

  memset (g_Password_str, 0, sizeof(g_Password_str));
  iniLoad (g_Password_str, "PASSWD", iniName);
  fprintf (stdout, "PASSWD:%s\n", g_Password_str);

  memset (g_ParticipantID_str, 0, sizeof(g_ParticipantID_str));
  iniLoad (g_ParticipantID_str, "PARTID", iniName);
  fprintf (stdout, "PARTID:%s\n", g_ParticipantID_str);

  memset (g_ClientID_str, 0, sizeof(g_ClientID_str));
  iniLoad (g_ClientID_str, "CIENTID", iniName);
  fprintf (stdout, "CIENTID:%s\n", g_ClientID_str);

  memset (g_AccountID_str, 0, sizeof(g_AccountID_str));
  iniLoad (g_AccountID_str, "INVESTORID", iniName);
  fprintf (stdout, "INVESTORID:%s\n", g_AccountID_str);

  memset (g_FrontAddress_str, 0, sizeof(g_FrontAddress_str));
  iniLoad (g_FrontAddress_str, "FRONTADDRESS", iniName);
  fprintf (stdout, "FRONTADDRESS:%s\n", g_FrontAddress_str);
}

/*
 *
 * 辅助函数: 填写login域
 */
static void
example_ReqUserLogin (CXeleFtdcReqUserLoginField *req)
{
  memset (req, 0, sizeof(*req));

  snprintf (req->UserID, sizeof(req->UserID), "%s", g_UserID_str);
  snprintf (req->Password, sizeof(req->Password), "%s", g_Password_str);
  snprintf (req->ParticipantID, sizeof(req->ParticipantID), "%s",
	    g_ParticipantID_str);
  req->DataCenterID = 25;
}

/*
 * 构造报单
 */
static void
example_makeup_order (CXeleFtdcInputOrderField *order)
{
  /* 会员代码 */
  snprintf (order->ParticipantID, sizeof(order->ParticipantID), "%s",
	    g_ParticipantID_str);
  /* 客户代码 */
  snprintf (order->ClientID, sizeof(order->ClientID), "%s", g_ClientID_str);
  /* 交易用户代码 */
  snprintf (order->UserID, sizeof(order->UserID), "%s", g_UserID_str);
  /* 合约代码 */
  strcpy (order->InstrumentID, "cu1705");
  /* 报单价格条件 */
  order->OrderPriceType = XELE_FTDC_OPT_LimitPrice;
  /* 买卖方向 */
  order->Direction = XELE_FTDC_D_Buy;
  /* 组合开平标志 */
  order->CombOffsetFlag[0] = XELE_FTDC_OF_Open;
  /* 组合投机套保标志 */
  order->CombHedgeFlag[0] = XELE_FTDC_HF_Speculation;
  /* 价格 */
  order->LimitPrice = 35960;
  /* 数量 */
  order->VolumeTotalOriginal = 10;
  /* 有效期类型 */
  order->TimeCondition = XELE_FTDC_TC_GFD;
  /* 成交量类型 */
  order->VolumeCondition = XELE_FTDC_VC_AV;
  /* 最小成交量 */
  order->MinVolume = 0;
  /* 触发条件 */
  order->ContingentCondition = XELE_FTDC_CC_Immediately;
  /* 止损价 */
  order->StopPrice = 0;
  /*  强平原因 */
  order->ForceCloseReason = XELE_FTDC_FCC_NotForceClose;
  /* 自动挂起标志 */
  order->IsAutoSuspend = 0;
}

/*
 * @class CSimpleOrderManager
 * @brief 利用报单管理场景，演示本API接口调用。
 * @notes CXeleTraderSpi中OnXXX()形式的方法为回调函数，不能阻塞过长时间,
 *        否则，会影响接收其他来自交易柜台的消息。
 */
class CSimpleOrderManager : public CXeleTraderSpi
{
public:
  CXeleFtdcReqUserLoginField login_info;

  /* 构造函数，需要一个有效的指向CXeleTraderApi实例的指针 */
  CSimpleOrderManager (CXeleTraderApi* pTraderApi) :
      m_pTraderApi (pTraderApi)
  {
  }

  ~CSimpleOrderManager ()
  {
  }

  /* 当客户端程序与交易柜台建立起通信连接，客户端程序需要进行登录 */
  virtual void
  OnFrontConnected ()
  {
    fprintf (stdout, "{%s}\n", __FUNCTION__);
    example_ReqUserLogin (&login_info);
    m_RspEvent = false;
    m_pTraderApi->ReqUserLogin (&login_info, 0);
  }

  /* 当客户端程序与交易柜台通信连接断开时，该方法被调用 */
  virtual void
  OnFrontDisconnected (int nReason)
  {
    fprintf (stdout, "OnFrontDisconnected:%d\n", nReason);
  }

  /* 当客户端程序发出登录请求之后，该方法会被调用，通知客户端程序登录是否成功  */
  virtual void
  OnRspUserLogin (CXeleFtdcRspUserLoginField *pRspUserLogin,
		  CXeleFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
  {

    if (pRspInfo->ErrorID != 0)
      {
	/* 失败， 客户端程序需进行错误处理  */
	PRINT_RSP_ERR(pRspInfo);
	exit (pRspInfo->ErrorID);
      }
    else
      {
	/* 成功，处理数据...  */
	PRINT_RSP(pRspInfo);
      }

    m_RspEvent = true;
  }

  virtual void
  OnRspOrderInsert (CXeleFtdcInputOrderField *pInputOrder,
		    CXeleFtdcRspInfoField *pRspInfo, int nRequestID,
		    bool bIsLast)
  {
    if (pRspInfo->ErrorID != 0)
      {
	/* 失败， 客户端程序需进行错误处理  */
	PRINT_RSP_ERR(pRspInfo);
	exit (pRspInfo->ErrorID);
      }
    else
      {
	/* 成功，处理数据...  */
	PRINT_RSP(pRspInfo);
      }

    m_RspEvent = true;
  }

  /* 报单回报 */
  virtual void
  OnRtnOrder (CXeleFtdcOrderField *pOrder)
  {
    printf ("OnRtnOrder:\n");
    printf ("OrderSysID=[%s]\n", pOrder->OrderSysID);
  }

  /* 针对用户请求的出错通知 */
  virtual void
  OnRspError (CXeleFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
  {
    PRINT_RSP(pRspInfo);
    /* 客户端程序需进行错误处理 */
    exit (-1);
  }

  /* 持仓查询请求 */
  void
  ReqQryClientPosition ()
  {
    CXeleFtdcQryClientPositionField qryPosition;
    memset (&qryPosition, 0, sizeof(CXeleFtdcQryClientPositionField));
    /* 客户代码 */
    snprintf (qryPosition.ClientID, sizeof(qryPosition.ClientID), "%s",
	      g_ClientID_str);
    /* 合约代码 */
    // strcpy (qryPosition.InstrumentID, "cu1705");
    /* 客户类型 */
    // qryPosition.ClientType = XELE_FTDC_CT_Company;
    m_RspEvent = false;

    m_pTraderApi->ReqQryClientPosition (&qryPosition, 1);
  }

  /* 持仓查询应答 */
  virtual void
  OnRspQryClientPosition (CXeleFtdcRspClientPositionField *pRspClientPosition,
			  CXeleFtdcRspInfoField *pRspInfo, int nRequestID,
			  bool bIsLast)
  {
    if (pRspInfo->ErrorID != 0)
      {
	/* 失败， 客户端程序需进行错误处理  */
	PRINT_RSP_ERR(pRspInfo);
	exit (pRspInfo->ErrorID);
      }
    else
      {
	/*  */
	PRINT_RSP(pRspInfo);
      }

    m_RspEvent = true;
  }

  /* 资金查询请求 */
  void
  ReqQryClientAccount ()
  {
    CXeleFtdcQryClientAccountField qryAccount;
    memset (&qryAccount, 0, sizeof(CXeleFtdcQryClientAccountField));
    /* 客户代码 */
    snprintf (qryAccount.ClientID, sizeof(qryAccount.ClientID), "%s",
	      g_ClientID_str);
    /* 资金帐号 */
    snprintf (qryAccount.AccountID, sizeof(qryAccount.AccountID), "%s",
	      g_AccountID_str);
    m_RspEvent = false;

    m_pTraderApi->ReqQryClientAccount (&qryAccount, 1);
  }

  /* 资金查询应答 */
  virtual void
  OnRspQryClientAccount (CXeleFtdcRspClientAccountField *pClientAccount,
			 CXeleFtdcRspInfoField *pRspInfo, int nRequestID,
			 bool bIsLast)
  {
    if (pRspInfo->ErrorID != 0)
      {
	/* 失败， 客户端程序需进行错误处理  */
	PRINT_RSP_ERR(pRspInfo);
	exit (pRspInfo->ErrorID);
      }
    else
      {
	/* 成功，处理数据...  */
	PRINT_RSP(pRspInfo);
      }

    m_RspEvent = true;
  }

  /* 报单查询请求 */
  void
  ReqQryOrder ()
  {
    CXeleFtdcQryOrderField qryOrder;
    memset (&qryOrder, 0, sizeof(CXeleFtdcQryOrderField));
    /* 客户代码 */
    snprintf (qryOrder.ClientID, sizeof(qryOrder.ClientID), "%s",
	      g_ClientID_str);
    m_RspEvent = false;

    m_pTraderApi->ReqQryOrder (&qryOrder, 1);
  }

  /* 报单查询应答 */
  virtual void
  OnRspQryOrder (CXeleFtdcOrderField* pOrderField,
		 CXeleFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
  {
    if (pRspInfo->ErrorID != 0)
      {
	/* 失败， 客户端程序需进行错误处理  */
	PRINT_RSP_ERR(pRspInfo);
	exit (pRspInfo->ErrorID);
      }
    else
      {
	/* 成功，处理数据...  */
	PRINT_RSP(pRspInfo);
      }

    m_RspEvent = true;
  }

  /* 成交单查询请求 */
  void
  ReqQryTrade ()
  {
    CXeleFtdcQryTradeField qryTrade;
    memset (&qryTrade, 0, sizeof(CXeleFtdcQryTradeField));
    /* 客户代码 */
    snprintf (qryTrade.ClientID, sizeof(qryTrade.ClientID), "%s",
	      g_ClientID_str);
    /* 合约代码 */
    strcpy (qryTrade.InstrumentID, "cu1705");
    /* 成交编号 */
    strcpy (qryTrade.InstrumentID, "233");
    m_RspEvent = false;

    m_pTraderApi->ReqQryTrade (&qryTrade, 1);
  }

  /* 成交单查询应答 */
  virtual void
  OnRspQryTrade (CXeleFtdcTradeField* pTradeField,
		 CXeleFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
  {

    if (pRspInfo->ErrorID != 0)
      {
	/* 失败， 客户端程序需进行错误处理  */
	PRINT_RSP_ERR(pRspInfo);
	exit (pRspInfo->ErrorID);
      }
    else
      {
	/* 成功，处理数据...  */
	PRINT_RSP(pRspInfo);
      }

    m_RspEvent = true;
  }

  /* 等待响应事件 */
  void
  WaitRspEvent ()
  {
    do
      {
	sched_yield();
      }
    while (true != m_RspEvent);
  }

  /* 测试流程：风控管理 */
  bool
  SimpleRiskManagement ()
  {

    /* 资金查询应答 */
    ReqQryClientAccount ();
    WaitRspEvent ();

    /* 持仓查询应答 */
    ReqQryClientPosition ();
    WaitRspEvent ();

    /* 成交单查询请求 */
    ReqQryTrade ();
    WaitRspEvent ();

    /* 报单查询请求 */
    ReqQryOrder ();
    WaitRspEvent ();

    return true;
  }

  /* 测试流程：报单管理 */
  void
  SimpleOrderManagement ()
  {
    /* 等待登入成功 */
    WaitRspEvent ();

    /* 检查是否符合风控 */
    if (SimpleRiskManagement ())
      {
	/* 满足风控条件,发送报单 */
	CXeleFtdcInputOrderField order;
	memset (&order, 0, sizeof(CXeleFtdcInputOrderField));
	example_makeup_order (&order);
	m_pTraderApi->ReqOrderInsert (&order, 1);
	WaitRspEvent ();
      }
  }

private:
  /* 指向CXeleTraderApi实例的指针 */
  CXeleTraderApi* m_pTraderApi;

  /* 响应事件标志 */
  bool m_RspEvent;
};

/*
 * 主函数:
 */
int
main (int argc, char* argv[])
{
  /*
   * 读取refer.ini
   */
  loadConfigFile ((char*) "config_userdemo.ini");

  std::string msg;

  /*
   * 创建对象, exchange=1(上期所），0或其他（中金所）
   */
  int exchangeID = 1;
  CXeleTraderApi* pTraderApi = CXeleTraderApi::CreateTraderApi (exchangeID);
  assert (pTraderApi);
  CSimpleOrderManager som (pTraderApi);
  pTraderApi->RegisterSpi (&som);

  pTraderApi->RegisterFront (g_FrontAddress_str);

  /*
   * 准备login的结构体
   */
  /*
   * 订阅相应的流
   */
  pTraderApi->SubscribePrivateTopic (XELE_TERT_RESTART);
  pTraderApi->SubscribePublicTopic (XELE_TERT_RESTART);
  /*
   * 开始登录, 使客户端程序开始与交易柜台建立连接
   */
  pTraderApi->Init ();

  /*
   * 打印API信息
   */
  fprintf (stdout, "%s\n", pTraderApi->GetVersion ());

  /*
   * 实例流程：报单管理
   */
  som.SimpleOrderManagement ();

  /*
   * 主线程打印
   */
  do
    {
      cout << "Input 'q' to disconnect API:";
      getline (cin, msg);
    }
  while (msg != "q");

  /*
   * 等待API实例退出
   */
  pTraderApi->Join ();

  /*
   * 释放API实例
   */
  pTraderApi->Release ();
  fprintf (stdout, "API release done. Exit Demo.\n");

  return 0;
}
