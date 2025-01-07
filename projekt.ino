#include <ESP8266WiFi.h>
#include <Servo.h>
#include <DHT.h>

// Servo setup
Servo myservo;
const int servoPin = D4;  // Use GPIO2 (D4 on NodeMCU) for the servo

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
String header;

// Variables
int servoPosition = 90; // Default servo position
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000; // Client timeout

void setup() {
  Serial.begin(115200);

  // Servo setup
  myservo.attach(servoPin);
  myservo.write(servoPosition);

  // DHT sensor setup
  dht.begin();

  // LED setup
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

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
    String currentLine = "";
    currentTime = millis();
    previousTime = currentTime;

    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;

        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Handle DHT sensor readings
            float temperature = dht.readTemperature();
            float humidity = dht.readHumidity();

            // LED control based on temperature
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

            // Display HTML page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<style>body { font-family: Arial; text-align: center; }</style>");
            client.println("<script>");
            client.println("function updateServo(val) {");
            client.println("  var xhttp = new XMLHttpRequest();");
            client.println("  xhttp.open(\"GET\", \"/?servo=\" + val, true);");
            client.println("  xhttp.send();");
            client.println("}");
            client.println("</script></head>");
            client.println("<body><h1>ESP8266 Servo Control</h1>");
            client.println("<p>Temperature: " + String(temperature) + " °C</p>");
            client.println("<p>Humidity: " + String(humidity) + " %</p>");
            client.println("<p>Servo Position: " + String(servoPosition) + "</p>");
            client.println("<input type=\"range\" min=\"0\" max=\"180\" value=\"" + String(servoPosition) + "\" ");
            client.println("oninput=\"updateServo(this.value)\">");
            client.println("</body></html>");

            // Handle GET requests
            if (header.indexOf("GET /?servo=") >= 0) {
              int startPos = header.indexOf('=');
              int endPos = header.indexOf(' ', startPos);
              String posValue = header.substring(startPos + 1, endPos);
              servoPosition = posValue.toInt();
              myservo.write(servoPosition);
              Serial.println("Updated Servo Position: " + String(servoPosition));
            }

            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
  }
}
