#ifndef enhancerwidget_hpp
#define enhancerwidget_hpp

#include <memory>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QImage>

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

    protected:
        void initializeGL() override;
        void paintGL() override;
        void resizeGL(int width, int height) override;

    private:
        QImage image_;
        bool dirty_;

        std::shared_ptr<QOpenGLShaderProgram> program_;
        std::shared_ptr<QOpenGLTexture> texture_;

        QOpenGLVertexArrayObject vao;
        QOpenGLBuffer vbo;
    };
}

#endif /* enhancerwidget_hpp */
