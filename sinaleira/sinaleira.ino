#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "MQTT";
const char* password = "12345678";

const char* mqtt_server = "172.16.0.200";
const char* hostname = "sinaleira-02";

char topic_controle[50];
char topic_status[50];

#define LED_VERDE D5
#define LED_AMARELO D4
#define LED_VERMELHO D3

WiFiClient espClient;
PubSubClient client(espClient);

bool aberto = false;
bool em_transicao = false;

unsigned long tempo_transicao = 0;
#define TEMPO_AMARELO 8000

void setup_wifi();

void setup_wifi()
{
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
}

void controlar_sinaleira(String comando){
  if (comando == "ABRIR" && !em_transicao){
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_AMARELO, LOW);
    digitalWrite(LED_VERMELHO, LOW);

    em_transicao = true;
    tempo_transicao = millis();
  }
}

void callback(char* topic, byte* payload, unsigned int length)
{
  String comando = "";
  for(int i = 0; i < length; i++){
    comando += (char)payload[i];
  }

  if (comando == "ABRIR" || comando == "FECHAR") {
    controlar_sinaleira(comando);
  }
}

void setup() {
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARELO, LOW);
  digitalWrite(LED_VERMELHO, HIGH);

  setup_wifi();

  snprintf(topic_controle, sizeof(topic_controle), "esquina/%s/controle", hostname);
  snprintf(topic_status,  sizeof(topic_status),  "esquina/%s/status", hostname);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void reconnect(){
  while(!client.connected()){
    if(client.connect(hostname)){
      client.subscribe(topic_controle);
      client.publish(topic_status, "REGISTRADA");
    } else{
      delay(1000);
    }
  }
}

void loop() { 
  if(!client.connected()){
    reconnect();
  }

  client.loop();
}
