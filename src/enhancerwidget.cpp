#include <enhancer/enhancerwidget.hpp>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <iostream>

#define TEXTURE_UNIT_ID 0

namespace enhancer
{
    EnhancerWidget::EnhancerWidget(QWidget* parent) :
    QOpenGLWidget(parent),
    dirty_(true)
    {
        image_ = QImage(64, 64, QImage::Format_RGBA8888);
        image_.fill(Qt::GlobalColor::darkGray);
    }

    EnhancerWidget::~EnhancerWidget()
    {
        makeCurrent();
        vbo.destroy();
        vao.destroy();
        texture_->destroy();
        doneCurrent();
    }

    void EnhancerWidget::setImage(const QImage& image)
    {
        image_ = image;
        dirty_ = true;
    }

    void EnhancerWidget::initializeGL()
    {
        const bool is_opengl_ready = initializeOpenGLFunctions();

        if (!is_opengl_ready)
        {
            std::cerr << "Error: Failed to prepare OpenGL profile." << std::endl;
            exit(1);
        }

        const GLubyte* opengl_version = glGetString(GL_VERSION);
        const GLubyte* glsl_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
        std::cout << "OpenGL Version: " << opengl_version << std::endl;
        std::cout << "GLSL Version: " << glsl_version << std::endl;

        vao.create();
        vbo.create();

        vao.bind();
        vbo.bind();
        {
            constexpr GLfloat vertex_data[] =
            {
                -1.0, -1.0,
                +1.0, -1.0,
                +1.0, +1.0,
                -1.0, +1.0,
            };
            vbo.allocate(vertex_data, sizeof(vertex_data) * sizeof(GLfloat));
        }
        vbo.release();
        vao.release();

        QOpenGLShader *vertex_shader = new QOpenGLShader(QOpenGLShader::Vertex, this);
        const char *vertex_shader_source = R"(
/* ---------------------------------------------------------------------------*/
#version 330

layout(location = 0) in vec2 vertex_position;
smooth out vec2 vertex_uv;

void main()
{
    gl_Position = vec4(vertex_position, 0.0, 1.0);
    vertex_uv = vertex_position * vec2(0.5) + vec2(0.5);
}
/* ---------------------------------------------------------------------------*/
        )";
        vertex_shader->compileSourceCode(vertex_shader_source);

        QOpenGLShader *fragment_shader = new QOpenGLShader(QOpenGLShader::Fragment, this);
        const char *fragment_shader_source = R"(
/* ---------------------------------------------------------------------------*/
#version 330

smooth in vec2 vertex_uv;
out vec4 frag_color;
uniform sampler2D texture_sampler;
uniform float parameters[6];

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

void main()
{
    // Get raw texture color
    vec4 color = texture(texture_sampler, vertex_uv);

    // Retrieve enhancement parameters
    float brightness = parameters[0] - 0.5;
    float contrast = parameters[1] - 0.5;
    float saturation = parameters[2] - 0.5;
    vec3 color_balance = vec3(parameters[3], parameters[4], parameters[5]);

    // Apply color balance
    color.xyz = changeColorBalance(color.xyz, color_balance);

    // Apply brightness
    color.x *= 1.0 + brightness;
    color.y *= 1.0 + brightness;
    color.z *= 1.0 + brightness;

    // Apply contrast
    float cont = tan((contrast + 1.0) * 3.1415926535 * 0.25);
    color.x = (color.x - 0.5) * cont + 0.5;
    color.y = (color.y - 0.5) * cont + 0.5;
    color.z = (color.z - 0.5) * cont + 0.5;

    // Clamp the values
    color = clamp(color, 0.0, 1.0);

    // Apply saturation
    vec3 hsvVector = rgb2hsv(color.xyz);
    float s = hsvVector.y;
    s *= saturation + 1.0;
    s = clamp(s, 0.0, 1.0);
    hsvVector.y = s;
    color.xyz = hsv2rgb(hsvVector);

    // Output the resulting color
    frag_color = color;
}
/* ---------------------------------------------------------------------------*/
        )";
        fragment_shader->compileSourceCode(fragment_shader_source);

        program_ = std::make_shared<QOpenGLShaderProgram>();
        program_->addShader(vertex_shader);
        program_->addShader(fragment_shader);
        program_->link();

        program_->bind();
        {
            program_->setUniformValue("texture_sampler", TEXTURE_UNIT_ID);
        }
        program_->release();
    }

    void EnhancerWidget::paintGL()
    {
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (dirty_)
        {
            texture_ = std::make_shared<QOpenGLTexture>(image_.mirrored(), QOpenGLTexture::DontGenerateMipMaps);
            dirty_ = false;
        }

        program_->bind();
        vao.bind();
        vbo.bind();
        texture_->bind(TEXTURE_UNIT_ID);
        {
            program_->enableAttributeArray("vertex_position");
            program_->setAttributeBuffer("vertex_position", GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));

            constexpr GLfloat parameters[] =
            {
                0.5, 0.6, 0.4, 0.2, 0.2, 0.9
            };
            program_->setUniformValueArray("parameters", parameters, 6, 1);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            program_->disableAttributeArray("vertex_position");
        }
        texture_->release();
        vbo.release();
        vao.release();
        program_->release();
    }

    void EnhancerWidget::resizeGL(int width, int height)
    {
        glViewport(0, 0, width, height);
    }
}
