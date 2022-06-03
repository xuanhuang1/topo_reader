/*
 * Definitions.h
 *
 *  Created on: Jan 30, 2012
 *      Author: bremer5
 */

#ifndef DISTRIBUTEDDEFINITIONS_H
#define DISTRIBUTEDDEFINITIONS_H


typedef float FunctionType;

typedef uint32_t GlobalIndexType;
static const GlobalIndexType GNULL = (GlobalIndexType)-1;

typedef uint32_t LocalIndexType;
static const LocalIndexType LNULL = (LocalIndexType)-1;


//! The index space to identify graphs
typedef uint32_t GraphID;

//! The rank space of MPI
typedef uint32_t RankID;



#ifndef NDEBUG

#include <cassert>

#define stwarning(msg,...) {char error[200] = "WARNING: %s::%u:\n\t";strcat(error,msg);strcat(error,"\n");fprintf(stderr,error,__FILE__,__LINE__ , ## __VA_ARGS__);}

#define sterror(condition,msg,...) {if ((condition)) { char error[200] = "ERROR: %s::%u:\n\t";strcat(error,msg);strcat(error,"\n");fprintf(stderr,error,__FILE__,__LINE__ , ## __VA_ARGS__);assert(false);}}

#define stmessage(condition,msg,...)  {if ((condition)) { char error[200] = "WARNING: %s::%u:\n";strcat(error,msg);strcat(error,"\n");fprintf(stderr,error,__FILE__,__LINE__ , ## __VA_ARGS__);}}

#else

#define stwarning(msg,...) {;}

#define sterror(condition,msg,...) {;}

#define stmessage(condition,msg,...)  {;}

#endif

#endif




