#ifndef MS58_REGS_H
#define MS58_REGS_H

// MS5837 Sensor Definitions
#define MS5837_ADDR               0x76  // I2C address of the MS5837 sensor
#define MS5837_RESET              0x1E  // Reset command
#define MS5837_PROM_READ_BASE     0xA0  // Base command for reading calibration data
#define MS5837_ADC_READ           0x00  // ADC read command
#define MS5837_CONVERT_D1_OSR_256 0x40  // D1 pressure conversion command (OSR=256)
#define MS5837_CONVERT_D2_OSR_256 0x50  // D2 temperature conversion command (OSR=256)
//The following are commands for getting D1 and D2 at different oversampling ratios (OCR).
//https://www.mouser.com/datasheet/2/418/5/NG_DS_MS5837-30BA_B1-1130109.pdf
//There are different computational times (delays) required for the different OCR Values
//which can be found in the datasheet. I am using the minimum value, for which dT=0.6ms,
//but the minimum delay in the code that is required is 2ms for some reason.
#define MS5837_CONVERT_D1_256 	0x40//I2C Command: Request D1 Conversion @ OCR=256 (min)
#define MS5837_CONVERT_D2_256 	0x50//I2C Command: Request D2 Conversion @ OCR=256 (min)
#define MS5837_CONVERT_D1_512 	0x42
#define MS5837_CONVERT_D2_512 	0x52
#define MS5837_CONVERT_D1_1024 	0x44
#define MS5837_CONVERT_D2_1024 	0x54
#define MS5837_CONVERT_D1_2048 	0x46
#define MS5837_CONVERT_D2_2048 	0x56
#define MS5837_CONVERT_D1_4096 	0x48
#define MS5837_CONVERT_D2_4096 	0x58
#define MS5837_CONVERT_D1_8192 	0x4A//I2C Command: Request D1 Conversion @ OCR=8192 (max)
#define MS5837_CONVERT_D2_8192 	0x5A//I2C Command: Request D2 Conversion @ OCR=8192 (max)

#define TCA9548_ADDR				0x74

#endif