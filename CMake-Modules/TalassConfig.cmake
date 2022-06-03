#ifndef TALASSCONFIG_H
#define TALASSCONFIG_H

#include <stdint.h>

#include <cassert>
#include <cstdio>

typedef @FUNCTION_TYPE@ FunctionType;

typedef @GLOBAL_INDEX_TYPE@ GlobalIndexType;
static const GlobalIndexType GNULL = (GlobalIndexType)-1;
typedef @SIGNED_GLOBAL_INDEX_TYPE@ SignedGlobalIndexType;

typedef @LOCAL_INDEX_TYPE@ LocalIndexType;
static const LocalIndexType LNULL = (LocalIndexType)-1;
typedef @SIGNED_LOCAL_INDEX_TYPE@ SignedLocalIndexType;

#if defined(WIN32) || defined(WIN64)

#define stwarning(msg,...) {;}
#define sterror(condition,msg,...) {;}
#define stmessage(condition,msg,...) {;}

#else

#ifndef NDEBUG

#include <cstdio>
#include <cstring>

#define stwarning(msg,...) {char error[200] = "WARNING: %s::%u:\n\t";strcat(error,msg);strcat(error,"\n");fprintf(stderr,error,__FILE__,__LINE__ , ## __VA_ARGS__);}
#define sterror(condition,msg,...) {if ((condition)) { char error[200] = "ERROR: %s::%u:\n\t";strcat(error,msg);strcat(error,"\n");fprintf(stderr,error,__FILE__,__LINE__ , ## __VA_ARGS__);assert(false);}}
#define stmessage(condition,msg,...)  {if ((condition)) { char error[200] = "WARNING: %s::%u:\n";strcat(error,msg);strcat(error,"\n");fprintf(stderr,error,__FILE__,__LINE__ , ## __VA_ARGS__);}}

#else

#define stwarning(msg,...) {;}
#define sterror(condition,msg,...) {;}
#define stmessage(condition,msg,...)  {;}

#endif

#endif



#endif

