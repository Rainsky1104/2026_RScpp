#pragma once
#include<vector>
#include<string>


namespace RemoteSensingAlgorithms {
    // 植被指数
    double calculateNDVI(double red, double nir){
        return (red-nir)/(red-nir+1e-9);
    };
    double calculateNDWI(double green, double nir){
        return (green-nir)/(green+nir+1e-9);
    };
    double calculateSAVI(double red, double nir, double L = 0.5){
        return 1.5*(nir-red)/(nir+red+L+1e-9);
    };
    double calculateEVI(double blue, double red, double nir){
        return 2.5*(nir-red)/(nir+6.0*red-7.5*blue+1.0+1e-9);
    };
    
    // 水体指数
    double calculateMNDWI(double green, double swir){
        return (green-swir)/(green+swir+1e-9);
    };
    double calculateAWEI(double blue, double green, double nir, double swir1, double swir2){
        return 4.0*(green-swir1)-(0.25*nir+2.75*swir2);
    };
    
    // 分类算法
    template<typename T>
    std::vector<int> kMeansClustering(const std::vector<T>& data, int k, int maxIter = 100){
        if (k <= 0 || data.empty()) throw std::invalid_argument("kMeansClustering: k must be > 0 and data not empty");
        const size_t n = data.size();
        if (k > (int)n) k = n;  // 每个点自成一类

        std::vector<int> labels(n, -1);
        std::vector<T> centroids(k);

        // 随机初始化中心：从数据中随机选取 k 个不同点
        std::random_device rd;
        std::mt19937 gen(rd()); // 随机数生成器
        std::uniform_int_distribution<size_t> dist(0, n - 1);
        for (int i = 0; i < k; ++i) {
            size_t idx = dist(gen);
            centroids[i] = data[idx];
        }

        for (int iter = 0; iter < maxIter; ++iter) {
            // 分配标签：计算每个点到各中心的距离（绝对差）
            bool changed = false;
            for (size_t i = 0; i < n; ++i) {
                double minDist = std::numeric_limits<double>::max();
                int best = 0;
                for (int j = 0; j < k; ++j) {
                    double d = fabs(static_cast<double>(data[i]) - static_cast<double>(centroids[j]));
                    if (d < minDist) {
                        minDist = d;
                        best = j;
                    }
                }
                if (labels[i] != best) {
                    labels[i] = best;
                    changed = true;
                }
            }
            if (!changed) break;

            // 更新中心：计算每个簇的均值
            std::vector<T> sums(k, T(0));
            std::vector<int> counts(k, 0);
            for (size_t i = 0; i < n; ++i) {
                sums[labels[i]] += data[i];
                counts[labels[i]]++;
            }
            for (int j = 0; j < k; ++j) {
                if (counts[j] > 0)
                    centroids[j] = sums[j] / counts[j];
            }
        }
        return labels;
        }
    }; // 仅处理一维数据，距离计算为绝对差，中心更新为均值
    
    // 图像处理
    template<typename T>
    std::vector<std::vector<T>> convolve(const std::vector<std::vector<T>>& image, 
                                const std::vector<std::vector<double>>& kernel){
                                     if (image.empty() || kernel.empty() || kernel[0].empty())
        throw invalid_argument("convolve: non-empty image and kernel required.");
    size_t imgRows = image.size();
    size_t imgCols = image[0].size();
    size_t kRows = kernel.size();
    size_t kCols = kernel[0].size();
    if (imgRows < kRows || imgCols < kCols)
        throw invalid_argument("convolve: kernel larger than image.");

    size_t outRows = imgRows - kRows + 1;
    size_t outCols = imgCols - kCols + 1;
    vector<vector<T>> output(outRows, vector<T>(outCols));

    for (size_t r = 0; r < outRows; ++r) {
        for (size_t c = 0; c < outCols; ++c) {
            double sum = 0.0;
            for (size_t kr = 0; kr < kRows; ++kr) {
                for (size_t kc = 0; kc < kCols; ++kc) {
                    sum += static_cast<double>(image[r + kr][c + kc]) * kernel[kr][kc];
                }
            } // 卷积计算，借用数学定义，等价于算两个矩阵的内积
            output[r][c] = static_cast<T>(sum);
        }
    }
    return output;
} // valid卷积不填充边缘
                            
    
    // 统计分析
    template<typename T>
    double calculateEntropy(const std::vector<T>& data){
    if (data.empty()) return 0.0;
    map<T, size_t> freq;
    for (const auto& val : data) freq[val]++;
    double entropy = 0.0;
    double n = static_cast<double>(data.size());
    for (const auto& p : freq) {
        double prob = p.second / n;
        entropy -= prob * log2(prob); // 计算信息熵公式用log2，单位是比特（bit），若用ln则单位是nat（自然单位）
    }
    return entropy; // 值越大表示数据分布越均匀，值越小表示数据分布越集中

    };
    
    template<typename T>
    std::vector<double> calculateHistogram(const std::vector<T>& data, int bins){
         if (data.empty() || bins <= 0) return std::vector<double>(bins, 0.0);
        auto [minIt, maxIt] = minmax_element(data.begin(), data.end());
        double minVal = static_cast<double>(*minIt);
        double maxVal = static_cast<double>(*maxIt);
        if (minVal == maxVal) { 
        std::vector<double> hist(bins, 0.0);
        hist[0] = static_cast<double>(data.size());
        return hist;
        }// 所有值相同，返回第一个 bin 有全部计数
    double binWidth = (maxVal - minVal) / bins; // 计算每个 bin 的宽度
    std::vector<double> hist(bins, 0.0);
    for (const auto& val : data) {
        double dVal = static_cast<double>(val);
        int idx = static_cast<int>((dVal - minVal) / binWidth); // 计算值所在的 bin 索引
        if (idx < 0) idx = 0;
        if (idx >= bins) idx = bins - 1;
        hist[idx]++;
    }
    return hist;
    };