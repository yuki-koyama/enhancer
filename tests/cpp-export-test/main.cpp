#include <QImage>
#include <enhancer/enhancer.hpp>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

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

inline QRgb convertEigenToQRgb(const Eigen::Vector3d& color)
{
    return qRgb(color[0] * 255.0, color[1] * 255.0, color[2] * 255.0);
}

inline Eigen::Vector3d convertQRgbToEigen(const QRgb& color)
{
    return Eigen::Vector3d(qRed(color), qGreen(color), qBlue(color)) / 255.0;
}

QImage enhanceQImage(const QImage& target_image, const Eigen::VectorXd& parameters)
{
    QImage enhanced_image(target_image.size(), target_image.format());
    for (int x = 0; x < target_image.width(); ++x)
    {
        for (int y = 0; y < target_image.height(); ++y)
        {
            const QRgb            qrgb_color           = target_image.pixel(x, y);
            const Eigen::Vector3d eigen_color          = convertQRgbToEigen(qrgb_color);
            const Eigen::Vector3d eigen_enhanced_color = enhancer::enhance(eigen_color, parameters);
            const QRgb            qrgb_enhanced_color  = convertEigenToQRgb(eigen_enhanced_color);

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

    for (int dim = 0; dim < enhancer::NUM_PARAMETERS; ++dim)
    {
        for (int step = 0; step < num_steps; ++step)
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
