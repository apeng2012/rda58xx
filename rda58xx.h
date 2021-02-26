/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-02-25     peng       the first version
 */
#ifndef PKG_RDA58XX_H__
#define PKG_RDA58XX_H__

#ifdef __cplusplus
extern "C" {
#endif

#define RDA58XX_I2C_ADDR (0x11)

struct rda58xx_device
{
    struct rt_i2c_bus_device *i2c;
    rt_mutex_t lock;
};
typedef struct rda58xx_device *rda58xx_device_t;

rda58xx_device_t rda58xx_init(const char *i2c_bus_name);
void rda58xx_deinit(rda58xx_device_t dev);
rt_err_t rda58xx_set_freq(rda58xx_device_t dev, uint32_t freq);
rt_err_t rda58xx_write_reg(rda58xx_device_t dev, uint8_t reg_addr, uint16_t val);
rt_err_t rda58xx_read_reg(rda58xx_device_t dev, uint8_t regaddr, uint16_t *pval);

#ifdef __cplusplus
}
#endif

#endif  // PKG_RDA58XX_H__
