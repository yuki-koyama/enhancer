#include <enhancer/enhancerwidget.hpp>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

namespace enhancer
{
    EnhancerWidget::EnhancerWidget(QWidget* parent) :
    QOpenGLWidget(parent)
    {
    }

    void EnhancerWidget::initializeGL()
    {
        initializeOpenGLFunctions();
    }

    void EnhancerWidget::paintGL()
    {
    }

    void EnhancerWidget::resizeGL(int width, int height)
    {
    }
}
