#ifndef enhancerwidget_hpp
#define enhancerwidget_hpp

#include <array>
#include <cassert>
#include <memory>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QImage>
#include <enhancer/enhancer.hpp>

class QOpenGLShaderProgram;
class QOpenGLTexture;

namespace enhancer
{
    class EnhancerWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_2_Core
    {
    public:
        EnhancerWidget(QWidget* parent = nullptr);
        ~EnhancerWidget();

        void setImage(const QImage& image);
        const QImage& getImage() const { return image_; }

        void setParameters(const std::array<GLfloat, NUM_PARAMETERS>& parameters)
        {
            parameters_ = parameters;
        }

        template <typename T>
        void setParameters(const std::array<T, NUM_PARAMETERS>& parameters)
        {
            for (int i = 0; i < NUM_PARAMETERS; ++ i) { parameters_[i] = static_cast<GLfloat>(parameters[i]); }
        }

        template <typename T>
        void setParameters(const std::vector<T>& parameters)
        {
            assert(parameters.size() == NUM_PARAMETERS);
            for (int i = 0; i < NUM_PARAMETERS; ++ i) { parameters_[i] = static_cast<GLfloat>(parameters[i]); }
        }

        template <typename T>
        void setParameters(const T parameters[])
        {
            for (int i = 0; i < NUM_PARAMETERS; ++ i) { parameters_[i] = static_cast<GLfloat>(parameters[i]); }
        }

    protected:
        void initializeGL() override;
        void paintGL() override;
        void resizeGL(int width, int height) override;

    private:
        QImage image_;
        bool   dirty_;

        std::array<GLfloat, NUM_PARAMETERS> parameters_;

        std::shared_ptr<QOpenGLShaderProgram> program_;
        std::shared_ptr<QOpenGLTexture>       texture_;

        QOpenGLVertexArrayObject vao;
        QOpenGLBuffer            vbo;
    };
}

#endif /* enhancerwidget_hpp */
