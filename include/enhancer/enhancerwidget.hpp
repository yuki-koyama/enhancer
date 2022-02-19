#ifndef enhancerwidget_hpp
#define enhancerwidget_hpp

#include <QImage>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <array>
#include <cassert>
#include <enhancer/enhancer.hpp>
#include <memory>

class QOpenGLShaderProgram;
class QOpenGLTexture;

namespace enhancer
{
    class EnhancerWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_2_Core
    {
    public:
        enum class Policy
        {
            AspectFit,
            AspectFill,
        };

        EnhancerWidget(const Policy policy = Policy::AspectFit, QWidget* parent = nullptr);
        ~EnhancerWidget();

        void setImage(const QImage& image);
        const QImage& getImage() const { return m_image; }

        void setParameters(const std::array<GLfloat, NUM_PARAMETERS>& parameters)
        {
            m_parameters = parameters;
        }

        template <typename T>
        void setParameters(const std::array<T, NUM_PARAMETERS>& parameters)
        {
            for (int i = 0; i < NUM_PARAMETERS; ++ i) { m_parameters[i] = static_cast<GLfloat>(parameters[i]); }
        }

        template <typename T>
        void setParameters(const std::vector<T>& parameters)
        {
            assert(parameters.size() == NUM_PARAMETERS);
            for (int i = 0; i < NUM_PARAMETERS; ++ i) { m_parameters[i] = static_cast<GLfloat>(parameters[i]); }
        }

        template <typename T>
        void setParameters(const T parameters[])
        {
            for (int i = 0; i < NUM_PARAMETERS; ++ i) { m_parameters[i] = static_cast<GLfloat>(parameters[i]); }
        }

        template <typename T>
        void setParameters(const Eigen::Matrix<T, Eigen::Dynamic, 1>& parameters)
        {
            assert(parameters.size() == NUM_PARAMETERS);
            for (int i = 0; i < NUM_PARAMETERS; ++ i) { m_parameters[i] = static_cast<GLfloat>(parameters[i]); }
        }

    protected:
        void initializeGL() override;
        void paintGL() override;
        void resizeGL(int width, int height) override;

    private:
        QImage m_image;
        bool   m_dirty;
        Policy m_policy;

        std::array<GLfloat, NUM_PARAMETERS> m_parameters;

        std::shared_ptr<QOpenGLShaderProgram> m_program;
        std::shared_ptr<QOpenGLTexture>       m_texture;

        QOpenGLVertexArrayObject m_vao;
        QOpenGLBuffer            m_vbo;
    };
} // namespace enhancer

#endif /* enhancerwidget_hpp */
