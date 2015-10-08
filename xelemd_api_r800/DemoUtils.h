/*!
 * @file DemoUtils.h
 *
 * 
 * @author shuaiw
 */

#ifndef DEMOUTILS_H_
#define DEMOUTILS_H_
#include <stddef.h>
#include <iostream>
#include <iomanip>
/*
 * 求结构体成员字节大小
 */
#define MEMB_SIZEOF(st_type, member) sizeof(((st_type*)0)->member)
/*
 * 求结构体成员字节偏移
 */
#define MEMB_CHARPTR(ptr, st_type, member) ((char*)ptr + offsetof(st_type, member))
/*
 * 辅助宏: 打印marketData域字段
 */
#define M_INPUT(ptr, CField, member, src) memcpy(MEMB_CHARPTR(ptr, CField, member), src, MEMB_SIZEOF(CField, member))
#define S_INPUT(ptr, CField, member, src) strncpy(MEMB_CHARPTR(ptr, CField, member), src, MEMB_SIZEOF(CField, member))
#define STR(x) #x

#define S_OUTPUT(ptr, CField, member) do{ \
    cout << setw(20)<<STR(CField)<<"->"<<STR(member)<<right<<setw(20)<<" :"<< (ptr)->member << endl; \
}while(0)

#define FORMAT_OUT(out, item) do{ \
            out<<setw(20)<<STR(item)<<":"<<right<<setw(20)<< pDepthMarketData->item ;\
            out<< endl;\
}while(0)



inline std::ostream& _hexdump_oneline2(std::ostream &os,
                                       const void* vbuf,
                                       int sz)
{
    char * const str = new char[sz * 4 + 1];
    char* wp = str;

    int cur(0);
    unsigned char* buf = (unsigned char*) vbuf;

    while (cur < sz)
    {
        snprintf(wp, 3, "%02hhx", buf[cur++]);
        wp += 2;
    }
    *wp++ = '\0';
    os.write(str, wp - str);
    os << std::endl;
    delete[] str;
    return os;
}

void printDepthMarketData(ostream& log,
                          const char* title,
                          CXeleMdFtdcDepthMarketDataField *pDepthMarketData)
{
    log << "< " << title << " >" << endl;

    FORMAT_OUT(log, LastPrice);

    FORMAT_OUT(log, OpenPrice);
    FORMAT_OUT(log, HighestPrice);
    FORMAT_OUT(log, LowestPrice);
    FORMAT_OUT(log, Volume);
    FORMAT_OUT(log, Turnover);
    FORMAT_OUT(log, OpenInterest);
    FORMAT_OUT(log, ClosePrice);
    FORMAT_OUT(log, SettlementPrice);
    FORMAT_OUT(log, UpperLimitPrice);
    FORMAT_OUT(log, LowerLimitPrice);
    FORMAT_OUT(log, CurrDelta);
    FORMAT_OUT(log, BidPrice1);
    FORMAT_OUT(log, BidVolume1);
    FORMAT_OUT(log, AskPrice1);
    FORMAT_OUT(log, AskVolume1);
    FORMAT_OUT(log, BidPrice2);
    FORMAT_OUT(log, BidVolume2);
    FORMAT_OUT(log, AskPrice2);
    FORMAT_OUT(log, AskVolume2);
    FORMAT_OUT(log, BidPrice3);
    FORMAT_OUT(log, BidVolume3);
    FORMAT_OUT(log, AskPrice3);
    FORMAT_OUT(log, AskVolume3);
    FORMAT_OUT(log, BidPrice4);
    FORMAT_OUT(log, BidVolume4);
    FORMAT_OUT(log, AskPrice4);
    FORMAT_OUT(log, AskVolume4);
    FORMAT_OUT(log, BidPrice5);
    FORMAT_OUT(log, BidVolume5);
    FORMAT_OUT(log, AskPrice5);
    FORMAT_OUT(log, AskVolume5);
    FORMAT_OUT(log, InstrumentID);
    FORMAT_OUT(log, UpdateTime);
    FORMAT_OUT(log, UpdateMillisec);
}

#endif /* DEMOUTILS_H_ */
