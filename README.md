# RDA58xx

## 1、介绍

这是一个 RT-Thread 的软件包，用于驱动收音机芯片RDA5820。

### 1.1 许可证

rda58xx 遵循 MIT 许可。

### 1.3 依赖

- RT-Thread 3.0+
- I2C 设备驱动程序

## 2、如何打开 rda58xx

使用 rda58xx package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages
    peripheral libraries and drivers --->
        [*] RDA58xx single-chip broadcase FM transceiver driver
```

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3、使用 rda58xx

请参考 rda58xx.c 中的 msh 命令 rda58xx 部分代码。

## 4、注意事项

- rda5820分24pin和20pin，目前只在24pin芯片测试过。
- rda5820控制端口采用I2c，SPI接口没有实现。
- rda5820有多种功能，目前只实现 FM 接收。

## 5、联系方式 & 感谢

* 维护：apeng2012
* 主页：https://github.com/apeng2012/rda58xx
