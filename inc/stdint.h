/**
 * @file stdint.h
 * @brief Standard integer types for ARM embedded systems
 * 
 * This is a minimal stdint.h implementation for embedded systems
 * when newlib headers are not available.
 */

#ifndef _STDINT_H
#define _STDINT_H

/* Exact-width integer types */
typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef signed short        int16_t;
typedef unsigned short      uint16_t;
typedef signed int          int32_t;
typedef unsigned int        uint32_t;
typedef signed long long    int64_t;
typedef unsigned long long  uint64_t;

/* Minimum-width integer types */
typedef signed char         int_least8_t;
typedef unsigned char       uint_least8_t;
typedef signed short        int_least16_t;
typedef unsigned short      uint_least16_t;
typedef signed int          int_least32_t;
typedef unsigned int        uint_least32_t;
typedef signed long long    int_least64_t;
typedef unsigned long long  uint_least64_t;

/* Fastest minimum-width integer types */
typedef int32_t   int_fast8_t;
typedef uint32_t  uint_fast8_t;
typedef int32_t   int_fast16_t;
typedef uint32_t  uint_fast16_t;
typedef int32_t   int_fast32_t;
typedef uint32_t  uint_fast32_t;
typedef int64_t   int_fast64_t;
typedef uint64_t  uint_fast64_t;

/* Integer types capable of holding object pointers */
typedef int32_t   intptr_t;
typedef uint32_t  uintptr_t;

/* Greatest-width integer types */
typedef int64_t   intmax_t;
typedef uint64_t  uintmax_t;

/* Limits of exact-width integer types */
#define INT8_MIN          (-128)
#define INT8_MAX          127
#define UINT8_MAX         255

#define INT16_MIN         (-32768)
#define INT16_MAX         32767
#define UINT16_MAX        65535

#define INT32_MIN         (-2147483648L)
#define INT32_MAX         2147483647L
#define UINT32_MAX        4294967295UL

#define INT64_MIN         (-9223372036854775808LL)
#define INT64_MAX         9223372036854775807LL
#define UINT64_MAX        18446744073709551615ULL

/* Limits of minimum-width integer types */
#define INT_LEAST8_MIN    INT8_MIN
#define INT_LEAST8_MAX    INT8_MAX
#define UINT_LEAST8_MAX   UINT8_MAX

#define INT_LEAST16_MIN   INT16_MIN
#define INT_LEAST16_MAX   INT16_MAX
#define UINT_LEAST16_MAX  UINT16_MAX

#define INT_LEAST32_MIN   INT32_MIN
#define INT_LEAST32_MAX   INT32_MAX
#define UINT_LEAST32_MAX  UINT32_MAX

#define INT_LEAST64_MIN   INT64_MIN
#define INT_LEAST64_MAX   INT64_MAX
#define UINT_LEAST64_MAX  UINT64_MAX

/* Limits of fastest minimum-width integer types */
#define INT_FAST8_MIN     INT32_MIN
#define INT_FAST8_MAX     INT32_MAX
#define UINT_FAST8_MAX    UINT32_MAX

#define INT_FAST16_MIN    INT32_MIN
#define INT_FAST16_MAX    INT32_MAX
#define UINT_FAST16_MAX   UINT32_MAX

#define INT_FAST32_MIN    INT32_MIN
#define INT_FAST32_MAX    INT32_MAX
#define UINT_FAST32_MAX   UINT32_MAX

#define INT_FAST64_MIN    INT64_MIN
#define INT_FAST64_MAX    INT64_MAX
#define UINT_FAST64_MAX   UINT64_MAX

/* Limits of integer types capable of holding object pointers */
#define INTPTR_MIN        INT32_MIN
#define INTPTR_MAX        INT32_MAX
#define UINTPTR_MAX       UINT32_MAX

/* Limits of greatest-width integer types */
#define INTMAX_MIN        INT64_MIN
#define INTMAX_MAX        INT64_MAX
#define UINTMAX_MAX       UINT64_MAX

/* Macros for minimum-width integer constants */
#define INT8_C(x)         ((int8_t)(x))
#define UINT8_C(x)        ((uint8_t)(x))
#define INT16_C(x)        ((int16_t)(x))
#define UINT16_C(x)       ((uint16_t)(x))
#define INT32_C(x)        ((int32_t)(x ## L))
#define UINT32_C(x)       ((uint32_t)(x ## UL))
#define INT64_C(x)        ((int64_t)(x ## LL))
#define UINT64_C(x)       ((uint64_t)(x ## ULL))

/* Macros for greatest-width integer constants */
#define INTMAX_C(x)       INT64_C(x)
#define UINTMAX_C(x)      UINT64_C(x)

/* Pointer-sized integer types */
#define PTRDIFF_MIN       INT32_MIN
#define PTRDIFF_MAX       INT32_MAX
#define SIZE_MAX          UINT32_MAX

#endif /* _STDINT_H */

