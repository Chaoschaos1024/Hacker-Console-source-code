<!-- Chinese -->
<!--
 * @Author: 无序熵增
 * @Date: 2025-12-11 22:22:40
 * @LastEditors: 无序熵增
 * @LastEditTime: 2025-12-18 19:00:38
 * @Description: 
 * 
 * Copyright (c) 2025 by 无序熵增, All Rights Reserved. 
-->
<div align="center">

🌍 **Language** / **语言选择**
  
[![中文](https://img.shields.io/badge/中文-red?style=for-the-badge)](readme.md)
[![English](https://img.shields.io/badge/English-blue?style=for-the-badge)](readme.en.md)

</div>

---
# Hacker Console 固件源码

## 📋 项目介绍
**Hacker Console** 是一款由本人设计的手持终端，采用**树莓派 B**系列作为主板。  
如果你对我的设计感兴趣，请在 **Bilibili** 上搜索 **“Hacker Console”**。

<div align="center">
<img src="images/front.jpg" alt="Hacker Console 正面" width="45%">
<img src="images/right.jpg" alt="Hacker Console 右侧" width="45%">
</div>

作为一款拓展性极强的手持终端，它具有以下特性：

- 🖥️ **免驱动的 HDMI 屏幕**，支持触摸功能
- ⌨️ **HID 协议键盘**，通过 USB 通信，即插即用
- 🔧 **独立 PCB 设计**，键盘可自由替换
- 📊 **状态显示副屏**，实时监控设备状态
- 🔌 **GPIO 引脚引出**，方便外设扩展
- 🏗️ **外壳设计开源**，支持个性化定制

## 🔧 固件说明
这里提供主控板上运行的程序，包含两个单片机的程序。

### ⌨️ 键盘模块程序
由于有新、旧两个键盘版本，请根据您手中的键盘版本选择对应的程序。  
（一般情况下，到手的键盘均已烧写好固件。）

| 键盘版本 | 对应程序目录 |
|----------|--------------|
| **新键盘** | `newkeyboard3` 文件夹 |
| **旧键盘** | `keyboard2` 文件夹 |

### 🖥️ 主板程序
主控板程序位于 `mainboard5` 文件夹中。

## 🛠️ 编译指南

### 📦 环境要求
- 使用 **Arduino IDE**
- 安装 **RP2040 编译包**
- **务必下载 4.4 版本** 的RP2040编译包

<div align="center">
<img src="images/Software_package_version.png" alt="RP2040编译包版本" width="80%">
<p><em>注意：必须选择 4.4 版本的编译包</em></p>
</div>

### ⚙️ 编译设置
1. **Boot Stage 2** → 选择 **Generic SPI/4**
2. **USB Stack** → 设置为 **"PICO SDK"**

<div align="center">
<img src="images/Compilation_Settings.png" alt="编译设置界面" width="80%">
<p><em>正确的编译设置界面</em></p>
</div>

### 📝 编译步骤
1. 打开 Arduino IDE 并载入项目
2. 使用快捷键 **`Ctrl + Alt + S`** 进行编译
3. 固件将自动导出至项目文件夹
4. 编译结果是一个 **`.uf2`** 格式的文件

<div align="center">
<table>
<tr>
<td align="center"><img src="images/Compilation.png" alt="编译过程" width="90%"><br>编译过程界面</td>
<td align="center"><img src="images/Compilation1.png" alt="编译结果" width="90%"><br>生成的UF2文件</td>
</tr>
</table>
</div>

## 🔄 烧录指南

### ⌨️ 键盘固件烧录
> **注意**：键盘程序需要**使用烧录器单独烧录**
>
> 如果您有能力修改代码，推荐使用 **picoprobe** 来烧录固件
> 当然，您也可以使用我的另外一个开源项目：**rp2040-rp2350-debugger**，这是一个专为树莓派官方IC设计的烧录器，兼容pickit外壳
> 地址在这：https://github.com/Chaoschaos1024/rp2040-rp2350-debugger
<div align="center">
<table>
<tr>
<td align="center"><img src="images/debuggerfront.jpg" alt="正面图" width="90%"><br>烧录器正面</td>
<td align="center"><img src="images/debuggerback.jpg" alt="反面图" width="90%"><br>烧录器反面</td>
</tr>
</table>
</div>

### 🖥️ 主板固件升级
#### 操作步骤：
1. 找到设备左侧的两个小孔（每个孔内有一个按键）
2. **按住上面的按键**不放
3. **按一下下面的按键**
4. 等待系统识别出U盘设备

<div align="center">
<img src="images/to_flash_mode.jpg" alt="进入烧录模式操作" width="60%">
<p><em>设备左侧的烧录按钮位置</em></p>
</div>

#### 固件更新：
5. 树莓派系统将识别出一个 **U盘**
6. 将编译好的 **`.uf2`** 固件文件拖入该U盘中
7. 设备将自动完成固件升级

<div align="center">
<img src="images/update.jpg" alt="固件更新界面" width="80%">
<p><em>将UF2文件拖入U盘完成升级</em></p>
</div>

## 🤝 贡献与反馈
**欢迎您：**
- ✏️ 对代码进行修改并提交Pull Request
- 💡 提供新的想法与改进建议
- 👥 加入交流群：**867028792** 与我取得联系

感谢您的关注与支持！

## 📄 许可证
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

本项目采用 **MIT 许可证** - 查看 [LICENSE](LICENSE) 文件了解更多细节。

---

<div align="center">
<sub>Made with ❤️ for the Hacker Console community</sub>
</div>