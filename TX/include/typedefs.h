#ifndef __TYPEDEFS_H
#define __TYPEDEFS_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef bool      BOOL;

#ifndef TRUE
#define TRUE  true
#endif
#ifndef FALSE
#define FALSE false
#endif

#define INFINITE  0xFFFFFFFFUL

#endif