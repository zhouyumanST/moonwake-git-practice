# Red Buoy Horizontal Offset

这是一个使用 C++17 和 OpenCV 编写的单图像命令行程序。程序从图片中提取红色区域，选择面积最大的候选目标，计算目标相对于画面中心的水平像素偏移，并输出目标位于画面左侧、中央还是右侧。

本程序用于说明一条最小的视觉处理链路，不是完整的无人船感知、定位或控制模块。它不会输出偏航角、航向指令或舵机控制量，也不能代表真实水面环境中的检测性能。

## 主要处理流程

1. 使用 `cv::imread` 读取一张 BGR 图像；
2. 将图像转换到 HSV 色彩空间；
3. 分别提取低色相段和高色相段的红色区域，并合并二值掩膜；
4. 使用形态学开运算去除部分小面积噪声；
5. 查找外部轮廓，选择面积最大的候选目标并计算外接矩形；
6. 根据外接矩形中心与图像中心的差值，输出水平偏移和相对位置。

目前，图像读取、红色区域提取、轮廓筛选和结果输出已经实现；`BuoyCenterX`、`HorizontalOffsetPx` 与 `RelativePosition` 三个几何计算函数仍保留为待完成实现。

## 输入与输出

程序接收一个图像路径：

```text
buoy_offset <image_path>
```

检测到目标时，程序以 `key=value` 形式输出：

```text
status=FOUND
image_width=<图像宽度>
buoy_x=<外接矩形左上角横坐标>
buoy_width=<外接矩形宽度>
buoy_center_x=<目标中心横坐标>
offset_px=<目标中心相对图像中心的水平偏移>
position=<LEFT|CENTER|RIGHT>
```

没有检测到满足条件的红色目标时输出：

```text
status=NOT_FOUND
```

图像坐标从左向右增大，因此 `offset_px < 0` 表示目标在画面左侧，`offset_px > 0` 表示目标在画面右侧。程序当前使用 40 像素作为中央容差，绝对偏移不超过该值时输出 `CENTER`。

## 主要目录结构

```text
moonwake-git-practice/
├── .github/
│   └── pull_request_template.md
├── config/
│   └── camera.example.yaml
├── docs/
│   └── review-checklist.md
├── samples/
│   ├── README.md
│   └── *.jpg
├── src/
│   └── buoy_offset.cpp
├── tests/
│   ├── check_detection.py
│   └── check_solution.py
├── .gitattributes
├── .gitignore
├── CONTRIBUTING.md
└── README.md
```

`config/camera.example.yaml` 是后续相机输入接口的配置示例，当前单图像程序不会读取该文件。

## 依赖

- Ubuntu 22.04；
- 支持 C++17 的编译器；
- OpenCV 4 C++ 开发库；
- Python 3，仅用于运行仓库中的检查脚本。

## 构建

请在确认命令能够在目标 Ubuntu 环境中重复执行后，补充以下信息：

编译器及版本：GCC 11.4.0
OpenCV 版本：4.5.4
# 在此填写经过验证的构建命令
```bash  
g++ -std=c++17 src/buoy_offset.cpp -o buoy_offset $(pkg-config --cflags --libs opencv4)
```

## 运行


# 在此填写程序运行命令，并至少保留一个可重复的样例输入
```bash
  ./buoy_offset samples/buoy_left.jpg
```
#output
```
status=FOUND
image_width=1000
buoy_x=116
buoy_width=289
buoy_center_x=260
offset_px=-240
position=LEFT
```
## 检查

`tests/check_detection.py` 检查图像读取和红色目标检测；`tests/check_solution.py` 检查目标中心、水平偏移和相对位置计算。请在下方记录经过验证的检查命令与实际结果。


# 在此填写检查命令
```bash  
python3 tests/check_solution.py ./buoy_offset
python3 tests/check_detection.py ./buoy_offset
```

```text
测试环境：Ubuntu 22.04、GCC 11.4.0、OpenCV 4.5.4
实际结果：Geometry and position logic: 10 samples passed​
 Provided image loading and buoy detection: 10 samples passed
```

## 样本与适用范围

`samples/` 中的图片来自同一张浮漂照片的不同裁剪，用于覆盖目标位于左侧、中央、右侧、容差边界、画面边缘以及不存在目标等情况。详细说明见 [samples/README.md](samples/README.md)。

当前检测器使用固定 HSV 阈值和最大轮廓规则，容易受到光照、水面反光、其他红色物体和目标尺度变化影响。它只适用于仓库中的固定样本，不应直接作为真实航行中的浮漂检测方案。

## 参与开发

仓库的分支、Commit、Pull Request 和 Review 约定见 [CONTRIBUTING.md](CONTRIBUTING.md)。审查代码时可参考 [Pull Request Review Checklist](docs/review-checklist.md)。
