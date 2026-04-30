#include "DataObject.h"
#include <iostream>
#include <stdexcept>
int DataObject::totalObjects = 0; 
// 静态成员初始化(不能加static因为类定义中才能写，否则变为全局函数)

DataObject::DataObject(const std::string& id, const std::string& name, const std::string& path)
    : id(id), name(name), path(path), size(0.0), createTime(time(nullptr)), accessCount(0) {
    totalObjects++;
} // metadata是类内成员，自动初始化

DataObject::~DataObject() {
    totalObjects--;
} // 构造与析构函数

DataObject::DataObject(const DataObject& other)
    : id(other.id), name(other.name), path(other.path), size(other.size), createTime(other.createTime), metadata(other.metadata), accessCount(0) {
    totalObjects++;
} // 拷贝构造函数(深拷贝)

DataObject& DataObject::operator=(const DataObject& other){
    if (this != &other) { // 防止自赋值
        id = other.id;
        name = other.name;
        path = other.path;
        size = other.size;
        metadata = other.metadata;
        createTime = other.createTime;
        accessCount = 0;
    }
    return *this;
}
// 拷贝运算符（拷贝中accessCount保持为0，即每个新对象都从0开始计数）
// 纯虚函数不需要在基类里实现，但强制在派生类里实现

void DataObject::addMetadata(const std::string& key, const std::string& value) {
    metadata[key] = value;
}

std::string DataObject::getMetadata(const std::string& key) const{
    auto it = metadata.find(key); // 迭代器自动判断类型
    if (it != metadata.end()) {
        return it->second; // map的元素是pair类型，first是key，second是value
    }
    throw std::runtime_error("Metadata key not found: " + key);//throw用于报错(向上抛出但不打印），runtime表示是运行时的问题
}

bool DataObject::hasMetadata(const std::string& key) const {
    return metadata.find(key) != metadata.end();
}

void DataObject::removeMetadata(const std::string&key){
    metadata.erase(key);
} // 元数据操作（依靠map自带函数)

int DataObject::getTotalObjects(){
    return totalObjects;
}

void DataObject::resetTotalObjects(){
    totalObjects = 0;
} // 静态成员函数，用于操作totalObjects，同上解释不能加static

void DataObject::updateSize(double newSize){
    if(newSize<0){ // 检查size非负
        throw std::invalid_argument("Size cannot be negative");//invalid_argument表示参数不合法
    }
    size = newSize;
}

void DataObject::setPath(const std::string&newpath){
    path = newpath;
}

