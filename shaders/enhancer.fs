#version 330

smooth in vec2 vertex_uv;
out vec4 frag_color;
uniform sampler2D texture_sampler;

#if defined(ENHANCER_V_1_0)
uniform float parameters[6];
#elif defined(ENHANCER_WITH_LIFT_GAMMA_GAIN)
uniform float parameters[14];
#else
uniform float parameters[5];
#endif

vec3 convertRgbToLinearRgb(const vec3 rgb)
{
    return pow(rgb, vec3(2.2));
}

vec3 convertLinearRgbToRgb(const vec3 linear_rgb)
{
    return pow(linear_rgb, vec3(1.0 / 2.2));
}

// Y'UV (BT.709) to linear RGB
// Values are from https://en.wikipedia.org/wiki/YUV
vec3 yuv2rgb(vec3 yuv)
{
    const mat3 m = mat3(+1.00000, +1.00000, +1.00000,  // 1st column
                        +0.00000, -0.21482, +2.12798,  // 2nd column
                        +1.28033, -0.38059, +0.00000); // 3rd column
    return m * yuv;
}

// Linear RGB to Y'UV (BT.709)
// Values are from https://en.wikipedia.org/wiki/YUV
vec3 rgb2yuv(vec3 rgb)
{
    const mat3 m = mat3(+0.21260, -0.09991, +0.61500,  // 1st column
                        +0.71520, -0.33609, -0.55861,  // 2nd column
                        +0.07220, +0.43600, -0.05639); // 3rd column
    return m * rgb;
}

float rgb2h(vec3 rgb)
{
    float r = rgb[0];
    float g = rgb[1];
    float b = rgb[2];

    float M = max(r, max(g, b));
    float m = min(r, min(g, b));

    float h;
    if (M == m) {
        h = 0.0;
    } else if (m == b) {
        h = 60.0 * (g - r) / (M - m) + 60.0;
    } else if (m == r) {
        h = 60.0 * (b - g) / (M - m) + 180.0;
    } else if (m == g) {
        h = 60.0 * (r - b) / (M - m) + 300.0;
    } else {
        h = 0.0;
    }
    h /= 360.0;
    if (h < 0.0) {
        h = h + 1.0;
    } else if (h > 1.0) {
        h = h - 1.0;
    }
    return h;
}

float rgb2s4hsv(vec3 rgb)
{
    float r = rgb[0];
    float g = rgb[1];
    float b = rgb[2];
    float M = max(r, max(g, b));
    float m = min(r, min(g, b));

    if (M < 1e-10) return 0.0;
    return (M - m) / M;
}

float rgb2s4hsl(vec3 rgb)
{
    float r = rgb[0];
    float g = rgb[1];
    float b = rgb[2];
    float M = max(r, max(g, b));
    float m = min(r, min(g, b));

    if (M - m < 1e-10) return 0.0;
    return (M - m) / (1.0 - abs(M + m - 1.0));
}

float rgb2L(vec3 rgb)
{
    float m = min(min(rgb.x, rgb.y), rgb.z);
    float M = max(max(rgb.x, rgb.y), rgb.z);

    return (M + m) * 0.5;
}

vec3 rgb2hsl(vec3 rgb)
{
    vec3 hsl;

    hsl.x = rgb2h(rgb);
    hsl.y = rgb2s4hsl(rgb);
    hsl.z = rgb2L(rgb);

    return hsl;
}

float h2rgb(float f1, float f2, float hue)
{
    if (hue < 0.0)
    hue += 1.0;
    else if (hue >= 1.0)
    hue -= 1.0;
    float res;
    if ((6.0 * hue) < 1.0)
    res = f1 + (f2 - f1) * 6.0 * hue;
    else if ((2.0 * hue) < 1.0)
    res = f2;
    else if ((3.0 * hue) < 2.0)
    res = f1 + (f2 - f1) * ((2.0 / 3.0) - hue) * 6.0;
    else
    res = f1;
    return res;
}

vec3 hsl2rgb(vec3 hsl)
{
    vec3 rgb;

    if (hsl.y == 0.0) {
        rgb = vec3(hsl.z, hsl.z, hsl.z); // Luminance
    } else {
        float f2;

        if (hsl.z < 0.5) {
            f2 = hsl.z * (1.0 + hsl.y);
        } else {
            f2 = (hsl.z + hsl.y) - (hsl.y * hsl.z);
        }

        float f1 = 2.0 * hsl.z - f2;

        rgb.x = h2rgb(f1, f2, hsl.x + (1.0 / 3.0));
        rgb.y = h2rgb(f1, f2, hsl.x);
        rgb.z = h2rgb(f1, f2, hsl.x - (1.0 / 3.0));
    }

    return rgb;
}

vec3 changeColorBalance(vec3 rgb, vec3 param)
{
    float lightness = rgb2L(rgb);

    const float a     = 0.25;
    const float b     = 0.333;
    const float scale = 0.7;

    vec3 midtones = (clamp((lightness - b) /  a + 0.5, 0.0, 1.0) * clamp((lightness + b - 1.0) / -a + 0.5, 0.0, 1.0) * scale) * param;

    vec3 newColor = rgb + midtones;
    newColor = clamp(newColor, 0.0, 1.0);

    // preserve luminosity
    vec3 newHsl = rgb2hsl(newColor);
    return hsl2rgb(vec3(newHsl.x, newHsl.y, lightness));
}

vec3 rgb2hsv(vec3 rgb)
{
    float h = rgb2h(rgb);
    float s = rgb2s4hsv(rgb);
    float v = max(rgb.x, max(rgb.y, rgb.z));
    return vec3(h, s, v);
}

vec3 hsv2rgb(vec3 hsv) {
    vec3 rgb;

    float r, g, b, h, s, v;

    h = hsv.x;
    s = hsv.y;
    v = hsv.z;

    if (s <= 0.001) {
        r = g = b = v;
    } else {
        float f, p, q, t;
        int i;
        h *= 6.0;
        i = int(floor(h));
        f = h - float(i);
        p = v * (1.0 - s);
        q = v * (1.0 - (s * f));
        t = v * (1.0 - (s * (1.0 - f)));
        if (i == 0 || i == 6) {
            r = v; g = t; b = p;
        } else if (i == 1) {
            r = q; g = v; b = p;
        } else if (i == 2) {
            r = p; g = v; b = t;
        } else if (i == 3) {
            r = p; g = q; b = v;
        } else if (i == 4) {
            r = t; g = p; b = v;
        } else if (i == 5) {
            r = v; g = p; b = q;
        }
    }
    rgb.x = r;
    rgb.y = g;
    rgb.z = b;
    return rgb;
}

vec3 applyLiftGammaGainEffect(const vec3 linear_rgb, const vec3 lift, const vec3 gamma, const vec3 gain)
{
    vec3 lift_applied_linear_rgb  = clamp((linear_rgb - vec3(1.0)) * (vec3(2.0) - lift) + vec3(1.0), 0.0, 1.0);
    vec3 gain_applied_linear_rgb  = lift_applied_linear_rgb * gain;
    vec3 gamma_applied_linear_rgb = pow(gain_applied_linear_rgb, vec3(1.0) / clamp(gamma, 1e-06, 2.0));

    return gamma_applied_linear_rgb;
}

vec3 applyTemperatureTintEffect(const vec3 linear_rgb, const float temperature, const float tint)
{
    const float scale = 0.10;
    return clamp(yuv2rgb(rgb2yuv(linear_rgb) + temperature * scale * vec3(0.0, -1.0, 1.0) + tint * scale * vec3(0.0, 1.0, 1.0)), 0.0, 1.0);
}

vec3 applyBrightnessEffect(const vec3 linear_rgb, const float brightness)
{
    const float scale = 1.5;
    return pow(linear_rgb, vec3(1.0 / (1.0 + scale * brightness)));
}

vec3 applySaturationEffect(const vec3 linear_rgb, const float saturation)
{
    vec3 hsv = rgb2hsv(clamp(linear_rgb, 0.0, 1.0));
    float s = clamp(hsv[1] * (saturation + 1.0), 0.0, 1.0);
    return hsv2rgb(vec3(hsv[0], s, hsv[2]));
}

vec3 applyContrastEffect(const vec3 linear_rgb, const float contrast)
{
    const float pi_4 = 3.14159265358979 * 0.25;
    float contrast_coef = tan((contrast + 1.0) * pi_4);
    return convertRgbToLinearRgb(max(contrast_coef * (convertLinearRgbToRgb(linear_rgb) - vec3(0.5)) + vec3(0.5), 0.0));
}

vec3 enhance(vec3 color)
{
    // Retrieve enhancement parameters
    float brightness   = clamp(parameters[0], 0.0, 1.0) - 0.5;
    float contrast     = clamp(parameters[1], 0.0, 1.0) - 0.5;
    float saturation   = clamp(parameters[2], 0.0, 1.0) - 0.5;
    float temperature  = clamp(parameters[3], 0.0, 1.0) - 0.5;
    float tint         = clamp(parameters[4], 0.0, 1.0) - 0.5;

#if defined(ENHANCER_WITH_LIFT_GAMMA_GAIN)
    vec3 lift  = vec3(0.5) + clamp(vec3(parameters[5], parameters[6], parameters[7]), 0.0, 1.0); // [0.5, 1.5]^3
    vec3 gamma = 2.0 * clamp(vec3(parameters[8], parameters[9], parameters[10]), 0.0, 1.0);      // [0.0, 2.0]^3
    vec3 gain  = 2.0 * clamp(vec3(parameters[11], parameters[12], parameters[13]), 0.0, 1.0);    // [0.0, 2.0]^3
#endif

    vec3 linear_rgb = convertRgbToLinearRgb(color);

#if defined(ENHANCER_WITH_LIFT_GAMMA_GAIN)
    // Lift/Gamma/Gain
    linear_rgb = applyLiftGammaGainEffect(linear_rgb, lift, gamma, gain);
#endif

    // Approximate temperature/tint effect
    linear_rgb = applyTemperatureTintEffect(linear_rgb, temperature, tint);

    // Brightness
    linear_rgb = applyBrightnessEffect(linear_rgb, brightness);

    // Contrast
    linear_rgb = applyContrastEffect(linear_rgb, contrast);

    // Saturation
    linear_rgb = applySaturationEffect(linear_rgb, saturation);

    return clamp(convertLinearRgbToRgb(linear_rgb), 0.0, 1.0);
}

#ifdef ENHANCER_V_1_0
vec3 enhance_v1(vec3 color)
{
    // Retrieve enhancement parameters
    float brightness   = parameters[0] - 0.5;
    float contrast     = parameters[1] - 0.5;
    float saturation   = parameters[2] - 0.5;
    vec3  balance      = vec3(parameters[3], parameters[4], parameters[5]) - vec3(0.5);

    // Apply color balance
    color = changeColorBalance(color, balance);

    // Apply brightness
    color *= 1.0 + brightness;

    // Apply contrast
    float cont = tan((contrast + 1.0) * 3.1415926535 * 0.25);
    color = (color - vec3(0.5)) * cont + vec3(0.5);

    // Clamp the values
    color = clamp(color, 0.0, 1.0);

    // Apply saturation
    vec3 hsv_color = rgb2hsv(color);
    float s = hsv_color[1];
    s *= saturation + 1.0;
    s = clamp(s, 0.0, 1.0);
    hsv_color[1] = s;
    color = hsv2rgb(hsv_color);

    // Output the resulting color
    return color;
}
#endif

void main()
{
    // Get raw texture color
    vec4 color = texture(texture_sampler, vertex_uv);

    // Enhance
#ifdef ENHANCER_V_1_0
    frag_color.xyz = enhance_v1(color.xyz);
#else
    frag_color.xyz = enhance(color.xyz);
#endif
    frag_color.w   = 1.0;
}
