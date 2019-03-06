#ifndef enhancerwidget_hpp
#define enhancerwidget_hpp

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QImage>

class QOpenGLShaderProgram;
class QOpenGLTexture;

namespace enhancer
{
    class EnhancerWidget : public QOpenGLWidget, protected QOpenGLFunctions
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

        QOpenGLShaderProgram* program_;
        QOpenGLTexture* texture_;
        QOpenGLBuffer vbo;
    };
}

#endif /* enhancerwidget_hpp */
