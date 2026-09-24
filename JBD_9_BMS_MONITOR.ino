#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <NimBLEDevice.h>

const char* AP_SSID = "JBD-MONITOR";
const char* AP_PASS = "⁨12345678⁩";

WebServer server(80);

struct BleDevice {
  String name;
  String address;
  int rssi;
};

BleDevice devices[40];
int deviceCount = 0;

String pageStart(const char* title)
{
  String s;

  s += "<!DOCTYPE html>";
  s += "<html lang='ru'>";
  s += "<head>";
  s += "<meta charset='UTF-8'>";
  s += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  s += "<title>";
  s += title;
  s += "</title>";

  s += "<style>";
  s += "body{font-family:Arial;background:#101820;color:#fff;margin:15px}";
  s += "h1,h2{color:#00d4ff}";
  s += "button{background:#1976d2;color:white;border:0;";
  s += "padding:14px 20px;border-radius:7px;font-size:16px}";
  s += "table{width:100%;border-collapse:collapse;margin-top:20px}";
  s += "th,td{border-bottom:1px solid #444;padding:9px;text-align:left}";
  s += ".good{color:#00e676}";
  s += ".warn{color:#ffca28}";
  s += "</style>";

  s += "</head>";
  s += "<body>";

  return s;
}

String pageEnd()
{
  return "</body></html>";
}


void scanBLE()
{
  deviceCount = 0;

  NimBLEScan* scan = NimBLEDevice::getScan();

  scan->clearResults();

  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(80);

  Serial.println();
  Serial.println("================================");
  Serial.println("BLE SCAN START");
  Serial.println("================================");

  NimBLEScanResults results =
      scan->getResults(8000, false);

  int count = results.getCount();

  Serial.print("Devices found: ");
  Serial.println(count);

  for (int i = 0; i < count && deviceCount < 40; i++)
  {
    const NimBLEAdvertisedDevice* dev =
        results.getDevice(i);

    if (!dev)
      continue;

    if (dev->haveName())
      devices[deviceCount].name =
          String(dev->getName().c_str());
    else
      devices[deviceCount].name = "(без имени)";

    devices[deviceCount].address =
        String(dev->getAddress().toString().c_str());

    devices[deviceCount].rssi =
        dev->getRSSI();

    Serial.print(deviceCount + 1);
    Serial.print(" | ");
    Serial.print(devices[deviceCount].name);
    Serial.print(" | ");
    Serial.print(devices[deviceCount].address);
    Serial.print(" | RSSI ");
    Serial.println(devices[deviceCount].rssi);

    deviceCount++;
  }

  scan->clearResults();

  Serial.println("================================");
  Serial.println("BLE SCAN END");
  Serial.println("================================");
}


void handleRoot()
{
  String html = pageStart("JBD ESP32 Monitor");

  html += "<h1>JBD ESP32 Monitor</h1>";

  html += "<p>ESP32 DevKit V1 / HW-463C V0.0.6</p>";

  html += "<p>";
  html += "Wi-Fi: <b>JBD-MONITOR</b><br>";
  html += "IP: <b>192.168.4.1</b>";
  html += "</p>";

  html += "<hr>";

  html += "<h2>Bluetooth</h2>";

  html += "<p>";
  html += "Поиск доступных BLE устройств";
  html += "</p>";

  html += "<a href='/scan'>";
  html += "<button>СКАНИРОВАТЬ BLE</button>";
  html += "</a>";

  html += "<hr>";

  html += "<h2>BMS</h2>";

  html += "<p>";
  html += "Сохранённых BMS: 0 / 9";
  html += "</p>";

  html += "<hr>";

  html += "<h2>Инвертор</h2>";

  html += "<p>";
  html += "POW-HVM6.2M-48V-N";
  html += "</p>";

  html += "<p class='warn'>";
  html += "ПОДКЛЮЧЕНИЕ ОТКЛЮЧЕНО";
  html += "</p>";

  html += pageEnd();

  server.send(200, "text/html", html);
}


void handleScan()
{
  scanBLE();

  String html = pageStart("BLE Scan");

  html += "<h1>BLE SCAN</h1>";

  html += "<p>Найдено устройств: <b>";
  html += String(deviceCount);
  html += "</b></p>";

  if (deviceCount == 0)
  {
    html += "<p class='warn'>";
    html += "BLE устройства не найдены.";
    html += "</p>";
  }
  else
  {
    html += "<table>";

    html += "<tr>";
    html += "<th>#</th>";
    html += "<th>Имя</th>";
    html += "<th>MAC</th>";
    html += "<th>RSSI</th>";
    html += "</tr>";

    for (int i = 0; i < deviceCount; i++)
    {
      html += "<tr>";

      html += "<td>";
      html += String(i + 1);
      html += "</td>";

      html += "<td>";
      html += devices[i].name;
      html += "</td>";

      html += "<td>";
      html += devices[i].address;
      html += "</td>";

      html += "<td>";
      html += String(devices[i].rssi);
      html += " dBm";
      html += "</td>";

      html += "</tr>";
    }

    html += "</table>";
  }

  html += "<br>";

  html += "<a href='/scan'>";
  html += "<button>СКАНИРОВАТЬ ЕЩЁ</button>";
  html += "</a>";

  html += "<br><br>";

  html += "<a href='/'>";
  html += "<button>НАЗАД</button>";
  html += "</a>";

  html += pageEnd();

  server.send(200, "text/html", html);
}


void setup()
{
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("JBD ESP32 MONITOR");
  Serial.println("ESP32 DevKit V1");
  Serial.println("HW-463C V0.0.6");
  Serial.println("================================");

  WiFi.mode(WIFI_AP);

  WiFi.softAP(
      AP_SSID,
      AP_PASS
  );

  Serial.print("Wi-Fi AP: ");
  Serial.println(AP_SSID);

  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  NimBLEDevice::init("JBD-MONITOR");

  NimBLEDevice::setPower(3);

  NimBLEScan* scan =
      NimBLEDevice::getScan();

  scan->setActiveScan(true);
  scan->setInterval(100);
  scan->setWindow(80);

  server.on("/", HTTP_GET, handleRoot);

  server.on("/scan", HTTP_GET, handleScan);

  server.begin();

  Serial.println("Web server started");
  Serial.println("Open ⁨http://192.168.4.1⁩");
}


void loop()
{
  server.handleClient();

  delay(5);
}
