# 🚑 Patient Monitoring & Room Health System 💙

This project is an **IoT-based Patient Monitoring System** that tracks **room conditions** (temperature, humidity, air quality, CO levels) and **vital health parameters** (heart rate, SpO₂, body temperature). It features an **emergency alert system** and a **web-based dashboard** for real-time monitoring.

## 📌 Features
✅ Monitors **room temperature & humidity** 🌡️  
✅ Tracks **heart rate & SpO₂ levels** ❤️  
✅ Measures **air quality & CO levels** 🌍  
✅ **Emergency alert button** with a buzzer 🚨  
✅ **Auto fan control** based on air quality  
✅ **Humidifier control** based on humidity levels  
✅ **Web dashboard** for real-time monitoring 🌐  

---

## 🛠️ Hardware Requirements
- **ESP32** (WiFi-enabled microcontroller)
- **MAX30100** (Pulse Oximeter for BPM & SpO₂)
- **MLX90614** (Infrared Temperature Sensor)
- **DHT11** (Temperature & Humidity Sensor)
- **MQ-7 & MQ-135** (CO & Air Quality Sensors)
- **Buzzer** (For emergency alerts)
- **Fan & Humidifier** (Controlled by ESP32)
- **Push Button** (Triggers emergency mode)

---

## 📦 Installation
### **1️⃣ Clone the Repository**
```bash
git clone https://github.com/pretom-das/patient-monitoring.git
cd patient-monitoring
