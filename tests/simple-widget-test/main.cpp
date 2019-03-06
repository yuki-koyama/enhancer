#include <enhancer/enhancerwidget.hpp>
#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

#if defined(__APPLE__)
    QSurfaceFormat format;
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
#endif

    enhancer::EnhancerWidget widget;
    widget.show();
    return app.exec();
}
