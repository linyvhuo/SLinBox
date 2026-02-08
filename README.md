# SLinBox 智能工具箱

## 项目简介

SLinBox是一款基于Qt5.14.2开发的智能工具箱，集成了多种实用工具，包括企业微信自动化、截图OCR、串口调试、进制转换、CRC校验等功能。

## 编译环境要求

- **操作系统**: Windows 7/10/11 (32/64位)
- **编译器**: Microsoft Visual Studio 2015 (MSVC2015)
- **Qt版本**: Qt 5.14.2 (MSVC2015 64bit)
- **OpenCV**: 可选（用于企业微信自动化模块的图标识别）

## 编译步骤

### 方法1：使用Qt Creator（推荐）

1. 打开Qt Creator
2. 文件 → 打开文件或项目 → 选择 `SLinBox.pro`
3. 选择编译套件：Desktop Qt 5.14.2 MSVC2015 64bit
4. 点击"构建" → "重新构建项目" (Ctrl+Shift+B)
5. 编译完成后，可执行文件位于 `release\SLinBox.exe`

### 方法2：使用批处理脚本

1. 双击运行 `build.bat` 文件
2. 脚本会自动设置Visual Studio环境并编译项目
3. 编译完成后，可执行文件位于 `release\SLinBox.exe`

### 方法3：手动编译

打开命令提示符(cmd.exe)，执行以下命令：

```cmd
F:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat amd64
cd /d E:\QT_Project\SLinBox
F:\Qt\Qt5.14.2\5.14.2\msvc2015_64\bin\qmake.exe SLinBox.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
F:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/BIN/amd64/nmake.exe
```

## OpenCV配置（可选）

如果需要使用企业微信自动化模块的图标识别功能，需要安装OpenCV：

1. 下载OpenCV 4.5.4：https://opencv.org/releases/
2. 解压到 `C:\opencv` 目录
3. 重新运行编译脚本

**注意**：如果没有安装OpenCV，项目仍然可以编译，但企业微信自动化模块的图标识别功能将被禁用。

## 项目结构

```
SLinBox/
├── core/                   # 核心类
│   ├── configmanager.h/cpp  # 配置管理
│   ├── logger.h/cpp        # 日志系统
│   ├── windowhelper.h/cpp  # Windows API封装
│   └── screenhelper.h/cpp  # 屏幕辅助类
├── modules/                # 功能模块
│   ├── wechatauto/         # 企业微信自动化
│   ├── screenshotocr/      # 截图+OCR
│   ├── serialport/         # 串口调试
│   ├── baseconvert/        # 进制转换
│   ├── crccheck/           # CRC校验
│   ├── timingdiagram/      # 时序图
│   ├── delaycalc/          # 延时计算
│   ├── asciiquery/         # ASCII查询
│   ├── checksum/           # 校验和
│   └── datasheet/          # 数据手册
├── ui/                     # 用户界面
│   ├── mainwindow.h/cpp/ui # 主窗口
│   └── floatwindow.h/cpp/ui # 悬浮窗
├── resources/              # 资源文件
│   └── ui/themes/          # 主题文件
├── SLinBox.pro            # 项目文件
├── build.bat              # 编译脚本
└── README.md              # 说明文档
```

## 功能模块

### 1. 企业微信自动化
- 自动启动企业微信
- 图标识别（需要OpenCV）
- 问答循环
- 问题库管理（支持TXT/CSV导入）

### 2. 截图+OCR
- 全屏/区域截图
- OCR文字识别（调用umi-OCR）
- 标注功能（直线、矩形）
- 结果导出（图片/文本）

### 3. 串口调试
- 串口配置（波特率、数据位、停止位、校验位）
- 数据收发（ASCII/HEX）
- 日志记录

### 4. 进制转换
- 2/8/10/16进制转换
- 实时转换

### 5. CRC校验
- CRC-8/16/32校验
- 多种多项式支持

### 6. 时序图绘制
- 信号绘制
- 时间轴标注

### 7. 延时计算
- 正向计算（时间→指令数）
- 反向计算（指令数→时间）

### 8. ASCII查询
- 字符↔ASCII码转换
- 完整ASCII码表

### 9. 校验和
- 8/16位累加和
- 异或和

### 10. 数据手册查询
- 元器件数据手册查询
- 在线搜索

## 配置文件

配置文件位于：`%USERPROFILE%\Documents\SLinBox\config.ini`

## 日志文件

日志文件位于：`%USERPROFILE%\Documents\SLinBox\logs\`

## 注意事项

1. 首次运行会自动创建配置文件和日志目录
2. 企业微信自动化需要准备图标模板文件，放置在：`%USERPROFILE%\Documents\SLinBox\templates\`
3. OCR功能需要安装umi-OCR并配置到系统PATH
4. 串口调试需要管理员权限

## 许可证

本项目仅供学习和研究使用。

## 联系方式

如有问题或建议，请联系开发者。