#pragma once
#include <vector>
#include <map>
#include <set> 
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <string>
#include <iostream>



template<typename T>
class DataManager {
private:
    std::vector<T> dataList;                     // 数据列表
    std::map<std::string, std::vector<T>> categoryMap;     // 按类别分组
    std::set<T> uniqueData;                      // 唯一数据集合（基于operator<）
    mutable int operationCount;             // 操作计数

    // 私有辅助：添加单个数据时更新分类映射和唯一集合
    void updateCategory(const T& data) {
        std::string type = data.getType();       // 调用 DataObject 的 getType()
        categoryMap[type].push_back(data);
        uniqueData.insert(data);            // set 自动去重，依赖 operator<, 需要 T 定义比较运算符
    }

    // 私有辅助：从 dataList 完全重建索引
    void rebuildIndex() {
        categoryMap.clear();
        uniqueData.clear();
        for (const auto& item : dataList)
            updateCategory(item);
    }

public:
    DataManager() : operationCount(0) {}

    ~DataManager() {} // 默认析构函数，vector和map会自动清理资源

    // 禁止拷贝（单例模式可选，保留声明但删除实现）
    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete; //防止巨大数据被复制，鼓励用merge或移动语义来组合数据管理器
    //=delete禁止编译器生成默认的拷贝构造函数和赋值运算符，防止误用


    void addData(const T& data) {
        dataList.push_back(data);
        updateCategory(data);
        ++operationCount;
    }

    void removeData(const std::string& id) {
        auto it = std::find_if(dataList.begin(), dataList.end(),
                          [&id](const T& d) { return d.getId() == id; });
        if (it != dataList.end()) {
            dataList.erase(it);
            rebuildIndex();   //调用私有函数刷新索引，保持分类映射的正确性
            ++operationCount;
        }
    }

    T findData(const std::string& id) const {
        for (const auto& d : dataList)
            if (d.getId() == id)
                return d;
        throw std::runtime_error("Data with ID " + id + " not found.");
    }
    rebuildIndex();
    bool contains(const std::string& id) const {
        for (const auto& d : dataList)
            if (d.getId() == id)
                return true;
        return false;
    }

    void clear() {
        dataList.clear();
        categoryMap.clear();
        uniqueData.clear();
        operationCount = 0;
    }

   

    void addDataList(const std::vector<T>& newData) {
        for (const auto& d : newData)
            addData(d);                    // 复用 addData，自动更新索引和计数
    }

    std::vector<T> getDataByType(const std::string& type) const {
        auto it = categoryMap.find(type);
        if (it != categoryMap.end())
            return it->second;
        return std::vector<T>();
    } // 根据类型返回数据列表，如果类型不存在返回空列表

   

    std::vector<T> sortBySize(bool ascending = true) const {
        std::vector<T> sorted = dataList;
        std::sort(sorted.begin(), sorted.end(),
             [ascending](const T& a, const T& b) {
                 return ascending ? (a.getSize() < b.getSize())
                                  : (a.getSize() > b.getSize());
             });
        return sorted;
    }

    std::vector<T> sortByName() const {
        std::vector<T> sorted = dataList;
        std::sort(sorted.begin(), sorted.end(),
             [](const T& a, const T& b) { return a.getName() < b.getName(); });
        return sorted;
    }

    std::vector<T> sortByTime() const {
        std::vector<T> sorted = dataList;
        std::sort(sorted.begin(), sorted.end(),
             [](const T& a, const T& b) { return a.getCreateTime() < b.getCreateTime(); });
        return sorted;
    } 
    //三种排序方法，分别按大小、名称和创建时间排序，返回新的排序后的列表，不修改原列表



    std::vector<T> filter(std::function<bool(const T&)> condition) const {
        std::vector<T> result;
        for (const auto& item : dataList)
            if (condition(item))
                result.push_back(item);
        return result;
    }



    double getTotalSize() const {
        double total = 0.0;
        for (const auto& d : dataList)
            total += d.getSize();
        return total;
    }

    int getCount() const {
        return static_cast<int>(dataList.size());
    }

    std::map<std::string, int> getTypeStatistics() const {
        std::map<std::string, int> stats;
        for (const auto& pair : categoryMap)
            stats[pair.first] = static_cast<int>(pair.second.size());
        return stats;
    }

    void printStatistics() const {
        std::cout << "DataManager Statistics:\n";
        std::cout << "  Total objects: " << getCount() << "\n";
        std::cout << "  Total size (MB): " << getTotalSize() << "\n";
        auto stats = getTypeStatistics();
        for (const auto& p : stats)
            std::cout << "  " << p.first << ": " << p.second << "\n";
        std::cout << "  Operations performed: " << operationCount << "\n";
    }

    // 运算符重载：支持索引访问和合并操作

    T& operator[](int index) {
        if (index < 0 || index >= static_cast<int>(dataList.size()))
            throw std::out_of_range("DataManager: index out of range");
        return dataList[index];
    }

    const T& operator[](int index) const {
        if (index < 0 || index >= static_cast<int>(dataList.size()))
            throw std::out_of_range("DataManager: index out of range");
        return dataList[index];
    }

    DataManager<T> operator+(const DataManager<T>& other) const {
        DataManager<T> result;
        result.addDataList(this->dataList);
        result.addDataList(other.dataList);
        return result;
    }

    DataManager<T>& operator+=(const DataManager<T>& other) {
        addDataList(other.dataList);
        return *this;
    }

   // 迭代器支持，允许范围for循环和标准算法使用

    typename std::vector<T>::iterator begin() { return dataList.begin(); }
    typename std::vector<T>::iterator end()   { return dataList.end(); }
    typename std::vector<T>::const_iterator begin() const { return dataList.begin(); }
    typename std::vector<T>::const_iterator end()   const { return dataList.end(); }

     // 模板类实现，管理任意类型的数据对象，提供多个功能，支持迭代器和运算符重载，使用私有辅助函数维护索引和分类映射，确保数据管理的高效性和一致性

    template<typename Func>
    void forEach(Func func) const {
        for (const auto& item : dataList)
            func(item);
    }

    template<typename ResultType>
    std::vector<ResultType> transform(std::function<ResultType(const T&)> func) const {
        std::vector<ResultType> result;
        result.reserve(dataList.size());
        for (const auto& item : dataList)
            result.push_back(func(item));
        return result;
    }

   

    static DataManager<T> merge(const DataManager<T>& a, const DataManager<T>& b) {
        DataManager<T> merged;
        merged.addDataList(a.dataList);
        merged.addDataList(b.dataList);
        return merged;
    }
}; 