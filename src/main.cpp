#include "main.h"


WiFiClient wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

DHT20 dht20;



RPC_Response setLedSwitchState(const RPC_Data &data)
{
    Serial.println("Received Switch state");
    bool newState = data;
    Serial.print("Switch state change: ");
    Serial.println(newState);
    digitalWrite(LED_PIN, newState);
    attributesChanged = true;
    return RPC_Response("setLedSwitchValue", newState);
}

const std::array<RPC_Callback, 1U> callbacks = {
    RPC_Callback{"setLedSwitchValue", setLedSwitchState}};

void processSharedAttributes(const Shared_Attribute_Data &data)
{
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        if (strcmp(it->key().c_str(), BLINKING_INTERVAL_ATTR) == 0)
        {
            const uint16_t new_interval = it->value().as<uint16_t>();
            if (new_interval >= BLINKING_INTERVAL_MS_MIN && new_interval <= BLINKING_INTERVAL_MS_MAX)
            {
                blinkingInterval = new_interval;
                Serial.print("Blinking interval is set to: ");
                Serial.println(new_interval);
            }
        }
        else if (strcmp(it->key().c_str(), LED_STATE_ATTR) == 0)
        {
            ledState = it->value().as<bool>();
            digitalWrite(LED_PIN, ledState);
            Serial.print("LED state is set to: ");
            Serial.println(ledState);
        }
    }
    attributesChanged = true;
}

const Shared_Attribute_Callback attributes_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
const Attribute_Request_Callback attribute_shared_request_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());

void InitWiFi()
{
    Serial.println("Connecting to AP ...");
    // Attempting to establish a connection to the given WiFi network
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        // Delay 500ms until a connection has been successfully established
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to AP");
}

const bool reconnect()
{
    // Check to ensure we aren't connected yet
    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
    {
        return true;
    }
    // If we aren't establish a new connection to the given WiFi network
    InitWiFi();
    return true;
}

void setup()
{
    Serial.begin(SERIAL_DEBUG_BAUD);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_A, INPUT);
    pinMode(BTN_B, INPUT);
    delay(1000);
    InitWiFi();
    Wire.begin(SDA_PIN, SCL_PIN);
    dht20.begin();
}

void loop()
{
    delay(10);
    getKeyInput(0, BTN_A);
    if (isButtonPressed(0) == 1)
    {
        Serial.println("Kien ga");
        tb.sendAttributeData("Button", 1);
        digitalWrite(LED_PIN, HIGH);
    }
    else {
        tb.sendAttributeData("Button", 0);
        digitalWrite(LED_PIN, LOW);
    }
    if (!reconnect())
    {
        return;
    }

    if (!tb.connected())
    {
        Serial.print("Connecting to: ");
        Serial.print(THINGSBOARD_SERVER);
        Serial.print(" with token ");
        Serial.println(TOKEN);
        if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT))
        {
            Serial.println("Failed to connect");
            return;
        }

        tb.sendAttributeData("macAddress", WiFi.macAddress().c_str());

        Serial.println("Subscribing for RPC...");
        if (!tb.RPC_Subscribe(callbacks.cbegin(), callbacks.cend()))
        {
            Serial.println("Failed to subscribe for RPC");
            return;
        }

        if (!tb.Shared_Attributes_Subscribe(attributes_callback))
        {
            Serial.println("Failed to subscribe for shared attribute updates");
            return;
        }

        Serial.println("Subscribe done");

        if (!tb.Shared_Attributes_Request(attribute_shared_request_callback))
        {
            Serial.println("Failed to request for shared attributes");
            return;
        }
    }

    if (attributesChanged)
    {
        attributesChanged = false;
        tb.sendAttributeData(LED_STATE_ATTR, digitalRead(LED_PIN));
    }


    //blink led
    // if (ledMode == 1 && millis() - previousStateChange > blinkingInterval)
    // {
    //     previousStateChange = millis();
    //     digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    //     Serial.print("LED state changed to: ");
    //     Serial.println(!digitalRead(LED_PIN));
    // }

    if (millis() - previousDataSend > telemetrySendInterval)
    {
        previousDataSend = millis();

        dht20.read();

        float temperature = dht20.getTemperature();
        float humidity = dht20.getHumidity();

        if (isnan(temperature) || isnan(humidity))
        {
            Serial.println("Failed to read from DHT20 sensor!");
        }
        else
        {
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.print(" °C, Humidity: ");
            Serial.print(humidity);
            Serial.println(" %");

            tb.sendTelemetryData("temperature", temperature);
            tb.sendTelemetryData("humidity", humidity);
        }

        tb.sendAttributeData("rssi", WiFi.RSSI());
        tb.sendAttributeData("channel", WiFi.channel());
        tb.sendAttributeData("bssid", WiFi.BSSIDstr().c_str());
        tb.sendAttributeData("localIp", WiFi.localIP().toString().c_str());
        tb.sendAttributeData("ssid", WiFi.SSID().c_str());
    }

    tb.loop();
}
