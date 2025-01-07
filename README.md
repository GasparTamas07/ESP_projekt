# ESP8266 LED vezérlő dokumentáció
## 1. A platform bemutatása
  Az ESP8266EX NodeMCU v3 egy alacsony költségű, Wi-Fi-képes mikrokontroller, amely ideális választás IoT-projektekhez. A beépített TCP/IP protokollstack lehetővé teszi az egyszerű vezeték nélküli hálózati csatlakozást, miközben a GPIO-lábak különféle perifériák vezérlésére használhatók.

  Ez a projekt a következő fő funkciókat valósítja meg:

  - Hőmérséklet- és páratartalom-érzékelés DHT11 szenzorral.
  - LED-ek állapotának szabályozása a mért hőmérséklet alapján.
  - Egy webes interfész, amely valós időben jeleníti meg az érzékelő adatokat, valamint a LED-ek aktuális állapotát.

  **Az ESP8266EX NodeMCU v3 fő jellemzői**
 - Processzor: 32 bites Tensilica L106, 80 MHz-es órajellel.
 - Wi-Fi: 802.11 b/g/n támogatás beépített TCP/IP stackkel.
 - GPIO lábak: 11 elérhető különféle perifériákhoz.
 - Flash memória: 4 MB.
 - Feszültség: 3,3V-os működés, USB-ről történő tápellátással.

## 2. Tervdokumentáció
**Kapcsolási rajz és felépítés**

  A projektben az ESP8266EX-hez egy DHT11 hőmérséklet- és páratartalom-érzékelő, valamint két LED (piros és zöld) csatlakozik. Az alábbi táblázat összefoglalja a csatlakozásokat:

| ESP8266 GPIO	| Alkatrész	| Funkció |
|:-----------:|:---------:|:-----:|
| GPIO4	| DHT11	| Hőmérséklet- és páratartalom mérés |
| GPIO5 (D1) | Piros LED | Hőmérséklet alapú visszajelzés |
| GPIO4 (D2) | Zöld LED |	Hőmérséklet alapú visszajelzés |

**Alkatrészlista**
| Alkatrész |	Mennyiség |	Leírás |
|:---------:|:---------:|:------:|
| ESP8266EX |	1 |	Wi-Fi képes mikrokontroller |
| DHT11 |	1	| Hőmérséklet- és páratartalom-érzékelő |
| Piros LED |	1 |	Hőmérséklet-visszajelzés |
| Zöld LED |	1 |	Hőmérséklet-visszajelzés |
| Ellenállás (220Ω) |	2 |	LED-ek áramkorlátozásához |
| Vezetékek	| Több |	Csatlakozások |

## 3. A megvalósítás leírása
**Működési leírás**

 1. Az ESP8266EX a DHT11 érzékelő segítségével méri a környezeti hőmérsékletet és páratartalmat.
 1. A mért hőmérséklet alapján vezérli a LED-ek állapotát:
    - Zöld LED: Világít, ha a hőmérséklet 20–22 °C között van.
    - Piros LED: Világít, ha a hőmérséklet kívül esik az optimális tartományon.
 1. Az adatokat egy beépített webes felületen jeleníti meg, amely valós időben frissül.

**Részletes működés**

A fő működési ciklus a következő lépésekből áll:

  1. **Hőmérséklet- és páratartalom-mérés:** Az ESP8266 olvassa a DHT11 érzékelőt és ellenőrzi az adatokat.

```cpp
float temperature = dht.readTemperature();
float humidity = dht.readHumidity();
if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
}
```

Ha a mért adatok érvényesek, a rendszer továbblép a LED-ek vezérlésére.

  2. **LED-ek vezérlése:** A mért hőmérséklet alapján az ESP8266 meghatározza, melyik LED világítson:

```cpp
if (temperature >= 20 && temperature <= 22) {
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
} else {
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
}
```

Ez egyszerű, átlátható logikával biztosítja a vizuális visszajelzést.

  3. **Webes felület frissítése:** A rendszer az aktuális adatokat HTML formátumban küldi a kliens böngészőjének:

```cpp
client.println("<!DOCTYPE html><html>");
client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
client.println("<style>body { text-align: center; font-family: Arial; }</style></head>");
client.println("<body><h1>ESP8266 Sensor Monitor</h1>");
client.println("<p>Hőmérséklet: " + String(temperature) + " °C</p>");
client.println("<p>Páratartalom: " + String(humidity) + " %</p>");
client.println("<p>Zöld LED: " + String(temperature >= 20 && temperature <= 22 ? "ON" : "OFF") + "</p>");
client.println("<p>Piros LED: " + String(temperature < 20 || temperature > 22 ? "ON" : "OFF") + "</p>");
client.println("</body></html>");
```

## 5. Következtetések
Ez a projekt jól demonstrálja az ESP8266EX IoT környezetben való alkalmazását:

 - **Egyszerű adatmegjelenítés:** A webes felület megkönnyíti az érzékelők adatainak nyomon követését.
 - **Valós idejű vezérlés:** Az ESP8266 gyors reagálást biztosít a környezeti változásokra.
 - **Könnyű bővíthetőség:** A rendszer egyszerűen kiterjeszthető további érzékelőkkel vagy vezérlőkkel.
