#include <enhancer/enhancerwidget.hpp>
#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    enhancer::EnhancerWidget widget;
    widget.show();
    return app.exec();
}
