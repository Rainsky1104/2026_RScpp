#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <opencv2/core.hpp>
#include "Pixel.h"
#include "DataObject.h"
#include "PointCloudData.h"
#include "SatelliteImage.h"
#include "DataManager.h"
#include "DataExporter.h"
#include "Algorithms.h"
#include <stdexcept>
#include <cmath>
#include <numeric>
#include <thread>
#include <windows.h>
#undef min
#undef max // 避免windows.h定义的min和max宏与std::numeric_limits冲突
#include <limits>


int main() {
    SetConsoleOutputCP(CP_UTF8);
    // 启用 OpenCV 多线程加速（根据 CPU 自动调整）
    cv::setNumThreads(std::thread::hardware_concurrency());

    std::cout << "========== 遥感数据智能处理与分析系统 ==========\n\n";

    // ========== 1. 加载真实数据 ==========
    std::cout << ">>> 1. 数据加载\n";
    
    
    SatelliteImage img1 = SatelliteImage::loadFromJPG("IMG001", "樱顶1号影像", "data\\photo1.jpg");
    SatelliteImage img2 = SatelliteImage::loadFromJPG("IMG002", "樱顶2号影像", "data\\photo2.jpg");
  

    PointCloudData pc1 = PointCloudData::loadFromPLY("PC001", "点云1号", "data\\cloud_and_poses.ply");
    // 额外生成一个随机点云,因为合并数据管理器等操作需要至少两个对象
    PointCloudData pc2("PC002", "随机点云", "./data");
    for (int i = 0; i < 500; ++i) {
        Point3D p(rand() % 100 / 10.0, rand() % 100 / 10.0, rand() % 50 / 10.0,
                  rand() % 255 / 255.0, rand() % 5);
        pc2.addPoint(p);
    }

    // ========== 2. 数据管理 ==========
    std::cout << "\n>>> 2. DataManager 创建与数据添加\n";
    DataManager<SatelliteImage> imgManager;
    imgManager.addData(img1);
    imgManager.addData(img2);

    DataManager<PointCloudData> pcManager;
    pcManager.addData(pc1);
    pcManager.addData(pc2);

    std::cout << "影像管理器对象数: " << imgManager.getCount() << std::endl;
    std::cout << "点云管理器对象数: " << pcManager.getCount() << std::endl;
    imgManager.printStatistics();
    pcManager.printStatistics();

    // ========== 3. 基本信息展示 ==========
    std::cout << "\n>>> 3. 数据对象基本信息\n";
    img1.display();
    std::cout << std::endl;
    pc1.display();

    // ========== 4. 影像处理与遥感指数 ==========
    std::cout << "\n>>> 4. 影像处理与遥感指数\n";
    
    // 4.1 OpenCV 加速的高斯模糊
    std::cout << "应用高斯模糊 (sigma=1.5)...\n";
    auto start = std::chrono::high_resolution_clock::now();
    img1.applyGaussianBlur(1.5);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "耗时: " << std::chrono::duration<double, std::milli>(end - start).count() << " ms\n";

    // 4.2 遥感指数计算（JPG 缺少 NIR 波段，按拟合公式计算NDVI等数值时结果可能有所偏差）
    std::cout << "\n计算 NDVI...\n";
    try {
        std::vector<std::vector<double>> ndvi = img1.calculateNDVI();
        double sum = 0.0;
        for (auto& row : ndvi) for (double v : row) sum += v;
        double meanNDVI = sum / (ndvi.size() * ndvi[0].size());
        std::cout << "平均 NDVI: " << meanNDVI << std::endl;

        std::cout << "计算 NDWI...\n";
        std::vector<std::vector<double>> ndwi = img1.calculateNDWI();
        sum = 0.0;
        for (auto& row : ndwi) for (double v : row) sum += v;
        std::cout << "平均 NDWI: " << sum / (ndwi.size() * ndwi[0].size()) << std::endl;

        std::cout << "计算 SAVI (L=0.5)...\n";
        std::vector<std::vector<double>> savi = img1.calculateSAVI(0.5);
        sum = 0.0;
        for (auto& row : savi) for (double v : row) sum += v;
        std::cout << "平均 SAVI: " << sum / (savi.size() * savi[0].size()) << std::endl;

    } catch (const std::runtime_error& e) {
        std::cerr << "指数计算错误: " << e.what() << std::endl;
    }

    // 4.3 亮温计算（同样受波段限制）
    if (img1.getBands() >= 5) {
        std::cout << "计算亮温...\n";
        std::vector<std::vector<double>> temp = img1.calculateTemperature();
        double sum = 0.0;
        double minTemp = std::numeric_limits<double>::max();
        double maxTemp = -std::numeric_limits<double>::max();
        size_t count = 0;
        for (const auto& row : temp) {
            for (double t : row) {
                sum += t;
                if (t < minTemp) minTemp = t;
                if (t > maxTemp) maxTemp = t;
                ++count;           
            }
        }
        double avgTemp = (count > 0) ? sum / count : 0.0;
        std::cout << "  平均亮温: " << avgTemp << " °C\n";
        std::cout << "  亮温范围: [" << minTemp << ", " << maxTemp << "] °C\n";
    }

    // ========== 5. 点云处理 ==========
    std::cout << "\n>>> 5. 点云处理\n";
    std::cout << "原始点云 " << pc1.getId() << " 点数: " << pc1.getPointCount() << std::endl;

    // 体素滤波
    PointCloudData pcVoxel = pc1.voxelFilter(0.3);
    std::cout << "体素滤波后点数: " << pcVoxel.getPointCount() << std::endl;

    // 统计离群点去除
    PointCloudData pcSOR = pcVoxel.statisticalOutlierRemoval(10, 1.0);
    std::cout << "SOR 滤波后点数: " << pcSOR.getPointCount() << std::endl;

    // 点云统计
    std::cout << "原始点云平均高度: " << pc1.getAverageHeight() << std::endl;
    std::cout << "高度范围: " << pc1.getHeightRange() << std::endl;

    // ========== 6. DataManager 高级操作 ==========
    std::cout << "\n>>> 6. DataManager 排序、过滤与合并\n";

    // 按大小排序（降序）
    auto sortedImgs = imgManager.sortBySize(false);
    std::cout << "按大小降序排列的影像 ID: ";
    for (auto& img : sortedImgs) std::cout << img.getId() << " ";
    std::cout << std::endl;

    // 过滤：云量小于10%的影像（JPG默认云量为0，故全部保留）
    auto clearImgs = imgManager.filter([](const SatelliteImage& img) {
        return img.getCloudCover() < 10.0;
    });
    std::cout << "云量 < 10% 的影像数量: " << clearImgs.size() << std::endl;

    // 合并两个点云管理器
    DataManager<PointCloudData> pcManager2;
    pcManager2.addData(pcSOR);
    DataManager<PointCloudData> mergedPC = pcManager + pcManager2;
    std::cout << "合并后点云管理器对象数: " << mergedPC.getCount() << std::endl;


    // ========== 7. 遥感算法库应用 ==========
    std::cout << "\n>>> 7. 遥感算法库应用\n";

    // 对点云高度值进行 K-means 聚类
    std::vector<double> heights;
    for (const auto& p : pc1.getPoints()) heights.push_back(p.z);
    auto labels = RemoteSensingAlgorithms::kMeansClustering(heights, 3, 50);
    std::cout << "点云高度 K-means 聚类完成 (类别数=3)\n";

    // 计算高度的信息熵
    double entropy = RemoteSensingAlgorithms::calculateEntropy(heights);
    std::cout << "点云高度信息熵: " << entropy << " bits\n";

    // 计算直方图
    auto hist = RemoteSensingAlgorithms::calculateHistogram(heights, 10);
    std::cout << "高度直方图 (10 bins): ";
    for (double h : hist) std::cout << h << " ";
    std::cout << std::endl;

    // ========== 8. 数据导出 ==========
    std::cout << "\n>>> 8. 数据导出\n";
    DataExporter exporter("./output");

    // 导出单个点云为 CSV
    if (exportToCSV<PointCloudData>(pc1, "./output/pc1_export.csv"))
        std::cout << "点云 CSV 导出成功\n";

    // 批量导出影像统计为 JSON
    std::vector<SatelliteImage> imgs = {img1, img2};
    int exported = exporter.batchExport(imgs, "json");
    std::cout << "批量导出 " << exported << " 个 JSON 文件\n";

    // 导出体素滤波后的点云
    exportToCSV<PointCloudData>(pcVoxel, "./output/pc1_voxel.csv");

    exporter.printExportStats();

    std::cout << "\n========== 系统演示结束 ==========\n";
    return 0;
}