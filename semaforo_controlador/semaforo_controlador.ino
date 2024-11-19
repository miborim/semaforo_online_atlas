#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// Certificado raiz do HiveMQ Cloud (válido até 2025)
const char* root_ca = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFBjCCAu6gAwIBAgIRAIp9PhPWLzDvI4a9KQdrNPgwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjQwMzEzMDAwMDAw
WhcNMjcwMzEyMjM1OTU5WjAzMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg
RW5jcnlwdDEMMAoGA1UEAxMDUjExMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB
CgKCAQEAuoe8XBsAOcvKCs3UZxD5ATylTqVhyybKUvsVAbe5KPUoHu0nsyQYOWcJ
DAjs4DqwO3cOvfPlOVRBDE6uQdaZdN5R2+97/1i9qLcT9t4x1fJyyXJqC4N0lZxG
AGQUmfOx2SLZzaiSqhwmej/+71gFewiVgdtxD4774zEJuwm+UE1fj5F2PVqdnoPy
6cRms+EGZkNIGIBloDcYmpuEMpexsr3E+BUAnSeI++JjF5ZsmydnS8TbKF5pwnnw
SVzgJFDhxLyhBax7QG0AtMJBP6dYuC/FXJuluwme8f7rsIU5/agK70XEeOtlKsLP
Xzze41xNG/cLJyuqC0J3U095ah2H2QIDAQABo4H4MIH1MA4GA1UdDwEB/wQEAwIB
hjAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwEgYDVR0TAQH/BAgwBgEB
/wIBADAdBgNVHQ4EFgQUxc9GpOr0w8B6bJXELbBeki8m47kwHwYDVR0jBBgwFoAU
ebRZ5nu25eQBc4AIiMgaWPbpm24wMgYIKwYBBQUHAQEEJjAkMCIGCCsGAQUFBzAC
hhZodHRwOi8veDEuaS5sZW5jci5vcmcvMBMGA1UdIAQMMAowCAYGZ4EMAQIBMCcG
A1UdHwQgMB4wHKAaoBiGFmh0dHA6Ly94MS5jLmxlbmNyLm9yZy8wDQYJKoZIhvcN
AQELBQADggIBAE7iiV0KAxyQOND1H/lxXPjDj7I3iHpvsCUf7b632IYGjukJhM1y
v4Hz/MrPU0jtvfZpQtSlET41yBOykh0FX+ou1Nj4ScOt9ZmWnO8m2OG0JAtIIE38
01S0qcYhyOE2G/93ZCkXufBL713qzXnQv5C/viOykNpKqUgxdKlEC+Hi9i2DcaR1
e9KUwQUZRhy5j/PEdEglKg3l9dtD4tuTm7kZtB8v32oOjzHTYw+7KdzdZiw/sBtn
UfhBPORNuay4pJxmY/WrhSMdzFO2q3Gu3MUBcdo27goYKjL9CTF8j/Zz55yctUoV
aneCWs/ajUX+HypkBTA+c8LGDLnWO2NKq0YD/pnARkAnYGPfUDoHR9gVSp/qRx+Z
WghiDLZsMwhN1zjtSC0uBWiugF3vTNzYIEFfaPG7Ws3jDrAMMYebQ95JQ+HIBD/R
PBuHRTBpqKlyDnkSHDHYPiNX3adPoPAcgdF3H2/W0rmoswMWgTlLn1Wu0mrks7/q
pdWfS6PJ1jty80r2VKsM/Dj3YIDfbjXKdaFU5C+8bhfJGqU3taKauuz0wHVGT3eo
6FlWkWYtbt4pgdamlwVeZEW+LM7qZEJEsMNPrfC03APKmZsJgpWCDWOKZvkZcvjV
uYkQ4omYCTX5ohy+knMjdOmdH9c7SpqEWBDC86fiNex+O0XOMEZSa8DA
-----END CERTIFICATE-----
)EOF";

WiFiClientSecure espClient;
PubSubClient client(espClient);

const char* ssid = "esp32";
const char* password = "jonathan123";

const char* mqtt_server = "c4bd3577289c4b58a61f581b8260c62c.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_username = "jonathan";
const char* mqtt_password = "Galo@2267";

const char* mqtt_client_id = "esp32_semaforo1";
const char* mqtt_topic_sync = "traffic_light_sync";

// Pinos dos LEDs
const int RED_PIN = 27;
const int YELLOW_PIN = 14;
const int GREEN_PIN = 12;

// LDR
const int LDR_PIN = 32;
const int LIGHT_THRESHOLD = 1000; // Ajuste o limite conforme necessário

// Estados do semáforo
enum TrafficState { RED, GREEN, YELLOW, LDR_MODE };
TrafficState currentState = RED;

// Tempos para cada estado (em ms)
const unsigned long RED_DURATION = 5000;
const unsigned long GREEN_DURATION = 5000;
const unsigned long YELLOW_DURATION = 2000;
const unsigned long LDR_BLINK_DURATION = 500; // Intervalo de piscar do LED amarelo no modo LDR

unsigned long stateStartTime = 0;

void setup_wifi() {
    delay(10);
    Serial.println("Conectando ao WiFi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi conectado!");
    Serial.println("Endereço IP: ");
    Serial.println(WiFi.localIP());
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Semáforo 1 é o Master, não precisa reagir às mensagens MQTT.
}

void reconnect() {
    while (!client.connected()) {
        Serial.println("Tentando conectar ao MQTT...");
        if (client.connect(mqtt_client_id, mqtt_username, mqtt_password)) {
            Serial.println("Conectado ao MQTT!");
        } else {
            Serial.print("Falha, rc=");
            Serial.print(client.state());
            Serial.println(" tentando novamente em 5 segundos.");
            delay(5000);
        }
    }
}

void setLights(bool red, bool yellow, bool green) {
    digitalWrite(RED_PIN, red);
    digitalWrite(YELLOW_PIN, yellow);
    digitalWrite(GREEN_PIN, green);
}

void handleLDR() {
    int ldrValue = analogRead(LDR_PIN);
    Serial.print("LDR Value: ");
    Serial.println(ldrValue);

    if (ldrValue >= LIGHT_THRESHOLD) {
        currentState = LDR_MODE;
    } else if (currentState == LDR_MODE) { 
        currentState = RED; 
        stateStartTime = millis();
    }
}


void changeState() {
    switch (currentState) {
        case RED:
            setLights(HIGH, LOW, LOW);
            if (millis() - stateStartTime >= RED_DURATION) {
                currentState = GREEN;
                stateStartTime = millis();
                client.publish(mqtt_topic_sync, "GREEN");
            }
            break;

        case GREEN:
            setLights(LOW, LOW, HIGH);
            if (millis() - stateStartTime >= GREEN_DURATION) {
                currentState = YELLOW;
                stateStartTime = millis();
                client.publish(mqtt_topic_sync, "YELLOW");
            }
            break;

        case YELLOW:
            setLights(LOW, HIGH, LOW);
            if (millis() - stateStartTime >= YELLOW_DURATION) {
                currentState = RED;
                stateStartTime = millis();
                client.publish(mqtt_topic_sync, "RED");
            }
            break;

        case LDR_MODE:
            setLights(HIGH, LOW, LOW);
            break;
    }
}

void setup() {
    pinMode(RED_PIN, OUTPUT);
    pinMode(YELLOW_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(LDR_PIN, INPUT);

    Serial.begin(115200);

    setup_wifi();
    espClient.setCACert(root_ca);
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(mqttCallback);

    while (!client.connected()) {
        reconnect();
    }

    client.subscribe(mqtt_topic_sync);

    stateStartTime = millis();
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    handleLDR();
    changeState();
}
