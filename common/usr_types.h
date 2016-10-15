/**----------------------------------------------------------------------------
 * Name:    usr_types.h
 * Purpose: user defined data types
 * Author:	JungJaeJoon on the www.kernel.bz
 *-----------------------------------------------------------------------------
 * Notes:
 *-----------------------------------------------------------------------------
 */


#ifndef USR_TYPES_H_INCLUDED
#define USR_TYPES_H_INCLUDED

//typedef char            int8_t;
typedef unsigned char   uint8_t;
typedef short int       int16_t;
typedef unsigned short  uint16_t;
typedef int             int32_t;
typedef unsigned int    uint32_t;

#define ARRAY_CNT(arr)  (sizeof(arr) / sizeof((arr)[0]))


#endif // USR_TYPES_H_INCLUDED
