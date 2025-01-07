#include <ESP8266WiFi.h>
#include <DHT.h>

// DHT setup
#define DHTPIN D3        // GPIO0 (D3 on NodeMCU)
#define DHTTYPE DHT11    // DHT11 sensor type
DHT dht(DHTPIN, DHTTYPE);

// LED setup
const int redLED = D1;   // GPIO5 (D1 on NodeMCU)
const int greenLED = D2; // GPIO4 (D2 on NodeMCU)

// Wi-Fi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Web server setup
WiFiServer server(80);
String header; // HTTP request header

void setup() {
  Serial.begin(115200);

  // DHT sensor setup
  dht.begin();

  // LED setup
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, LOW);

  // Wi-Fi setup
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.println("IP address: " + WiFi.localIP().toString());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New Client.");
    String currentLine = ""; // To store incoming HTTP request
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;

        if (c == '\n') {
          // End of HTTP header, send response
          if (currentLine.length() == 0) {
            // Read temperature and humidity
            float temperature = dht.readTemperature();
            float humidity = dht.readHumidity();
            if (isnan(temperature) || isnan(humidity)) {
                Serial.println("Failed to read from DHT sensor!");
                return;
            }

            // Control LEDs based on temperature
            if (temperature >= 20 && temperature <= 22) {
              digitalWrite(greenLED, HIGH);
              digitalWrite(redLED, LOW);
            } else {
              digitalWrite(redLED, HIGH);
              digitalWrite(greenLED, LOW);
            }

            // Generate HTTP response
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // HTML response
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<style>body { text-align: center; font-family: Arial; }</style></head>");
            client.println("<body><h1>ESP8266 Sensor Monitor</h1>");
            client.println("<p>Hőmérséklet: " + String(temperature) + " °C</p>");
            client.println("<p>Páratartalom: " + String(humidity) + " %</p>");
            client.println("<p>Zöld LED: " + String(temperature >= 20 && temperature <= 22 ? "ON" : "OFF") + "</p>");
            client.println("<p>Piros LED: " + String(temperature < 20 || temperature > 22 ? "ON" : "OFF") + "</p>");
            client.println("</body></html>");

            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = ""; // Clear the header variable
    client.stop();
    Serial.println("Client disconnected.");
  }
}
