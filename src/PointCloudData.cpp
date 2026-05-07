#include"PointCloudData.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <numeric>

Point3D::Point3D(double x, double y, double z, double intensity, int cls)
    : x(x), y(y), z(z), intensity(intensity), classification(cls) {}

Point3D Point3D::operator+(const Point3D&other) const{
    return Point3D(x + other.x, y + other.y, z + other.z, intensity + other.intensity, classification);    
}

Point3D Point3D::operator-(const Point3D&other) const{
    return Point3D(x - other.x, y - other.y, z - other.z, intensity - other.intensity, classification);
}

double Point3D::distanceTo(const Point3D& other) const{
    double dx = x - other.x;
    double dy = y - other.y;
    double dz = z - other.z;
    return sqrt(dx * dx + dy * dy + dz * dz);
}

void PointCloudData::updateBounds(double x,double y,double z){
    if(points.empty()){
        minX = maxX = x;
        minY = maxY = y;
        minZ = maxZ = z;
    }
    else{
        if(x < minX) minX = x;
        if(x > maxX) maxX = x;
        if(y < minY) minY = y;
        if(y > maxY) maxY = y;
        if(z < minZ) minZ = z;
        if(z > maxZ) maxZ = z;
    }
}

PointCloudData::PointCloudData(const std::string& id, const std::string& name, const std::string& path)
    : DataObject(id, name, path), minX(0), maxX(0), minY(0), maxY(0), minZ(0), maxZ(0) {}

PointCloudData::PointCloudData(const PointCloudData& other)
    : DataObject(other), // 必须调用基类的拷贝构造函数来复制基类成员
      minX(other.minX), maxX(other.maxX),
      minY(other.minY), maxY(other.maxY),
      minZ(other.minZ), maxZ(other.maxZ),
      points(other.points)
{
    this->accessCount = 0;
}

PointCloudData::~PointCloudData(){} // vector会自动释放内存

void PointCloudData::display() const {
    std::cout << "PointCloudData: " << name << " (" << id << ")" << std::endl;
    std::cout << "Path: " << path << ", Size: " << size << " MB, Created: " << std::ctime(&createTime);
    std::cout << "Points: " << points.size() << ", Bounds: [" 
              << minX << ", " << maxX << "] x [" 
              << minY << ", " << maxY << "] x [" 
              << minZ << ", " << maxZ << "]" << std::endl;
}

DataObject* PointCloudData::clone() const {
    return new PointCloudData(*this); // 使用拷贝构造函数创建新对象
}

bool PointCloudData::exportData(const std::string& format) const {
    std::string filename = this->getId() + "_export." + format; // getId()是基类DataObject的成员函数
    if (format == "txt"|| format == "csv") {
        std::ofstream file(filename);
        for (const auto& point : points) {
            file << std::fixed << std::setprecision(6) << point.x << "," << point.y << "," << point.z << "," << point.intensity << "," << point.classification << std::endl;
        }
        file.close();
        return true;
    }

    else if (format == "ply") {
        std::ofstream file(filename, std::ios::binary); // std::ios::binary表示系统不对文件进行任何处理，尤其是windows系统默认会将文本文件中的换行符转换为\r\n
        file << "ply\n";
        file << "format ascii 1.0\n";
        file << "element vertex " << points.size() << "\n";
        file << "property float x\n";
        file << "property float y\n";
        file << "property float z\n";
        file << "property float intensity\n";
        file << "property int classification\n";
        file << "end_header\n"; // 来自ai的ply格式说明
        for (const auto& p : points) {
            file << p.x << " " << p.y << " " << p.z << " "
                 << p.intensity << " " << p.classification << "\n";
        }
        file.close();
        return true;
    }
    
    else {
        std::cerr << "Unsupported export format: " << format << std::endl;
        return false;
    }
} // 按照要求导出点云数据文件，支持txt、csv和ply格式

void PointCloudData::addPoint(const Point3D& point){
    points.push_back(point);
    updateBounds(point.x, point.y, point.z);
}

void PointCloudData::addPoints(const std::vector<Point3D>& newPoints){
    for(const auto& point : newPoints){ //:表示迭代作用域是newPoints
        addPoint(point);
    }
}

int PointCloudData::getPointCount() const{
    return static_cast<int>(points.size());
} // static_cast<int>将size()返回的size_t类型转换为int类型，避免编译器警告
       

PointCloudData& PointCloudData::operator+=(const PointCloudData& other){
    PointCloudData result(*this); // 使用拷贝构造函数创建新对象
    for(const auto& point : other.points){
        result.addPoint(point); // addPoint会自动更新边界框
    }
    *this = result; 
    return *this;
}

PointCloudData PointCloudData::operator+(const PointCloudData& other) const {
    PointCloudData result(*this); // 使用拷贝构造函数创建新对象
    result += other; // 使用+=运算符重载函数
    return result;
}

Point3D& PointCloudData::operator[](int index){ // 通过下标访问点云数据中的点
    if(index < 0 || index >= static_cast<int>(points.size())){
        throw std::out_of_range("Index out of range");
    }
    return points[index];
} // 可修改

const Point3D& PointCloudData::operator[](int index) const{
    if(index < 0 || index >= static_cast<int>(points.size())){
        throw std::out_of_range("Index out of range");
    }
    return points[index];
} // 不可修改

 class voxelkey{
    public:
        int hx,hy,hz;
        voxelkey(int x,int y,int z):hx(x),hy(y),hz(z){}
        bool operator==(const voxelkey& other) const{
        return hx == other.hx && hy == other.hy && hz == other.hz;
    }
}; // 为点云计算准备的哈希表的键

class voxelkeyhash{
    public:
        size_t operator()(const voxelkey& key) const{
            return ((std::hash<int>()(key.hx) ^
            (std::hash<int>()(key.hy) << 1)) >> 1) ^
            (std::hash<int>()(key.hz) << 1);
    } // 为了避免算出的哈希值有碰撞（多个输入的输出值一样），使用复杂的位运算
};

 PointCloudData PointCloudData::voxelFilter(double voxelsize) const{
   if(points.empty()|| voxelsize <= 0) return *this; // 如果点云数据为空或者体素大小不合法，直接返回原始点云数据
   std::unordered_map<voxelkey, std::vector<Point3D>, voxelkeyhash> voxelMap; // 使用哈希表存储每个体素内的点云数据，此处必须用vector才能储存体素里的所有点
   for(const auto& point : points){
       int hx = static_cast<int>(std::floor(point.x / voxelsize));
       int hy = static_cast<int>(std::floor(point.y / voxelsize));
       int hz = static_cast<int>(std::floor(point.z / voxelsize));
       voxelkey key(hx, hy, hz);
       voxelMap[key].push_back(point); // 将点添加到对应体素的点云数据列表中
   }
   PointCloudData result("filtered" + id, name + " _voxel", path); // 生成待返回的结果
   for(const auto& p: voxelMap){
        const auto& pointsInVoxel = p.second; // 先在整个表里找到每个vector，再在vector里找到每个点
        double sumX = 0, sumY = 0, sumZ = 0, sumIntensity = 0;
        int count = static_cast<int>(pointsInVoxel.size());
        for(const auto& point : pointsInVoxel){
              sumX += point.x;
              sumY += point.y;
              sumZ += point.z;
              sumIntensity += point.intensity;
        }
        std::unordered_map<int,int> classCount; // 对分类标签求众数
        for(const auto& point : pointsInVoxel){
              classCount[point.classification]++; // 统计每个分类标签出现的次数，key是分类标签，value是出现次数，即key=分类标签值的value++
        }
        int majorityClass = -1;
        int maxCount = 0;
        for (const auto& p : classCount) {
            if (p.second > maxCount) {
            maxCount = p.second;
            majorityClass = p.first; // value最大的key即为众数的分类标签值
            }
        }
        result.addPoint(Point3D(sumX / count, sumY / count, sumZ / count, sumIntensity / count,majorityClass));
   }
    return result;
}

 PointCloudData PointCloudData::statisticalOutlierRemoval(int k, double stdDev) const{
    if(points.size() <2|| k <= 0 || stdDev <= 0) return *this; // 如果点云数据为空或者参数不合法，直接返回原始点云数据
    if(k >= static_cast<int>(points.size())) k = static_cast<int>(points.size()) - 1; // 如果k大于等于点云数据的数量，调整k为点云数据数量减1，避免越界
    std::vector<double> distances(points.size(), 0.0); 
    for(size_t i = 0; i < points.size(); ++i){
        std::vector<double> neighborDistances;
        neighborDistances.reserve(points.size() - 1);
        for(size_t j = 0; j < points.size(); ++j){
            if(i != j){
                neighborDistances.push_back(points[i].distanceTo(points[j])); //调用Point3D的distanceTo函数计算点i与其他点j的距离
            }
        }
        std::nth_element(neighborDistances.begin(), neighborDistances.begin() + k, neighborDistances.end()); // 使用nth_element算法找到第k个最近邻的距离，只关注前几个距离而不在乎内部顺序，比全排序效率高
        double sum=std::accumulate(neighborDistances.begin(), neighborDistances.begin() + k, 0.0); // 用函数计算前k个距离的和
        distances[i] = sum / k; // 计算点i的平均邻居距离
    }
    double mean = std::accumulate(distances.begin(), distances.end(), 0.0) / distances.size(); 
    double variance = 0.0;
    for(const auto& d : distances){
        variance += (d - mean) * (d - mean);
    }
    variance /= distances.size();
    double threshold = mean + stdDev * std::sqrt(variance); // 按公式计算距离阈值参数
    PointCloudData result("filtered" + id, name + " _sor", path); 
    for(size_t i = 0; i < points.size(); ++i){
        if(distances[i] <= threshold){ // 如果点i的平均邻居距离小于等于阈值，认为它是内点，保留在结果中
            result.addPoint(points[i]);
        }
    }
    return result;
 }
 
 double PointCloudData::getAverageHeight() const{
    if(points.empty()) return 0.0;
    double sum = 0.0;
    for(const auto& point : points){
        sum += point.z;
    }
    return sum / static_cast<double>(points.size());
}

double PointCloudData::getHeightRange() const{
    if(points.empty()) return 0.0;
    return maxZ - minZ;
}

void PointCloudData::printStatistics() const{
    std::cout << "PointCloudData: " << name << " (" << id << ")" << std::endl;
    std::cout << "Total Points: " << points.size() << std::endl;
    std::cout << "Bounds: [" 
              << minX << ", " << maxX << "] x [" 
              << minY << ", " << maxY << "] x [" 
              << minZ << ", " << maxZ << "]" << std::endl;
    std::cout << "Average Height: " << getAverageHeight() << std::endl;
    std::cout << "Height Range: " << getHeightRange() << std::endl;
}