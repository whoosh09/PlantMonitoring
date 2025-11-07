# 🌿 Smart Plant Monitoring System (NodeMCU + NETPIE + Telegram)

An IoT project that monitors **soil moisture, temperature, and humidity** using **NodeMCU (ESP8266)** and **DHT22**. It automatically alerts you on **Telegram** and controls a **servo shade** to protect your plant when needed.

![projectFinal](https://github.com/user-attachments/assets/17cc3821-6285-4958-b6a2-2fe88d239ae4)


---

## 🚀 Features
* 🌡️ Reads **temperature & humidity** from **DHT22**.
* 🌱 Detects **soil moisture** using an analog sensor.
* 🤖 Sends **Telegram alerts** (dry, perfect, or wet).
* 💡 **RGB LED** indicator for plant condition.
* ☁️ Publishes data to **NETPIE** using MQTT.
* ⚙️ Smooth **servo movement** for shade control.

---

## 🔧 Setup
1. Copy `secrets_example.h` → `secrets.h`.
2. Fill in your **WiFi**, **NETPIE**, and **Telegram** credentials.
3. Open `SmartPlant.ino` in Arduino IDE and upload it to your NodeMCU board.
4. View live data on the **NETPIE dashboard** and get **Telegram alerts** instantly.

---
## 🌐 NETPIE Setup (Required)

To use this project, you need a **NETPIE (broker.netpie.io)** account for MQTT communication and dashboard visualization.

### 🧩 1. Create a NETPIE Account
* Go to [https://broker.netpie.io](https://broker.netpie.io).
* Sign up and log in to your **Developer Console**.

---

### ⚙️ 2. Create a New Device
1. In the dashboard, click **Create Device** → choose a name (e.g., `SoilMoist`).
2. Copy the following credentials:
    * **Client ID**
    * **Username**
    * **Password**
3. Paste them into your local `secrets.h` file:
    ```c
    const char* MQTT_CLIENT_ID = "your_client_id";
    const char* MQTT_USERNAME  = "your_username";
    const char* MQTT_PASSWORD  = "your_password";
    ```

### ✨ 3. Build Your Dashboard
Go to the Dashboard tab and click **“+ Widget”** to start adding widgets. Configure each widget with the corresponding Topic/Configuration string for easy copy-paste:

| Widget Type | Display Name (Example) | Topic/Configuration String | Control Action (If applicable) |
| :--- | :--- | :--- | :--- |
| **Gauge** 🌡️ | Temperature Gauge | `#["SoilMoist"]["shadow"]["temperature"]` | N/A |
| **Gauge** 💧 | Humidity Gauge | `#["SoilMoist"]["shadow"]["humidity"]` | N/A |
| **Gauge** 🌱 | Moisture Gauge | `#["SoilMoist"]["shadow"]["moisture"]` | N/A |
| **Text Display** 🧾 | Moisture Value | `#["SoilMoist"]["shadow"]["moisture"]` | N/A |
| **Chart** 📊 | Trends Over Time | `#["SoilMoist"]["feed"]` | N/A |
| **Slider** ⚙️ | Servo Shade Control | `#["SoilMoist"]["shadow"]["shade"]` | `On Slide Action: #["SoilMoist"].publishMsg("shade", value)` |
| **On/Off Toggle** 🌤️ | Servo Shade Toggle | `(#["SoilMoist"]["shadow"]["shade"] == "0") ? true : false` | `ON Action: #["SoilMoist"].publishMsg("shade", "0")`<br>`OFF Action: #["SoilMoist"].publishMsg("shade", "180")` |

> 📘 For more details, refer to the official [NETPIE Documentation](https://docs.netpie.io/en/)

---
## 🧰 Hardware
| Component | Description |
| :--- | :--- |
| **NodeMCU ESP8266** | Main microcontroller (WiFi enabled) |
| **DHT22 Sensor** | Measures temperature & humidity |
| **Soil Moisture Sensor** | Detects soil water levels |
| **Servo Motor** | Controls plant shade |
| **RGB LED** | Indicates moisture level visually |

---

## 🖼️ Example Dashboard
<img width="1873" height="952" alt="netpie" src="https://github.com/user-attachments/assets/acfa9d03-e41f-48f5-b922-0b510887e7d4" />


---
## 📜 License
MIT License © 2025 whoosh09
See [LICENSE](./LICENSE) for details.
