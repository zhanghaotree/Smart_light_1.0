﻿# **基于ESP8266的智能灯-程序说明书**  
## 1. 硬件基础  
本程序基于安信可的ESP8266-12F模组，在单板上集成了电源模块、LED恒流驱动模块、继电器、光敏电阻等。可以实现色温和亮度的调节，支持1600万色彩灯光效。全部功能实现远程控制。
## 2. 服务器  
使用了EMQ服务器，负责消息路由转发等。自建Tomcat+Apache+MySQL服务器，实现Web端控制。
## 3. 通信协议
|属性	|参数	|权限	|说明|
|------|-------|-------|----|
|PWM1	|A0	|R/W	|白光LED亮度值，整形数，0~255|
|PWM2	|A1	|R/W	|暖白LED亮度值，整形数，0~255|
|光敏电阻读数	|A2	|R	|整形数，0~1024，光敏电阻的电压值|
|SW1	|A3	|R/W	|数值，继电器1，0代表关，1代表开|
|SW2	|A4	|R/W	|数值，继电器2，0代表关，1代表开|
|RGB-R	|A5	|R/W	|RGB灯环R值，0~255|
|RGB-G	|A6	|R/W	|RGB灯环G值，0~255|
|RGB-B	|A7	|R/W	|RGB灯环B值，0~255|
|RGB-Model	|A8	|R/W	|RGB灯环模式，光效模式--自定义，0-255|
|Data	|AA	|R/W	|透传数据，最多128位|
|OTA	|U	|W	|OTA升级命令，参数为bin文件的url，字符串|
|是否上报	|D0（OD0/CD0）	|R/W	|D0的bit0代表A0是否主动上报，以此类推|
|上报间隔	|V0	|R/W	|表示主动上传的时间间隔，单位：秒|
## Z.版权信息
作者：张浩