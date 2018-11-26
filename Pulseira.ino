//                                               PULSEURA AUTOMATIZADA PARA 
//                                                       IDOSOS                                  
//                                                COM USO DO THINGSPEAK

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <dummy.h>

//Carrega a biblioteca Wire 
//Biblioteca do Acelerometro MPU6050
#include <Wire.h>

//Endereco I2C do MPU6050
const int MPU=0x68;
//Variaveis para armazenar valores dos sensores
int AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

//Definir o SSID da rede WiFi
const char* ssid = "Online Nery";
//Definir a senha da rede WiFi
const char* password = "nery9134321";

//Colocar a API Key para escrita neste campo
//Ela é fornecida no canal que foi criado na aba API Keys
String apiKey = "NREL114IR1EZC1GI";//trocar pela API Write
const char* server = "api.thingspeak.com";

// Chama a biblioteca WiFi
WiFiClient client;

//Define a pinagem do ESP8266
#define D0    16
#define D1    5
#define D2    4
#define D3    0
#define D4    2
#define D5    14
#define D6    12
#define D7    13
#define D8    15
#define D9    3
#define D10   1


//Definicao dos pinos
int led = D3;
int led2 = D7;
int led_panico = D4;
int botao = D8;
int pinoSensor = D6;
int pino_buzzer = D5;
int vibra = D9;

//Variáveis que conterá os estados do botão, do sensor e do led (0 LOW, 1 HIGH).
int estado_led;
int estado_sensor;
int estado_botao; 

void botao_panico(){
  //Lendo o estado do pino 5, constante botao, e atribuindo o resultado a variável estadoBotao.
  estado_botao = digitalRead(botao);          
  //Verificando o estado do botão para definir se acenderá ou apagará o led.  
  if (estado_botao == HIGH) {
    digitalWrite(led_panico,HIGH);//Botão pressionado, acende o led.
  } else {
    digitalWrite(led_panico,LOW);  //Botão não pressionado, apaga o led. 
  }       
  }
  
void acelerometro (){
  //Inicia todo o processo para leitura dos dados do MPU6050
  Wire.beginTransmission(MPU); // iniciando a transmissão
  Wire.write(0x3B); // definindo o registrador que será utilizado
  Wire.endTransmission(false);
  //Solicita os dados do sensor
  Wire.requestFrom(MPU,14,true); 
  //Armazenando os valor lido pelo arduino
  //Aceleração do eixo X, Y e Z 
  AcX=Wire.read()<<8|Wire.read();      
  AcY=Wire.read()<<8|Wire.read();  
  AcZ=Wire.read()<<8|Wire.read(); 
  //Rotação nos eixos X, Y e Z
  GyX=Wire.read()<<8|Wire.read();  
  GyY=Wire.read()<<8|Wire.read();  
  GyZ=Wire.read()<<8|Wire.read(); 
  //Temperatura
  Tmp=Wire.read()<<8|Wire.read(); 

  //Imprime os valores na serial
  Serial.println("Acelerometro \t\tGiroscópio"); 
  Serial.print(" X = "); Serial.print(AcX);
  Serial.print("\t\t X = "); Serial.println(GyX);
  Serial.print(" Y = "); Serial.print(AcY);
  Serial.print("\t\t Y = "); Serial.println(GyY);
  Serial.print(" Z = "); Serial.print(AcZ);
  Serial.print("\t\t Z = "); Serial.println(GyZ);
  Serial.print("\t\t Temp = "); Serial.println(Tmp/340.00+36.53);

  //Condiçoes para que um LED de alerta seja acionado
  //Registra possiveis quedas
  estado_led = digitalRead(led2);
  if (GyY > 65000 && GyZ > 65000 ){
    digitalWrite(led2, HIGH);
    delay(3000);   
 }
 else {
  digitalWrite(led2, LOW);  
  }
}

  void infra (){
    //Faz a leitura do sensor IR e assim verifica se a pulseira está colocada
    estado_sensor = digitalRead(pinoSensor);
    if (estado_sensor == 0){
      Serial.print("Objeto proximo do pulso: ");
      Serial.println(estado_sensor);
      Serial.println("");
      digitalWrite(led, HIGH);
      digitalWrite(vibra, LOW);
    }
    else {
      Serial.print("Objeto longe do pulso: ");
      Serial.println(estado_sensor);
      Serial.println("");
      digitalWrite(led, LOW);
      // Aciona o buzzer 3 vezes caso a pulseira esteja longe do pulso
      for (int i = 1; i <= 3; i++)
      {
      digitalWrite(pino_buzzer, HIGH);
      delay(100);
      digitalWrite(pino_buzzer, LOW);
      delay(100);
      digitalWrite(vibra, HIGH);
      }
      }
     }
     
void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); 
  //Inicializa o MPU-6050
  Wire.write(0); 
  Wire.endTransmission(true);
  //Inicia o WiFi
  WiFi.begin(ssid, password);

  //Pinos
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led_panico, OUTPUT);
  pinMode(botao,INPUT);
  pinMode(pinoSensor, INPUT);
  pinMode(pino_buzzer, OUTPUT);
  pinMode(vibra, OUTPUT);
  //Mostra na Serial
  Serial.println("Waiting for connections...");
  Serial.println("Teste Acelerômetro");
  Serial.println("Teste Sensor Infravermelho");
  //Espera a conexão com a rede
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
}
}

void loop() {
  botao_panico (); //Chama a função do botão
  acelerometro(); //Chama a função do MPU6050
  infra ();  //Chama a função do Sensor IR Infravermelho
  delay(2000);  

  //Inicia um client TCP para o envio dos dados
  if (client.connect(server,80)) {
    String postStr = apiKey;
           postStr +="&amp;field1=";
           postStr += String(estado_led);
           postStr +="&amp;field2=";
           postStr += String(estado_botao);
           postStr +="&amp;field3=";
           postStr += String(estado_sensor);
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
     //Lê os estados das variavés e envia para o ThingSpeak
     Serial.print("Estado do LED: ");
     Serial.println(estado_led);
     Serial.print("Estado do BOTAO: ");
     Serial.println(estado_botao);
     Serial.print("Estado do SENSOR: ");
     Serial.println(estado_sensor);
     Serial.println("..................................................................................");
      
}
client.stop();
}


