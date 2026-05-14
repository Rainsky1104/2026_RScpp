#pragma once
#include <string>
#include <vector>
#include "DataObject.h"
#include "PointCloudData.h"   // 需要访问 PointCloudData 的 getPoints() 等成员
#include "SatelliteImage.h"
#include <fstream>
#include <sstream>
#include <iostream>


// 前向声明可打破循环依赖，也加快编译速度（在下方才实现）
template<typename T>
bool exportToCSV(const DataObject& data, const std::string& filename);

template<typename T>
bool exportToJSON(const DataObject& data, const std::string& filename);

class DataExporter {
private:
    std::string exportPath;
    int exportCount;

public:
    DataExporter(const std::string& path) : exportPath(path), exportCount(0) {}
    ~DataExporter() = default;

    // 批量导出
    template<typename T>
    int batchExport(const std::vector<T>& dataList, const std::string& format) {
        int count = 0;
        for (const auto& data : dataList) {
            std::string filename = exportPath + "/" + data.getId() + "_export." + format;
            bool success = false;
            if (format == "csv")
                success = exportToCSV<T>(data, filename);
            else if (format == "json")
                success = exportToJSON<T>(data, filename);
            else {
                std::cerr << "Unsupported batch format: " << format << std::endl; // 报错不支持的格式
                continue;
            }
            if (success) ++count;
        }
        exportCount += count;
        return count;
    }

    void printExportStats() const{
        std::cout << "Total exports: " << exportCount << "\n";
        std::cout << "Export path: " << exportPath << "\n";
    };
};



// 防止json文件中字符串引号和反斜杠导致格式错误，进行转义处理
static inline std::string jsonEscape(const std::string& s) { // static inline函数在头文件中定义，允许在多个翻译单元中使用而不会引起链接错误
    std::string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else out += c;
    }
    return out;
}

// 导出CSV
template<typename T>
bool exportToCSV(const DataObject& data, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    // 友元可直接访问 data.id, data.name, data.metadata 等
    file << "ID,Name,Path,Size,Type\n";
    file << data.id << "," << data.name << "," << data.path << ","
         << data.size << "," << data.getType() << "\n\n";

    file << "Metadata\n";
    for (const auto& kv : data.metadata) {
        file << kv.first << "," << kv.second << "\n";
    }
    file << "\n";

    // 根据实际派生类型导出具体数据
    const PointCloudData* pc = dynamic_cast<const PointCloudData*>(&data); // dynamic_cast确认一个基类指针/引用是否实际上指向某个派生类对象，如果是，就安全地转换为派生类指针/引用，从而访问派生类独有的成员
    if (pc) {
        file << "PointCloud Data\n";
        file << "X,Y,Z,Intensity,Classification\n";
        for (const auto& p : pc->getPoints()) {
            file << p.x << "," << p.y << "," << p.z << ","
                 << p.intensity << "," << p.classification << "\n";
        }
    } else {
        const SatelliteImage* img = dynamic_cast<const SatelliteImage*>(&data);
        if (img) {
            file << "Satellite Image Summary\n";
            file << "Width,Height,Bands,Sensor,CloudCover\n";
            file << img->getWidth() << "," << img->getHeight() << "," << img->getBands() << ","
                 << img->getSensorType() << "," << img->getCloudCover() << "\n";
        } else {
            file << "No specific data export available.\n";
        }
    }
    file.close();
    return true;
}

// 导出JSON
template<typename T>
bool exportToJSON(const DataObject& data, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    std::ostringstream json;
    json << "{\n";
    json << "  \"id\": \"" << jsonEscape(data.id) << "\",\n";
    json << "  \"name\": \"" << jsonEscape(data.name) << "\",\n";
    json << "  \"path\": \"" << jsonEscape(data.path) << "\",\n";
    json << "  \"size\": " << data.size << ",\n";
    json << "  \"type\": \"" << data.getType() << "\",\n";
    json << "  \"metadata\": {\n";
    bool firstMeta = true;
    for (const auto& kv : data.metadata) {
        if (!firstMeta) json << ",\n";
        json << "    \"" << jsonEscape(kv.first) << "\": \"" << jsonEscape(kv.second) << "\"";
        firstMeta = false;
    }
    json << "\n  },\n";

    const PointCloudData* pc = dynamic_cast<const PointCloudData*>(&data);
    if (pc) {
        json << "  \"pointCount\": " << pc->getPointCount() << ",\n";
        json << "  \"bounds\": {\n";
        json << "    \"minX\": " << pc->getMinX() << ",\n";
        json << "    \"maxX\": " << pc->getMaxX() << ",\n";
        json << "    \"minY\": " << pc->getMinY() << ",\n";
        json << "    \"maxY\": " << pc->getMaxY() << ",\n";
        json << "    \"minZ\": " << pc->getMinZ() << ",\n";
        json << "    \"maxZ\": " << pc->getMaxZ() << "\n";
        json << "  }\n";
    } else {
        const SatelliteImage* img = dynamic_cast<const SatelliteImage*>(&data);
        if (img) {
            json << "  \"image\": {\n";
            json << "    \"width\": " << img->getWidth() << ",\n";
            json << "    \"height\": " << img->getHeight() << ",\n";
            json << "    \"bands\": " << img->getBands() << ",\n";
            json << "    \"sensor\": \"" << jsonEscape(img->getSensorType()) << "\",\n";
            json << "    \"cloudCover\": " << img->getCloudCover() << "\n";
            json << "  }\n";
        }
    } // 不确定是Dataobject的哪一个派生类故分开讨论
    json << "}\n";
    file << json.str();
    file.close();
    return true;
}