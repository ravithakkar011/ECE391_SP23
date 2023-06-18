/* types.h - Defines to use the familiar explicitly-sized types in this 
 * OS (uint32_t, int8_t, etc.).  This is necessary because we don't want
 * to include <stdint.h> when building this OS
 * vim:ts=4 noexpandtab
 */

#ifndef _TYPES_H
#define _TYPES_H

#define NULL 0

#ifndef ASM

#define BITS_32         32
#define BYTES_24B       24
#define BYTES_32B       32
#define BYTES_52B       52
#define BYTES_64B       64
#define BYTES_1KB       1024
#define BYTES_4KB       4096

/* Types defined here just like in <stdint.h> */
typedef int int32_t;
typedef unsigned int uint32_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef char int8_t;
typedef unsigned char uint8_t;

#endif /* ASM */

#endif /* _TYPES_H */
