# 遥感数据智能处理与分析系统

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.10+-green.svg)](https://cmake.org/)
[![OpenCV](https://img.shields.io/badge/OpenCV-4.13.0-red.svg)](https://opencv.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

## 📖 项目简介

本项目是一个基于C++的遥感数据智能处理与分析系统，支持多光谱卫星影像和三维点云数据的加载、处理、分析和导出。系统提供了丰富的遥感指数计算、点云滤波、数据管理、算法库等功能，并集成OpenCV实现高性能图像处理，使用CMake进行跨平台构建。

**主要特性**：
- 遥感指数计算（NDVI、NDWI、SAVI、EVI、亮温等）
- 点云体素滤波与统计离群值去除（SOR）
- 数据管理器模板类（增删查、排序、过滤、分组、合并）
- 数据导出（CSV、JSON格式）
- K‑means聚类、信息熵、直方图等遥感算法
- 基于OpenCV的图像增强（高斯模糊、中值滤波、归一化、重采样）
- CMake构建 + Git版本管理 + GitHub远程备份

---

## 📁 目录结构

```
2025302131011_LiuYizheng_RsProject/
├── src/                       # 源代码目录
│   ├── Algorithms.h           # 遥感算法库（NDVI、K‑means、卷积、熵等）
│   ├── DataExporter.h         # 数据导出类及友元函数（CSV/JSON）
│   ├── DataManager.h          # 数据管理器模板类
│   ├── DataObject.h           # 数据抽象基类
│   ├── DataObject.cpp         # 基类实现
│   ├── Pixel.h                # 像素模板类
│   ├── PointCloudData.h       # 点云数据类
│   ├── PointCloudData.cpp     # 点云实现（加载PLY、滤波、统计）
│   ├── SatelliteImage.h       # 卫星影像类
│   ├── SatelliteImage.cpp     # 影像实现（指数计算、OpenCV处理）
│   └── main.cpp               # 系统演示主程序
├── CMakeLists.txt             # CMake构建配置
├── README.md                  # 项目说明（本文件）
└── .gitignore                 # Git忽略规则
```

---

## 🛠️ 环境要求

- **操作系统**：Windows 10/11 （推荐使用MSYS2环境）
- **编译器**：GCC 11+ 或 Clang，支持 C++17 标准
- **构建工具**：CMake 3.10+
- **依赖库**：OpenCV 4.x （用于图像处理）
- **版本管理**：Git （可选，用于克隆仓库）

---

## 🔧 编译与运行

### 方式一：使用 MSYS2 + CMake

1. **安装 MSYS2 并配置环境**  
   从 [msys2.org](https://www.msys2.org/) 下载安装程序，运行后打开 **MSYS2 UCRT64** 终端。

2. **安装编译工具链、CMake 和 OpenCV**  
   ```bash
   pacman -S --needed mingw-w64-ucrt-x86_64-toolchain \
                    mingw-w64-ucrt-x86_64-cmake \
                    mingw-w64-ucrt-x86_64-gdb \
                    mingw-w64-ucrt-x86_64-opencv
   ```

3. **克隆仓库**  
   ```bash
   git clone git@github.com:Rainsky1104/2026_RScpp.git
   cd 2026_RScpp
   ```

4. **构建项目**  
   ```bash
   mkdir build && cd build
   cmake .. -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc
   cmake --build .
   ```

5. **运行程序**  
   ```bash
   ./RS_app.exe
   ```

> 如果 CMake 找不到 OpenCV，可手动指定：  
> `cmake .. -DOpenCV_DIR=/ucrt64/lib/cmake/opencv4`

### 方式二：使用 Visual Studio Code + CMake Tools

- 安装 VSCode 插件 **CMake Tools** 和 **C/C++**。
- 打开项目文件夹，按 `Ctrl+Shift+P` 运行 `CMake: Configure`，选择 **GCC for x86_64-w64-mingw32 (ucrt64)**。
- 点击底部状态栏的 **Build** 按钮（齿轮图标）或按 `F7` 编译。
- 编译成功后点击 **Run** 按钮（三角形图标）执行程序。

---

## 📦 功能演示

系统运行后将依次执行以下操作，并输出结果至终端及 `./output` 目录：

1. 从 `data/` 文件夹加载两张 JPG 影像（模拟为5波段）和一个 ASCII PLY 点云（约10万点）。
2. 使用 `DataManager` 管理影像和点云，展示统计、排序、过滤、合并功能。
3. 对影像进行高斯模糊，计算 NDVI、NDWI、SAVI 及亮温。
4. 对点云进行体素滤波（降采样至约300点）和 SOR 滤波（剔除离群点）。
5. 对点云高度值执行 K‑means 聚类（3类），计算信息熵和直方图。
6. 导出点云为 CSV 文件，批量导出影像统计为 JSON 文件。

详细输出示例参见 [运行结果](#运行结果) 部分。

---

## 🖥️ 运行结果

```
========== 遥感数据智能处理与分析系统 ==========

>>> 1. 数据加载
>>> 2. DataManager 创建与数据添加
影像管理器对象数: 2
点云管理器对象数: 2

>>> 3. 数据对象基本信息
Satellite Image: 樱顶1号影像 (IMG001) - 1920x1080x5 bands
PointCloudData: 点云1号 (PC001) - 106906 points, bounds: [-3.15,2.96]x[-0.045,1.05]x[-3.47,2.81]

>>> 4. 影像处理与遥感指数
高斯模糊耗时: 243.8 ms
平均 NDVI: 0.3107, NDWI: -0.2410, SAVI: 0.3201
平均亮温: 21.13°C, 范围 [21.13,21.13]°C

>>> 5. 点云处理
体素滤波后点数: 307, SOR滤波后点数: 274
平均高度: 0.212, 高度范围: 6.273

>>> 6. DataManager 排序、过滤与合并
按大小降序影像 ID: IMG001 IMG002
云量<10%影像数: 2, 合并后点云管理器对象数: 3

>>> 7. 遥感算法库应用
K‑means聚类完成, 高度信息熵: 16.58 bits
直方图: 3 51 417 406 15966 42007 38568 8661 738 89

>>> 8. 数据导出
点云 CSV 导出成功, 批量导出 2 个 JSON 文件
Total exports: 2
```

生成的导出文件位于 `./output/` 目录下。

---

## 🤝 贡献与许可

本项目为武汉大学遥感信息工程学院遥感应用软件设计课程大作业，遵守 MIT 开源协议。欢迎提交 Issue 和 Pull Request。
**GitHub**：[Rainsky1104](https://github.com/Rainsky1104)  
**仓库地址**：https://github.com/Rainsky1104/2026_RScpp

---

## 📧 联系方式

如有问题或建议，请通过 GitHub Issues 联系。

---
