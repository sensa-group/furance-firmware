/* 
 * File:   DS18B20.h
 * Author: zi
 *
 * Created on October 2, 2019, 3:30 AM
 */

#ifndef DS18B20_H
#define	DS18B20_H

#ifdef	__cplusplus
extern "C" {
#endif

// Model IDs
#define DS18S20MODEL 0x10  // also DS1820
#define DS18B20MODEL 0x28
#define DS1822MODEL  0x22
#define DS1825MODEL  0x3B
#define DS28EA00MODEL 0x42

void DS18B20O_init(void);
double DS18B20O_readT1(void);
double DS18B20O_readT2(void);


#ifdef	__cplusplus
}
#endif

#endif	/* DS18B20_H */
