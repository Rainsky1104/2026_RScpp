#pragma once
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <string>

template<typename T>
class Pixel {
private:
    T red, green, blue, nir;     // 多光谱波段
    T thermal;                    // 热红外波段
    double timestamp;             // 采集时间戳
    mutable int accessCount;      // 访问计数（mutable演示）
    
public:
    // 构造函数（支持默认参数）
    Pixel(T r = 0, T g = 0, T b = 0, T n = 0, T t = 0, double ts = 0): 
        red(r), green(g), blue(b), nir(n), thermal(t), timestamp(ts), accessCount(0){
    };
    
    // 拷贝构造函数
    Pixel(const Pixel<T>& other){
        red = other.red;
        green = other.green;
        blue = other.blue;
        nir = other.nir;
        thermal = other.thermal;
        timestamp = other.timestamp;
        accessCount = 0;
    };
    
    // 析构函数
    ~Pixel(){};
    
    // 赋值运算符
    Pixel<T>& operator=(const Pixel<T>& other){
        if(this != &other){ // 防止自赋值
            red = other.red;
            green = other.green;
            blue = other.blue;
            nir = other.nir;
            thermal = other.thermal;
            timestamp = other.timestamp;
            accessCount = 0; // 访问计数不复制，重置为0
        }
        return *this;
    };
    
    // 算术运算符重载
    Pixel<T> operator+(const Pixel<T>& other) const{
        return Pixel<T>(this->getRed() + other.red, this->getGreen() + other.green, this->getBlue() + other.blue, 
                        this->getNir() + other.nir, this->getThermal() + other.thermal, timestamp);
    }; //必须调用getRed等访问器来获取值，以便更新访问计数

    Pixel<T> operator-(const Pixel<T>& other) const{
        return Pixel<T>(this->getRed() - other.red, this->getGreen() - other.green, this->getBlue() - other.blue, 
                        this->getNir() - other.nir, this->getThermal() - other.thermal, timestamp);
    };

    Pixel<T> operator*(T factor) const{
        return Pixel<T>(this->getRed() * factor, this->getGreen() * factor, this->getBlue() * factor, 
                        this->getNir() * factor, this->getThermal() * factor, timestamp);
    };

    Pixel<T> operator/(T divisor) const{
        return Pixel<T>(this->getRed() / divisor, this->getGreen() / divisor, this->getBlue() / divisor, 
                        this->getNir() / divisor, this->getThermal() / divisor, timestamp);
    };

    Pixel<T>& operator+=(const Pixel<T>& other){
        red += other.red;
        green += other.green;
        blue += other.blue;
        nir += other.nir;
        thermal += other.thermal;
        timestamp = other.timestamp;
        accessCount = 0;
        return *this;
    };

    Pixel<T>& operator-=(const Pixel<T>& other){
        red -= other.red;
        green -= other.green;
        blue -= other.blue;
        nir -= other.nir;
        thermal -= other.thermal;
        timestamp = other.timestamp;
        accessCount = 0;
        return *this;
    };

    // 比较运算符
    bool operator==(const Pixel<T>& other) const{
        return red == other.red && green == other.green && blue == other.blue && 
               nir == other.nir && thermal == other.thermal;
    };

    bool operator!=(const Pixel<T>& other) const{
        return !(*this == other);
    };

    
    // 类型转换运算符
    operator T() const { return getBrightness(); }  // 转换为亮度值
    operator std::string() const { 
    return "Pixel(R:" + std::to_string(red) + ", G:" + std::to_string(green) + ", B:" + std::to_string(blue) +
           " N:" + std::to_string(nir) + ", T:" + std::to_string(thermal) + " time:" + std::to_string(timestamp) + ")";
}

    // 下标运算符（访问波段）
    T& operator[](int band){
        switch(band){
            case 0: return red;
            case 1: return green;
            case 2: return blue;
            case 3: return nir;
            case 4: return thermal;
            default: throw std::out_of_range("Band index out of range");
        } // 自定义波段的访问代码
    };

    const T& operator[](int band) const{
        switch(band){
            case 0: return red;
            case 1: return green;
            case 2: return blue;
            case 3: return nir;
            case 4: return thermal;
            default: throw std::out_of_range("Band index out of range");
        }
    }; // const版本的下标运算符，返回const引用以防止修改
    
    // 友元函数（流输入输出）
    friend std::ostream& operator<<(std::ostream& os, const Pixel<T>& pixel) {
        os << "R:" << pixel.red << " G:" << pixel.green 
           << " B:" << pixel.blue << " NIR:" << pixel.nir
           << " Thermal:" << pixel.thermal;
        return os;
    }

    // 遥感指数计算
    double getNDVI() const{
        return (getNir() - getRed()) / (getNir() + getRed() + 1e-7); // 加小常数避免除零
    };       // 归一化植被指数

    double getNDWI() const{       // 归一化水体指数
        return (getGreen() - getNir()) / (getGreen() + getNir() + 1e-7);
    };

    double getSAVI() const{       // 土壤调节植被指数
        return (1.5 * (getNir() - getRed())) / (getNir() + getRed() + 0.5);
    };

    double getBrightness() const{  // 亮度值
        return (getRed() + getGreen() + getBlue()+ getNir() + getThermal()) / 5.0;
    };

    double getTemperature() const{  // 亮温（基于热红外）
        const double K1 = 607.76; 
        const double K2 = 1260.56;// 示例常数，以 Landsat 5/7 为例,不同传感器的常数不同
        double thermal_val = static_cast<double>(getThermal());
        if (thermal_val <= 0) return 0.0; // 无效值
        double temp = K2 / std::log(K1 / thermal_val + 1.0);
        return temp - 273.15; 
    }  // 简化处理，实际应用中需要更复杂的转换公式
    
    
    // 获取器
    T getRed() const { accessCount++; return red; }
    T getGreen() const { accessCount++; return green; }
    T getBlue() const { accessCount++; return blue; }
    T getNir() const { accessCount++; return nir; }
    T getThermal() const { accessCount++; return thermal; }
    int getAccessCount() const { return accessCount; }
    
    // 设置器
    void setRed(T r) { red = r; }
    void setGreen(T g) { green = g; }
    void setBlue(T b) { blue = b; }
    void setNir(T n) { nir = n; }
    void setThermal(T t) { thermal = t; }
};