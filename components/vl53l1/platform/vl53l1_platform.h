/**
 * @file  vl53l1_platform.h
 * @brief Those platform functions are platform dependent and have to be implemented by the user
 */

#ifndef COMPONENTS_VL53L1_PLATFORM_VL53L1_PLATFORM
#define COMPONENTS_VL53L1_PLATFORM_VL53L1_PLATFORM

#include "vl53l1_types.h"
#include "i2cdev.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct
	{
		uint32_t dummy;
	} VL53L1_Dev_t;

	typedef VL53L1_Dev_t *VL53L1_DEV;

	/** @brief VL53L1_WriteMulti() definition.\n
 * To be implemented by the developer
 */
	int8_t VL53L1_WriteMulti(
		i2c_dev_t *dev,
		uint16_t index,
		uint8_t *pdata,
		uint32_t count);
	/** @brief VL53L1_ReadMulti() definition.\n
 * To be implemented by the developer
 */
	int8_t VL53L1_ReadMulti(
		i2c_dev_t *dev,
		uint16_t index,
		uint8_t *pdata,
		uint32_t count);
	/** @brief VL53L1_WrByte() definition.\n
 * To be implemented by the developer
 */
	int8_t VL53L1_WrByte(
		i2c_dev_t *dev,
		uint16_t index,
		uint8_t data);
	/** @brief VL53L1_WrWord() definition.\n
 * To be implemented by the developer
 */
	int8_t VL53L1_WrWord(
		i2c_dev_t *dev,
		uint16_t index,
		uint16_t data);
	/** @brief VL53L1_WrDWord() definition.\n
 * To be implemented by the developer
 */
	int8_t VL53L1_WrDWord(
		i2c_dev_t *dev,
		uint16_t index,
		uint32_t data);
	/** @brief VL53L1_RdByte() definition.\n
 * To be implemented by the developer
 */
	int8_t VL53L1_RdByte(
		i2c_dev_t *dev,
		uint16_t index,
		uint8_t *pdata);
	/** @brief VL53L1_RdWord() definition.\n
 * To be implemented by the developer
 */
	int8_t VL53L1_RdWord(
		i2c_dev_t *dev,
		uint16_t index,
		uint16_t *pdata);
	/** @brief VL53L1_RdDWord() definition.\n
 * To be implemented by the developer
 */
	int8_t VL53L1_RdDWord(
		i2c_dev_t *dev,
		uint16_t index,
		uint32_t *pdata);
	/** @brief VL53L1_WaitMs() definition.\n
 * To be implemented by the developer
 */
	int8_t VL53L1_WaitMs(
		i2c_dev_t *dev,
		int32_t wait_ms);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_VL53L1_PLATFORM_VL53L1_PLATFORM */
