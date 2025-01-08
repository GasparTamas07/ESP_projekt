#include <ESP8266WiFi.h>
#include <DHT.h>

// DHT setup
#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// LED setup
const int redLED = D0;   
const int greenLED = D1; 

// Wi-Fi credentials
const char* ssid = "";
const char* password = "";

// Web server setup
WiFiServer server(80);
String header; // HTTP request header

// Temperature limits
float tempMin = 20.0; // Default minimum temperature
float tempMax = 22.0; // Default maximum temperature

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
            // Process temperature update request
            if (header.indexOf("GET /update") >= 0) {
              int tempMinIndex = header.indexOf("tempMin=");
              int tempMaxIndex = header.indexOf("tempMax=");
              
              if (tempMinIndex != -1 && tempMaxIndex != -1) {
                String tempMinValue = header.substring(tempMinIndex + 8, header.indexOf('&', tempMinIndex));
                String tempMaxValue = header.substring(tempMaxIndex + 8, header.indexOf(' ', tempMaxIndex));
                
                tempMin = tempMinValue.toFloat();
                tempMax = tempMaxValue.toFloat();
                
                Serial.println("Updated temperature limits:");
                Serial.println("tempMin: " + String(tempMin));
                Serial.println("tempMax: " + String(tempMax));
              }
            }

            // Read temperature and humidity
            float temperature = dht.readTemperature();
            float humidity = dht.readHumidity();
            if (isnan(temperature) || isnan(humidity)) {
                Serial.println("Failed to read from DHT sensor!");
                return;
            }

            // Control LEDs based on temperature
            if (temperature >= tempMin && temperature <= tempMax) {
              digitalWrite(greenLED, HIGH);
              digitalWrite(redLED, LOW);
            } else {
              digitalWrite(redLED, HIGH);
              digitalWrite(greenLED, LOW);
            }

            // Generate HTTP response
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html; charset=utf-8");
            client.println("Connection: close");
            client.println();

            // HTML response
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<style>body { text-align: center; font-family: Arial; }</style></head>");
            client.println("<body><h1>ESP8266 Sensor Monitor</h1>");
            client.println("<p>Hőmérséklet: " + String(temperature) + " °C</p>");
            client.println("<p>Páratartalom: " + String(humidity) + " %</p>");
            client.println("<p>Zöld LED: " + String(temperature >= tempMin && temperature <= tempMax ? "ON" : "OFF") + "</p>");
            client.println("<p>Piros LED: " + String(temperature < tempMin || temperature > tempMax ? "ON" : "OFF") + "</p>");
            client.println("<form action=\"/update\" method=\"get\">");
            client.println("<label for=\"tempMin\">Minimum hőmérséklet:</label>");
            client.println("<input type=\"number\" id=\"tempMin\" name=\"tempMin\" step=\"0.1\" value=\"" + String(tempMin) + "\"><br>");
            client.println("<label for=\"tempMax\">Maximum hőmérséklet:</label>");
            client.println("<input type=\"number\" id=\"tempMax\" name=\"tempMax\" step=\"0.1\" value=\"" + String(tempMax) + "\"><br>");
            client.println("<input type=\"submit\" value=\"Mentés\">");
            client.println("</form>");
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
