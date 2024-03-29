# enhancer

[![macOS-Qt6](https://github.com/yuki-koyama/enhancer/actions/workflows/macos.yml/badge.svg)](https://github.com/yuki-koyama/enhancer/actions/workflows/macos.yml)
[![Ubuntu-Qt5](https://github.com/yuki-koyama/enhancer/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/yuki-koyama/enhancer/actions/workflows/ubuntu.yml)
![GitHub](https://img.shields.io/github/license/yuki-koyama/enhancer)

A C++/GLSL library for enhancing photographs (adjusting brightness, contrast, etc.).

This repository contains the following three features:
- __GLSL shaders__: Enhancement functionality as shaders for real-time enhancement applications.
- __C++ functions__: Enhancement functionality as C++ functions for display-less environments.
- __Qt Widget__: Utility Qt-based widget for easing the use of the GLSL shaders.

## Supported Parameters

### Default Set

- __Brightness__:
  - Apply a gamma curve in the `RGB` space
- __Contrast__:
  - Apply an S curve in the (gamma-corrected) `RGB` space
- __Saturation__:
  - Scale the saturation
- __Temperature__:
  - Add an offset toward the `-U+V` direction in the `Y'UV` space
- __Tint__:
  - Add an offset toward the `+U+V` direction in the `Y'UV` space

### Optional

- __Lift/Gamma/Gain__ (9D)
  - Reproduce common effects used in many color grading software packages
- __Color Balance__ (3D)
  - Adjust the `RGB` values while preserving the lightness

### Examples

Brightness
![Brightness](./docs/p0.jpg)

Contrast
![Contrast](./docs/p1.jpg)

Saturation
![Saturation](./docs/p2.jpg)

Temperature
![Temperature](./docs/p3.jpg)

Tint
![Tint](./docs/p4.jpg)

## Required Runtime Environments

- GLSL 3.3
- OpenGL 3.2 Core Profile (for Qt features only)

## Dependencies

### C++ Features

- Eigen (for macOS users, `brew install eigen`; for Ubuntu18.04 users, `apt-get install libeigen3-dev`)

### C++ Qt Features

- Qt6 (for macOS users, `brew install qt`) or Qt5 (for Ubuntu 18.04 users, `apt-get install qt5-default`)

## C++ API

```
Eigen::Vector3d enhance(const Eigen::Vector3d& input_rgb,
                        const Eigen::VectorXd& parameters);
```
where `input_rgb` is a 3-dimensional vector (\[0, 1\]^3), and `parameters` is a 5-dimensional vector (\[0, 1\]^5).

## Projects using enhancer

- Sequential Gallery [SIGGRAPH 2020] <https://github.com/yuki-koyama/sequential-gallery>
- Sequential Line Search [SIGGRAPH 2017] <https://github.com/yuki-koyama/sequential-line-search>
- SelPh [CHI 2016] <https://github.com/yuki-koyama/selph>

## Note: About Older Version (v1)

This library used to have a different procedure to enhance photographs, which was used for some previous publications [Koyama+UIST14; Koyama+CHI16; Koyama+SIGGRAPH17]. The original procedure has `color_balance` parameters (3D) instead of `temperature` and `tint` (2D). Thus, the total number of parameters was 6, not 5.

The `color_balance` parameters are for manipulating the chrominance of the input color, which is inherently 2D, and so assigning 3 parameters for this purpose is considered redundant. For example, `color_balance = [0.5, 0.5, 0.5]` and `color_balance = [0.2, 0.2, 0.2]` result in the same color even though the parameter sets are different.

Thus, the enhancement procedure was updated to use `temperature` and `tint` instead of `color_balance`. These two parameters are popularly used in Adobe Photoshop and Lightroom and so experts are likely to familiar with the concept. These two parameters also appeared in [Kapoor+IJCV14].
