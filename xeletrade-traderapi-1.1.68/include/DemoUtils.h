/*!
 * @file DemoUtils.h
 *
 * 
 * @author shuaiw
 */

#ifndef DEMOUTILS_H_
#define DEMOUTILS_H_

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

/*
 * 响应信息
 */
#define PRINT_RSP(RSPINFO)  fprintf(stdout, "\n{%s} Pass ErrorID:%d, ErrorMsg:%s nRequestID:%d Chain:%d\n\n", __FUNCTION__, \
	     RSPINFO->ErrorID, RSPINFO->ErrorMsg, nRequestID, bIsLast)

#define PRINT_RSP_ERR(RSPINFO)  fprintf(stdout, "\n{%s} Fail ErrorID:%d, ErrorMsg:%s nRequestID:%d Chain:%d\n\n", __FUNCTION__, \
	     RSPINFO->ErrorID, RSPINFO->ErrorMsg, nRequestID, bIsLast)

#endif /* DEMOUTILS_H_ */
