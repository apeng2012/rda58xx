/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-02-25     peng       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include <stdlib.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME "rda58xx"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

#include "rda58xx.h"

#ifdef PKG_USING_RDA58XX
/**
 * This function write a rda58xx register
 *
 * @param dev the name of rda58xx device
 * @param reg_addr register address
 * @param val The value to be written
 *
 * @return the operation status, RT_EOK on OK; RT_ERROR on error
 */
rt_err_t rda58xx_write_reg(rda58xx_device_t dev, uint8_t reg_addr, uint16_t val)
{
    rt_err_t result;
    uint8_t buf[3];
    RT_ASSERT(dev);

    buf[0] = reg_addr;
    buf[1] = (uint8_t)(val >> 8);
    buf[2] = (uint8_t)val;
    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        if (rt_i2c_master_send(dev->i2c, RDA58XX_I2C_ADDR, 0, buf, 3) == 3)
        {
            result = RT_EOK;
        }
        else
        {
            result = RT_ERROR;
        }
    }
    else
    {
        LOG_W("The rda58xx could not respond  at this time. Please try again");
    }
    rt_mutex_release(dev->lock);

    return result;
}

/**
 * This function read a rda58xx register
 *
 * @param dev the name of rda58xx device
 * @param reg_addr register address
 * @param pval The pointer to return register value
 *
 * @return the operation status, RT_EOK on OK; RT_ERROR on error
 */
rt_err_t rda58xx_read_reg(rda58xx_device_t dev, uint8_t regaddr, uint16_t *pval)
{
    rt_err_t result;
    uint8_t buf[2];
    struct rt_i2c_msg msgs[] =
    {
        {
            .addr = RDA58XX_I2C_ADDR,
            .flags = RT_I2C_WR,
            .buf = &regaddr,
            .len = 1,
        },
        {
            .addr = RDA58XX_I2C_ADDR,
            .flags = RT_I2C_RD,
            .buf = &buf[0],
            .len = 2,
        },
    };
    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result == RT_EOK)
    {
        if (rt_i2c_transfer(dev->i2c, msgs, 2) == 2)
        {
            *pval = (((uint16_t)buf[0]) << 8) + buf[1];
            result = RT_EOK;
        }
        else
        {
            result = RT_ERROR;
        }
    }
    else
    {
        LOG_W("The rda58xx could not respond  at this time. Please try again");
    }
    rt_mutex_release(dev->lock);

    return result;
}

/**
 * This function set transmit frequency
 *
 * @param dev the name of rda58xx device
 * @param freq kHz
 *
 * @return the operation status, RT_EOK on OK; RT_ERROR on error
 */
rt_err_t rda58xx_set_freq(rda58xx_device_t dev, uint32_t freq)
{
    uint16_t tmp;
    uint16_t chan;
    uint32_t space = 100;

    rda58xx_read_reg(dev, 0x03, &tmp);

    switch (tmp & 0x0003)
    {
    case 0x0000:
        space = 100;
        break;
    case 0x0001:
        space = 200;
        break;
    case 0x0002:
        space = 50;
        break;
    default:
        LOG_E("register 03H SPACE[1:0] 11 not included");
        return RT_ERROR;
    }

    switch (tmp & 0x000C)
    {
    case 0x0000:
        if ((freq > 108000) || (freq < 87000))
        {
            LOG_E("freq out or range BAND0(87MHz ~ 108MHz)");
            return RT_ERROR;
        }
        chan = (freq - 87000) / space;
        break;
    case 0x0004:
        if ((freq > 91000) || (freq < 76000))
        {
            LOG_E("freq out or range BAND0(76MHz ~ 91MHz)");
            return RT_ERROR;
        }
        chan = (freq - 76000) / space;
        break;
    default:
        LOG_E("register 03H BAND[1:0] only included 00 and 01");
        return RT_ERROR;
    }

    tmp &= 0x000F;
    tmp |= 0x0010;  // enable turn
    tmp |= chan << 6;
    return rda58xx_write_reg(dev, 0x03, tmp);
}

/**
 * This function initializes rda58xx registered device driver
 *
 * @param i2c_bus_name the name of i2c bus
 *
 * @return the rda58cxx device.
 */
rda58xx_device_t rda58xx_init(const char *i2c_bus_name)
{
    rda58xx_device_t dev;

    RT_ASSERT(i2c_bus_name);

    dev = rt_calloc(1, sizeof(struct rda58xx_device));
    if (dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for rda58xx device on '%s' ", i2c_bus_name);
        return RT_NULL;
    }

    dev->i2c = rt_i2c_bus_device_find(i2c_bus_name);
    if (dev->i2c == RT_NULL)
    {
        LOG_E("Can't find rda58xx device on '%s' ", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    dev->lock = rt_mutex_create("mutex_rda58xx", RT_IPC_FLAG_FIFO);
    if (dev->lock == RT_NULL)
    {
        LOG_E("Can't create mutex for rda58xx device on '%s' ", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    rda58xx_write_reg(dev, 0x02, 0x0002);  // soft reset
    rt_thread_mdelay(50);
    rda58xx_write_reg(dev, 0x02, 0xC001);  // power up
    rt_thread_mdelay(600);
    rda58xx_write_reg(dev, 0x03, 0x0000);
    rda58xx_write_reg(dev, 0x05, 0x8848);
    rda58xx_write_reg(dev, 0x16, 0xC000);

    return dev;
}

/**
 * This function releases memory and deletes mutex lock
 *
 * @param dev the pointer of device driver structure
 */
void rda58xx_deinit(rda58xx_device_t dev)
{
    RT_ASSERT(dev);

    rt_mutex_delete(dev->lock);

    rt_free(dev);
}

void rda58xx(int argc, char *argv[])
{
    static rda58xx_device_t dev = RT_NULL;

    if (argc > 1)
    {
        if (!strcmp(argv[1], "probe"))
        {
            if (argc > 2)
            {
                /* initialize the device when first probe */
                if (!dev || strcmp(dev->i2c->parent.parent.name, argv[2]))
                {
                    /* deinit the old device */
                    if (dev)
                    {
                        rda58xx_deinit(dev);
                    }
                    dev = rda58xx_init(argv[2]);
                }
            }
            else
            {
                rt_kprintf("Please using 'rda58xx probe <i2c_bus_name>' first\n");
            }
        }
        else if (!strcmp(argv[1], "freq"))
        {
            if (dev)
            {
                if (argc > 2)
                {
                    rda58xx_set_freq(dev, atoi(argv[2]));
                }
            }
            else
            {
                rt_kprintf("Please using 'rda58xx probe <i2c_bus_name>' first\n");
            }
        }
        else
        {
            rt_kprintf("Unknown command. Please enter 'rda58xx' for help\n");
        }
    }
    else
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("rda58xx probe <i2c_bus_name>     - init device\n");
        rt_kprintf("rda58xx freq <FM_frequency_kHz>  - turn FM freq\n");

    }
}
MSH_CMD_EXPORT(rda58xx, rda58xx broadcast FM transceiver function);
#endif  // PKG_USING_RDA58XX
