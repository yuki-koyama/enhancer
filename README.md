# enhancer

A C++11 / GLSL library for enhancing photographs / images (adjusting brightness, contrast, etc.)

## Supported Parameters

- Brightness
- Contrast
- Saturation
- Color Balance
  - Red
  - Green
  - Blue

## C++ API

```
Eigen::Vector3d enhance(const Eigen::Vector3d& input_rgb,
                        const Eigen::VectorXd& parameters);
```
where `input_rgb` is a 3-dimensional vector (\[0, 1\]^3), and `parameters` is a 6-dimensional vector (\[0, 1\]^6).

## Projects using enhancer

- Sequential Line Search <https://github.com/yuki-koyama/sequential-line-search>
- SelPh <https://github.com/yuki-koyama/selph>
