#include <ssl_client.h>
#include <dummy.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

//Definicao das variaveis
const int pir1 = 14;
const int pir2 = 13; 
const int led1 = 18;
const int led2 = 15;

int sinal1;
int sinal2;
int travasole = 5;//Pino de entrada onde é acionada a trava
int letrava;

const char* ssid = "Online Nery"; //Aqui o nome da sua rede local wi fi
const char* password =  "nery9134321"; // Aqui a senha da sua rede local wi fi

const char* mqttServer = "m15.cloudmqtt.com"; // Aqui o endereÃ§o do seu servidor fornecido pelo site 
const int mqttPort =  13910; // Aqui mude para sua porta fornecida pelo site
const char* mqttUser = "poucpysi"; //  Aqui o nome de usuario fornecido pelo site
const char* mqttPassword = "YKqY0-hCcGXG"; //  Aqui sua senha fornecida pelo site
const char* mqttTopicSub ="Casa"; 

//Colocar a API Key para escrita neste campo
//Ela é fornecida no canal que foi criado na aba API Keys
String apiKey = "V0P1NNHAIWAIDTRI";//trocar pela API Write
const char* server = "api.thingspeak.com";

WiFiClient client;
WiFiClient espClient;
PubSubClient client_mqtt(espClient);

void mqtt_callback(char* topic, byte* payload, unsigned int length);

void trava (){
String comando;

 if (Serial.available()>0)
    {
        String comando = Serial.readStringUntil('\n'); //comando para leitura de uma String para abertura da porta
        //comando = Serial.read(); //comando usado caso eu queira acionar a trava apenas com um caractere
          
        if (comando == "abre1") 
        {
           digitalWrite(travasole,LOW);
           Serial.print("Porta aberta");
           delay(5000);
           digitalWrite(travasole,HIGH);
           Serial.print("Porta fechada");
           
           }
  else {
          digitalWrite(travasole,HIGH);
          Serial.print("Porta fechada");
          }
digitalWrite(letrava,LOW);
    }
  }

void sensor_pir(){
  //Faz a leitura do sensor de presença (retorna HIGH ou LOW)
  sinal1 = digitalRead(pir1);
  sinal2 = digitalRead(pir2); 
  
  if (sinal1 == HIGH){
    digitalWrite (led1, HIGH);
    delay(5000);
    Serial.println ("Localização - Cômodo 1");
    }
  if (sinal2 == HIGH) {
    digitalWrite (led2, HIGH);
    Serial.println ("Localização - Cômodo 2");
    delay(5000);
    }
    else {
      digitalWrite (led1, LOW);
      digitalWrite (led2, LOW);
      Serial.println ("Sem Movimento");
      }
  }

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  pinMode (pir1, INPUT);
  pinMode (pir2, INPUT);
  pinMode (led1, OUTPUT);
  pinMode (led2, OUTPUT);
  pinMode(travasole, OUTPUT);
  digitalWrite(travasole, HIGH); 
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
     
  }
  Serial.println("Connected to the WiFi network");
 
  client_mqtt.setServer(mqttServer, mqttPort);
  client_mqtt.setCallback(callback);
 
  while (!client_mqtt.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client_mqtt.connect("NodeMCU ESP32", mqttUser, mqttPassword )) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client_mqtt.state());
      delay(2000);
 
    }
  }
  //subscreve no tópico
  client_mqtt.subscribe(mqttTopicSub);
  client_mqtt.publish("Trava Eletrica","Palavra-chave");
  client_mqtt.subscribe("Porta Casa");
 
}

void callback(char* topic, byte* payload, unsigned int length) {
 
  //armazena msg recebida em uma sring
  payload[length] = '\0';
  String strMSG = String((char*)payload);
 
  
  Serial.print("Mensagem chegou do tópico: ");
  Serial.println(topic);
  Serial.print("Mensagem:");
  Serial.print(strMSG);
  Serial.println();
  Serial.println("..........................");
 
 letrava = digitalRead(travasole); 
        
  //aciona saída conforme msg recebida 
  if (strMSG == "abre1")
    {   
        digitalWrite(travasole,LOW);
        Serial.println("Porta aberta");
        delay(5000);
        digitalWrite(travasole,HIGH);
        Serial.println("Porta fechada");       
    }
if (strMSG == "abrir")
    {
        digitalWrite(travasole,LOW);
        Serial.println("Porta aberta");
    }
if (strMSG == "fechar")
    {
        digitalWrite(travasole,HIGH);
        Serial.println("Porta fechada");
    }
  }

 
//função pra reconectar ao servido MQTT
void reconect() {
  //Enquanto estiver desconectado
  while (!client_mqtt.connected()) {
   Serial.print("Tentando conectar ao servidor MQTT");
    
    bool conectado = strlen(mqttUser) > 0 ?
                     client_mqtt.connect("ESP8266Client", mqttUser, mqttPassword) :
                     client_mqtt.connect("ESP8266Client");
 
    if(conectado) {
      Serial.println("Conectado!");
      
      //subscreve no tópico
      client_mqtt.subscribe(mqttTopicSub, 1); //nivel de qualidade: QoS 1
    } else {
      Serial.println("Falha durante a conexão.Code: ");
      Serial.println( String(client_mqtt.state()).c_str());
      Serial.println("Tentando novamente em 10 s");
      
      //Aguarda 10 segundos 
      delay(10000);
    }
  }
}


void loop() {
  if (!client_mqtt.connected()) {
    reconect();
  }
  client_mqtt.loop();
  trava ();
  sensor_pir();
  delay(5000);

  //Inicia um client TCP para o envio dos dados
  if (client.connect(server,80)) {
    String postStr = apiKey;
           postStr +="&amp;field1=";
           postStr += String(sinal1);
           postStr +="&amp;field2=";
           postStr += String(sinal2);
           postStr +="&amp;field3=";
           postStr += String(letrava);
           postStr += "\r\n\r\n";

     client.print("POST /update HTTP/1.1\n");
     client.print("Host: api.thingspeak.com\n");
     client.print("Connection: close\n");
     client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
     client.print("Content-Type: application/x-www-form-urlencoded\n");
     client.print("Content-Length: ");
     client.print(postStr.length());
     client.print("\n\n");
     client.print(postStr);

     //Logs na porta serial
     Serial.print("Comodo 1: ");
     Serial.print(sinal1);
     Serial.print(" Comodo 2: ");
     Serial.println(sinal2);
     Serial.print(" Trava Elerica: ");
     Serial.println(letrava);
     }
  client.stop();
}
