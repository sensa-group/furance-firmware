#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdint.h>

/*
 * Uncomment to enable debugging
 * Comment to disable debugging
 */
#define DEBUG_ENABLED       1

/*
 * Uncomment to enable hardware debugging
 * Comment to disable hardware debugging
 */
#define DEBUG_HARDWARE      1

/*
 * Uncomment to enable software debugging
 * Comment to disable software debugging
 */
//#define DEBUG_SOFTWARE      1

/*
 * Buffer size for printf function
 */
#define DEBUG_BUFFER_SIZE       255

void DEBUG_init(void);
void DEBUG_logString(const char *str);
void DEBUG_logByte(uint8_t data);
void DEBUG_logInteger(long long data);
void DEBUG_printf(const char *str, ...);

#endif // _DEBUG_H_
