#ifndef enhancerwidget_hpp
#define enhancerwidget_hpp

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QImage>

namespace enhancer
{
    class EnhancerWidget : public QOpenGLWidget, protected QOpenGLFunctions
    {
    public:
        EnhancerWidget(QWidget* parent = nullptr);
        void setImage(const QImage& image) { image_ = image; }

    protected:
        void initializeGL() override;
        void paintGL() override;
        void resizeGL(int width, int height) override;

    private:
        QImage image_;
    };
}

#endif /* enhancerwidget_hpp */
