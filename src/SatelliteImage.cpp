#include"SatelliteImage.h"
#include"Algorithms.h"
#include<algorithm>
#include<numeric>
#include<stdexcept>
#include<cmath>
#include<fstream>
#include<iomanip>
#include<sstream>
#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>

void SatelliteImage::calculateStatistics() {
    bandStatistics.clear();
    if (data.empty() || bands == 0) return;
    bandStatistics.resize(bands, 0.0);
    std::vector<size_t> counts(bands, 0);
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            for (int b = 0; b < bands; ++b) {
                bandStatistics[b] += static_cast<double>(data[r][c][b]);
                counts[b]++;
            }
        }
    }
    for (int b = 0; b < bands; ++b)
        if (counts[b] > 0) bandStatistics[b] /= counts[b];
} // 计算每个波段的平均值，存储在bandStatistics中，使用counts统计每个波段的像素数量以计算平均值

void SatelliteImage::validateCoordinates(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height)
        throw std::out_of_range("Pixel coordinates out of image bounds.");
} // 私有辅助函数：验证像素坐标是否合法，抛出异常

SatelliteImage::SatelliteImage(const std::string& id, const std::string& name, const std::string& path,
                               int w, int h, int b, const std::string& sensor, double time)
    : DataObject(id, name, path), width(w), height(h), bands(b), sensorType(sensor),
      cloudCover(0.0), acquisitionTime(time) {
    data.resize(height, std::vector<Pixel<double>>(width));
    calculateStatistics();
}


SatelliteImage::SatelliteImage(const SatelliteImage& other)
    : DataObject(other), width(other.width), height(other.height), bands(other.bands),
      sensorType(other.sensorType), cloudCover(other.cloudCover),
      acquisitionTime(other.acquisitionTime), data(other.data),
      bandStatistics(other.bandStatistics) {}


SatelliteImage::~SatelliteImage() {}


SatelliteImage& SatelliteImage::operator=(const SatelliteImage& other) {
    if (this != &other) {
        DataObject::operator=(other);
        width = other.width;
        height = other.height;
        bands = other.bands;
        sensorType = other.sensorType;
        cloudCover = other.cloudCover;
        acquisitionTime = other.acquisitionTime;
        data = other.data;
        bandStatistics = other.bandStatistics;
    }
    return *this;
} //即拷贝赋值运算符

void SatelliteImage::display() const { //实现时可以不写override
    std::cout << "Satellite Image: " << name << " (" << id << ")\n";
    std::cout << "  Dimensions: " << width << " x " << height << " x " << bands << " bands\n";
    std::cout << "  Sensor: " << sensorType << ", Cloud Cover: " << cloudCover << "%\n";
    std::cout << "  Path: " << path << ", Size (MB): " << size << "\n";
}

DataObject* SatelliteImage::clone() const {
    return new SatelliteImage(*this);
}

bool SatelliteImage::exportData(const std::string& format) const {
    if (format == "csv") {
        std::string filename = id + "_export.csv";
        std::ofstream file(filename);
        if (!file.is_open()) return false;
        file << "row,col";
        for (int b = 0; b < bands; ++b) file << ",band" << b;
        file << "\n";
        for (int r = 0; r < height; ++r) {
            for (int c = 0; c < width; ++c) {
                file << r << "," << c;
                for (int b = 0; b < bands; ++b)
                    file << "," << std::fixed << std::setprecision(6) << data[r][c][b]; //fixed和setprecision控制小数点格式和精度
                file << "\n";
            }
        }
        file.close();
        return true;
    } else if (format == "txt") {
        std::string filename = id + "_info.txt";
        std::ofstream file(filename);
        if (!file.is_open()) return false;
        file << "Image ID: " << id << ", Name: " << name << "\n";
        file << "Size: " << width << "x" << height << "x" << bands << "\n";
        file << "Sensor: " << sensorType << ", Cloud Cover: " << cloudCover << "%\n";
        file << "Band statistics (mean values):\n";
        for (size_t i = 0; i < bandStatistics.size(); ++i)
            file << "  Band " << i+1 << ": " << bandStatistics[i] << "\n";
        file.close();
        return true;
    }
    std::cerr << "Unsupported export format: " << format << std::endl; // 其他格式暂不支持
    return false;
}

double SatelliteImage::getQualityScore() const {
    return std::max(0.0, 100.0 - cloudCover);
}

bool SatelliteImage::isValid() const {
    return width > 0 && height > 0 && bands > 0 && !data.empty();
}


SatelliteImage SatelliteImage::operator+(const SatelliteImage& other) const {
    if (width != other.width || height != other.height || bands != other.bands)
        throw std::invalid_argument("Image dimensions must match for addition.");
    SatelliteImage result(id + "_sum", name + "+" + other.name, path,
                          width, height, bands, sensorType, acquisitionTime);
    for (int r = 0; r < height; ++r)
        for (int c = 0; c < width; ++c)
            result.data[r][c] = data[r][c] + other.data[r][c];
    result.calculateStatistics();
    return result;
}

SatelliteImage SatelliteImage::operator-(const SatelliteImage& other) const {
    if (width != other.width || height != other.height || bands != other.bands)
        throw std::invalid_argument("Image dimensions must match for subtraction.");
    SatelliteImage result(id + "_diff", name + "-" + other.name, path,
                          width, height, bands, sensorType, acquisitionTime);
    for (int r = 0; r < height; ++r)
        for (int c = 0; c < width; ++c)
            result.data[r][c] = data[r][c] - other.data[r][c];
    result.calculateStatistics();
    return result;
}

SatelliteImage SatelliteImage::operator*(double factor) const {
    SatelliteImage result(id + "_mul", name + "*factor", path,
                          width, height, bands, sensorType, acquisitionTime);
    for (int r = 0; r < height; ++r)
        for (int c = 0; c < width; ++c)
            result.data[r][c] = data[r][c] * factor;
    result.calculateStatistics();
    return result;
}

SatelliteImage SatelliteImage::operator/(double divisor) const {
    if (divisor == 0.0)
        throw std::invalid_argument("Division by zero in image scaling.");
    SatelliteImage result(id + "_div", name + "/factor", path,
                          width, height, bands, sensorType, acquisitionTime);
    for (int r = 0; r < height; ++r)
        for (int c = 0; c < width; ++c)
            result.data[r][c] = data[r][c] / divisor;
    result.calculateStatistics();
    return result;
}

SatelliteImage& SatelliteImage::operator+=(const SatelliteImage& other) {
    if (width != other.width || height != other.height || bands != other.bands)
        throw std::invalid_argument("Image dimensions must match for +=");
    for (int r = 0; r < height; ++r)
        for (int c = 0; c < width; ++c)
            data[r][c] += other.data[r][c];
    calculateStatistics();
    return *this;
}

SatelliteImage& SatelliteImage::operator-=(const SatelliteImage& other) {
    if (width != other.width || height != other.height || bands != other.bands)
        throw std::invalid_argument("Image dimensions must match for -=");
    for (int r = 0; r < height; ++r)
        for (int c = 0; c < width; ++c)
            data[r][c] -= other.data[r][c];
    calculateStatistics();
    return *this;
}


bool SatelliteImage::operator==(const SatelliteImage& other) const {
    return width == other.width && height == other.height && bands == other.bands &&
           sensorType == other.sensorType && cloudCover == other.cloudCover &&
           acquisitionTime == other.acquisitionTime && data == other.data &&
           bandStatistics == other.bandStatistics;
}

bool SatelliteImage::operator!=(const SatelliteImage& other) const {
    return !(*this == other);
}

std::vector<Pixel<double>>& SatelliteImage::operator[](int row) {
    if (row < 0 || row >= height) throw std::out_of_range("Row index out of range.");
    return data[row];
} // 返回的是对行的引用，可以修改数据

const std::vector<Pixel<double>>& SatelliteImage::operator[](int row) const {
    if (row < 0 || row >= height) throw std::out_of_range("Row index out of range.");
    return data[row];
} // const版本返回const引用，不能修改数据


SatelliteImage::operator double() const {
    return cloudCover;
} // 转换为云量百分比，直接返回成员变量cloudCover

SatelliteImage::operator std::string() const {
    std::ostringstream oss;
    oss << "SatelliteImage(" << id << ", " << width << "x" << height
        << ", " << bands << " bands)";
    return oss.str();
} // 转换为摘要字符串，使用ostringstream构建字符串


SatelliteImage SatelliteImage::getSubImage(int x, int y, int w, int h) const {
    if (x < 0 || y < 0 || x + w > width || y + h > height || w <= 0 || h <= 0)
        throw std::out_of_range("Sub-image region out of bounds.");
    SatelliteImage sub(id + "_sub", name + "_sub", path, w, h, bands, sensorType, acquisitionTime);
    for (int r = 0; r < h; ++r)
        for (int c = 0; c < w; ++c)
            sub.data[r][c] = data[y + r][x + c];
    sub.calculateStatistics();
    sub.cloudCover = cloudCover;
    return sub;
}

std::vector<double> SatelliteImage::getBandValues(int band) const {
    if (band < 0 || band >= bands) throw std::out_of_range("Band index out of range.");
    std::vector<double> values;
    values.reserve(width * height);
    for (int r = 0; r < height; ++r)
        for (int c = 0; c < width; ++c)
            values.push_back(data[r][c][band]);
    return values;
}

SatelliteImage SatelliteImage::extractBand(int band) const {
    if (band < 0 || band >= bands) throw std::out_of_range("Band index out of range.");
    SatelliteImage bandImg(id + "_b" + std::to_string(band), name + "_band", path,
                           width, height, 1, sensorType, acquisitionTime);
    for (int r = 0; r < height; ++r)
        for (int c = 0; c < width; ++c)
            bandImg.data[r][c][0] = data[r][c][band];
    bandImg.calculateStatistics();
    return bandImg;
}


std::vector<std::vector<double>> SatelliteImage::calculateNDVI() const {
    if (bands < 4) throw std::runtime_error("NDVI requires at least 4 bands.");
    const int redBand = 0, nirBand = 3;
    std::vector<std::vector<double>> ndvi(height, std::vector<double>(width));
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            double red = data[r][c][redBand];
            double nir = data[r][c][nirBand];
            double denom = nir + red;
            ndvi[r][c] = (abs(denom) < 1e-9) ? 0.0 : (nir - red) / denom;
        }
    }
    return ndvi;
}

std::vector<std::vector<double>> SatelliteImage::calculateNDWI() const {
    if (bands < 4) throw std::runtime_error("NDWI requires at least 4 bands.");
    const int greenBand = 1, nirBand = 3;
    std::vector<std::vector<double>> ndwi(height, std::vector<double>(width));
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            double green = data[r][c][greenBand];
            double nir = data[r][c][nirBand];
            double denom = green + nir;
            ndwi[r][c] = (abs(denom) < 1e-9) ? 0.0 : (green - nir) / denom;
        }
    }
    return ndwi;
}

std::vector<std::vector<double>> SatelliteImage::calculateSAVI(double L) const {
    if (bands < 4) throw std::runtime_error("SAVI requires at least 4 bands.");
    const int redBand = 0, nirBand = 3;
    std::vector<std::vector<double>> savi(height, std::vector<double>(width));
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            double red = data[r][c][redBand];
            double nir = data[r][c][nirBand];
            double denom = nir + red + L;
            savi[r][c] = (abs(denom) < 1e-9) ? 0.0 : (nir - red) / denom * (1.0 + L);
        }
    }
    return savi;
}

std::vector<std::vector<double>> SatelliteImage::calculateTemperature() const {
    if (bands < 5) throw std::runtime_error("Temperature requires 5 bands.");
    const int thermalBand = 4;
    const double K1 = 607.76, K2 = 1260.56;
    std::vector<std::vector<double>> temp(height, std::vector<double>(width));
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            double val = data[r][c][thermalBand];
            if (val <= 0) temp[r][c] = 0.0;
            else {
                double tb = K2 / log(K1 / val + 1.0);
                temp[r][c] = tb - 273.15;
            }
        }
    }
    return temp;
}

double SatelliteImage::getMeanValue() const {
    double sum = 0.0;
    size_t count = 0;
    for (int r = 0; r < height; ++r)
        for (int c = 0; c < width; ++c)
            for (int b = 0; b < bands; ++b) {
                sum += static_cast<double>(data[r][c][b]);
                ++count;
            }
    return count ? sum / count : 0.0;
}

double SatelliteImage::getStdDev() const {
    double mean = getMeanValue();
    double sq_sum = 0.0;
    size_t count = 0;
    for (int r = 0; r < height; ++r)
        for (int c = 0; c < width; ++c)
            for (int b = 0; b < bands; ++b) {
                double diff = static_cast<double>(data[r][c][b]) - mean;
                sq_sum += diff * diff;
                ++count;
            }
    return count > 1 ? sqrt(sq_sum / count) : 0.0;
}

double SatelliteImage::getMinValue() const {
    double minVal = std::numeric_limits<double>::max();
    for (int r = 0; r < height; ++r)
        for (int c = 0; c < width; ++c)
            for (int b = 0; b < bands; ++b)
                minVal = std::min(minVal, static_cast<double>(data[r][c][b]));
    return minVal;
}

double SatelliteImage::getMaxValue() const {
    double maxVal = std::numeric_limits<double>::lowest();
    for (int r = 0; r < height; ++r)
        for (int c = 0; c < width; ++c)
            for (int b = 0; b < bands; ++b)
                maxVal = std::max(maxVal, static_cast<double>(data[r][c][b]));
    return maxVal;
}

void SatelliteImage::printStatistics() const {
    std::cout << "Image Statistics for " << id << ":\n";
    std::cout << "  Size: " << width << "x" << height << "x" << bands << std::endl;
    std::cout << "  Mean: " << getMeanValue() << "  StdDev: " << getStdDev() << std::endl;
    std::cout << "  Min: " << getMinValue() << "  Max: " << getMaxValue() << std::endl;
    if (!bandStatistics.empty()) {
        std::cout << "  Per-band means: ";
        for (size_t i = 0; i < bandStatistics.size(); ++i)
            std::cout << bandStatistics[i] << " ";
        std::cout << std::endl;
    }
}



// 私有辅助函数：将内部数据转为 cv::Mat (5 通道 double)
cv::Mat SatelliteImage::toCvMat() const {
    cv::Mat mat(height, width, CV_64FC(5));
    for (int r = 0; r < height; ++r) {
        cv::Vec<double, 5>* rowPtr = mat.ptr<cv::Vec<double, 5>>(r);
        for (int c = 0; c < width; ++c) {
            const Pixel<double>& p = data[r][c];
            // 使用下标运算符访问波段，不触发 accessCount
            rowPtr[c] = cv::Vec<double, 5>(p[0], p[1], p[2], p[3], p[4]);
        }
    }
    return mat;
}

// 私有辅助函数：将 cv::Mat 写回内部数据
void SatelliteImage::fromCvMat(const cv::Mat& mat) {
    CV_Assert(mat.type() == CV_64FC(5));
    int newHeight = mat.rows;
    int newWidth  = mat.cols;

    data.clear();
    data.resize(newHeight, std::vector<Pixel<double>>(newWidth));

    for (int r = 0; r < newHeight; ++r) {
        const cv::Vec<double, 5>* rowPtr = mat.ptr<cv::Vec<double, 5>>(r);
        for (int c = 0; c < newWidth; ++c) {
            const cv::Vec<double, 5>& v = rowPtr[c];
            data[r][c] = Pixel<double>(v[0], v[1], v[2], v[3], v[4], 0.0);
        }
    }
    width  = newWidth;
    height = newHeight;
    calculateStatistics();
}

void SatelliteImage::applyGaussianBlur(double sigma) {
    cv::Mat mat = toCvMat();
    cv::GaussianBlur(mat, mat, cv::Size(0, 0), sigma);
    fromCvMat(mat);
}

void SatelliteImage::applyMedianFilter(int kernelSize) {
    cv::Mat mat = toCvMat();
    cv::medianBlur(mat, mat, kernelSize);
    fromCvMat(mat);
}

void SatelliteImage::normalize() {
    cv::Mat mat = toCvMat();
    cv::normalize(mat, mat, 0.0, 1.0, cv::NORM_MINMAX);
    fromCvMat(mat);
}

void SatelliteImage::resample(int newWidth, int newHeight) {
    cv::Mat mat = toCvMat();
    cv::Mat resized;
    cv::resize(mat, resized, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_LINEAR);
    fromCvMat(resized);
}
// 调用OpenCV的函数进行图像处理，先将内部数据转换为cv::Mat格式，处理后再转换回内部数据格式，保持原有数据结构不变，同时利用OpenCV的高效算法进行处理


void SatelliteImage::setCloudCover(double cover) {
    if (cover < 0.0 || cover > 100.0)
        throw std::invalid_argument("Cloud cover must be 0-100.");
    cloudCover = cover;
}

void SatelliteImage::setAcquisitionTime(double time) {
    acquisitionTime = time;
}


SatelliteImage SatelliteImage::createRandomImage(const std::string& id, int w, int h) {
    SatelliteImage img(id, "RandomImage", "./", w, h, 5, "Simulated", 0.0);
    srand(static_cast<unsigned>(time(nullptr)));
    for (int r = 0; r < h; ++r)
        for (int c = 0; c < w; ++c) {
            Pixel<double> pix(rand() % 256, rand() % 256, rand() % 256,
                              rand() % 256, rand() % 256, 0.0);
            img.data[r][c] = pix;
        }
    img.calculateStatistics();
    return img;
}

SatelliteImage SatelliteImage::createConstantImage(const std::string& id, int w, int h,
                                                   const Pixel<double>& value) {
    SatelliteImage img(id, "ConstantImage", "./", w, h, 5, "Constant", 0.0);
    for (int r = 0; r < h; ++r)
        for (int c = 0; c < w; ++c)
            img.data[r][c] = value;
    img.calculateStatistics();
    return img;
}
// 静态工厂方法：返回类实例，创建随机图像和常量图像
