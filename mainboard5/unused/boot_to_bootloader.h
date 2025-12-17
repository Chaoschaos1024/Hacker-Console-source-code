#pragma once
#include "pico/bootrom.h"
#include <Arduino.h>

class BootToBootloader
{
public:
    /**
     * 构造函数
     * @param trigger_char 接收到该字符触发跳转 bootloader，默认 'B'
     */
    BootToBootloader(char trigger_char = 'B')
        : trigger(trigger_char)
    {
    }

    /**
     * 初始化 USB 串口
     */
    void begin()
    {
        Serial.begin(115200);
        while (!Serial)
        {
            // 等待 USB 串口就绪
            delay(10);
        }
        Serial.println("BootToBootloader ready. Send trigger char to enter bootloader.");
    }

    /**
     * 在 loop 中调用，检查串口
     */
    void check()
    {
        while (Serial.available())
        {
            char c = Serial.read();
            if (c == trigger)
            {
                Serial.println("Entering bootloader...");
                Serial.flush();       // 确保消息发送完
                delay(10);            // 小延迟让消息发送出去
                reset_usb_boot(0, 0); // 跳转到 USB bootloader
            }
        }
    }

private:
    char trigger;
};