#include <string>
#include <sstream>
#include <iomanip>
#include <enhancer/enhancer.hpp>
#include <QImage>

std::string convertParametersToString(const Eigen::VectorXd& parameters)
{
    std::ostringstream sstream;

    sstream << "p";
    for (int i = 0; i < parameters.size(); ++ i)
    {
        sstream << "_";
        sstream << std::fixed << std::setprecision(2) << parameters[i];
    }

    return sstream.str();
}

QImage enhanceQImage(const QImage& target_image, const Eigen::VectorXd& parameters)
{
    QImage enhanced_image(target_image.size(), target_image.format());
    for (int x = 0; x < target_image.width(); ++ x)
    {
        for (int y = 0; y < target_image.height(); ++ y)
        {
            const QRgb            qrgb_color           = target_image.pixel(x, y);
            const Eigen::Vector3d eigen_color          = Eigen::Vector3d(qRed(qrgb_color),
                                                                         qGreen(qrgb_color),
                                                                         qBlue(qrgb_color)) / 255.0;
            const Eigen::Vector3d eigen_enhanced_color = enhancer::enhance(eigen_color, parameters);
            const QRgb            qrgb_enhanced_color  = qRgb(eigen_enhanced_color[0] * 255,
                                                              eigen_enhanced_color[1] * 255,
                                                              eigen_enhanced_color[2] * 255);

            enhanced_image.setPixel(x, y, qrgb_enhanced_color);
        }
    }

    return enhanced_image;
}

int main(int argc, char** argv)
{
    constexpr int num_steps    = 5;
    constexpr int target_width = 960;

    Q_INIT_RESOURCE(enhancer_resources);
    const QImage target_image = QImage("://test-images/DSC03039.JPG").scaledToWidth(target_width);

    for (int dim = 0; dim < enhancer::NUM_PARAMETERS; ++ dim)
    {
        for (int step = 0; step < num_steps; ++ step)
        {
            std::vector<double> parameters_data(enhancer::NUM_PARAMETERS, 0.5);
            parameters_data[dim] = static_cast<double>(step) / static_cast<double>(num_steps - 1);

            const Eigen::VectorXd parameters     = Eigen::Map<const Eigen::VectorXd>(parameters_data.data(), enhancer::NUM_PARAMETERS);
            const QImage          enhanced_image = enhanceQImage(target_image, parameters);
            const std::string     path           = "./" + convertParametersToString(parameters) + ".png";

            enhanced_image.save(QString::fromStdString(path));
        }
    }

    return 0;
}
