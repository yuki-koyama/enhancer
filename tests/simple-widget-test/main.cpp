#include <QApplication>
#include <enhancer/enhancerwidget.hpp>

#if defined(EXPORT_TEST)
#include <iomanip>
#include <sstream>
#include <string>

std::string convertParametersToString(const Eigen::VectorXd& parameters)
{
    std::ostringstream sstream;

    sstream << "p";
    for (int i = 0; i < parameters.size(); ++i)
    {
        sstream << "_";
        sstream << std::fixed << std::setprecision(2) << parameters[i];
    }

    return sstream.str();
}
#endif

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Q_INIT_RESOURCE(enhancer_resources);

#if defined(__APPLE__)
    QSurfaceFormat format;
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
#endif

    enhancer::EnhancerWidget widget;
    widget.setImage(QImage("://test-images/DSC03039.JPG"));
    widget.show();

#if defined(EXPORT_TEST)
    constexpr int num_steps = 5;
    constexpr int width     = 960;
    constexpr int height    = 640;

    widget.setFixedWidth(width / 2);
    widget.setFixedHeight(height / 2);

    for (int dim = 0; dim < enhancer::NUM_PARAMETERS; ++dim)
    {
        for (int step = 0; step < num_steps; ++step)
        {
            std::vector<double> parameters_data(enhancer::NUM_PARAMETERS, 0.5);
            parameters_data[dim] = static_cast<double>(step) / static_cast<double>(num_steps - 1);

            const Eigen::VectorXd parameters = Eigen::Map<const Eigen::VectorXd>(parameters_data.data(), enhancer::NUM_PARAMETERS);
            const std::string     path       = "./" + convertParametersToString(parameters) + ".png";

            widget.setParameters(parameters_data);
            widget.grab().save(QString::fromStdString(path));
        }
    }

    return 0;
#else
    widget.setParameters({ 0.6, 0.4, 0.6, 0.6, 0.5 });

    return app.exec();
#endif
}
