#include <enhancer/enhancerwidget.hpp>
#include <QFile>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <iostream>

#define TEXTURE_UNIT_ID 0

namespace enhancer
{
    EnhancerWidget::EnhancerWidget(const Policy policy, QWidget* parent) :
    QOpenGLWidget(parent),
    m_dirty(true),
    m_policy(policy)
    {
        m_image = QImage(64, 64, QImage::Format_RGBA8888);
        m_image.fill(Qt::GlobalColor::darkGray);

        m_parameters.fill(0.5);
    }

    EnhancerWidget::~EnhancerWidget()
    {
        makeCurrent();
        m_vbo.destroy();
        m_vao.destroy();
        if (m_texture.get() != nullptr) { m_texture->destroy(); }
        doneCurrent();
    }

    void EnhancerWidget::setImage(const QImage& image)
    {
        m_image = image;
        m_dirty = true;
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

        constexpr GLfloat vertex_data[] =
        {
            -1.0, -1.0,
            +1.0, -1.0,
            +1.0, +1.0,
            -1.0, +1.0,
        };

        const auto code_loader = [](const char* file_name) -> QString
        {
            QFile file(file_name);

            if (!file.open(QIODevice::ReadOnly))
            {
                std::cerr << "Error: failed to load shader codes." << std::endl;
            }

#if defined(ENHANCER_WITH_LIFT_GAMMA_GAIN)
            QString code;
            QTextStream stream(&file);
            while (!stream.atEnd())
            {
                QString line = stream.readLine();

                if (line.contains("#version"))
                {
                    line.append("\n\n#define ENHANCER_WITH_LIFT_GAMMA_GAIN\n");
                }

                code.append(line).append("\n");
            }
#else
            const QString code = QTextStream(&file).readAll();
#endif

            file.close();

            return code;
        };

        const QString vert_code = code_loader("://shaders/enhancer.vs");
        const QString frag_code = code_loader("://shaders/enhancer.fs");

        m_program = std::make_shared<QOpenGLShaderProgram>();
        m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vert_code);
        m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, frag_code);
        m_program->link();
        m_program->bind();
        m_program->setUniformValue("texture_sampler", TEXTURE_UNIT_ID);
        m_program->release();

        m_vbo.create();
        m_vbo.bind();
        m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
        m_vbo.allocate(vertex_data, sizeof(vertex_data) * sizeof(GLfloat));
        m_vbo.release();

        m_vao.create();
        m_vao.bind();

        m_vbo.bind();
        m_program->enableAttributeArray("vertex_position");
        m_program->setAttributeBuffer("vertex_position", GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));

        m_vao.release();
    }

    void EnhancerWidget::paintGL()
    {
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const int image_width = this->m_image.width();
        const int image_height = this->m_image.height();
        const int w = width() * devicePixelRatio();
        const int h = height() * devicePixelRatio();

        switch (m_policy) {
            case Policy::AspectFit:
            {
                if (w * image_height == h * image_width)
                {
                    glViewport(0, 0, w, h);
                }
                else if (w * image_height > h * image_width)
                {
                    const int w_corrected = h * image_width / image_height;
                    glViewport((w - w_corrected) / 2, 0, w_corrected, h);
                }
                else if (w * image_height < h * image_width)
                {
                    const int h_corrected = w * image_height / image_width;
                    glViewport(0, (h - h_corrected) / 2, w, h_corrected);
                }
                break;
            }
            case Policy::AspectFill:
            {
                if (w * image_height == h * image_width)
                {
                    glViewport(0, 0, w, h);
                }
                else if (w * image_height > h * image_width)
                {
                    const int h_corrected = w * image_height / image_width;
                    glViewport(0, (h - h_corrected) / 2, w, h_corrected);
                }
                else if (w * image_height < h * image_width)
                {
                    const int w_corrected = h * image_width / image_height;
                    glViewport((w - w_corrected) / 2, 0, w_corrected, h);
                }
                break;
            }
        }

        if (m_dirty)
        {
            m_texture = std::make_shared<QOpenGLTexture>(m_image.mirrored(), QOpenGLTexture::DontGenerateMipMaps);
            m_dirty = false;
        }

        m_program->bind();
        m_program->setUniformValueArray("parameters", m_parameters.data(), NUM_PARAMETERS, 1);

        m_texture->bind(TEXTURE_UNIT_ID);
        m_vao.bind();
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        m_vao.release();
        m_texture->release();
        m_program->release();
    }

    void EnhancerWidget::resizeGL(int width, int height)
    {
    }
}
