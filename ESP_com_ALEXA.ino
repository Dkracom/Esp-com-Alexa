#include <Arduino.h>
#include <fauxmoESP.h>
#include <ESP8266WiFi.h>
#define SERIAL_BAUDRATE 115200
#define WIFI_SSID "Sergio"
#define WIFI_PASS "WAsd1234-"

//Declaração das Variáveis
int controle = 0;
int menu = 0;
int contador = 0;
int disp_adic[4] = {0,0,0,0};
int RELAY_PIN[4] = {16,5,4,0};
char *dispositivos[] = {"Dispositivo 1", "Dispositivo 2", "Dispositivo 3", "Dispositivo 4"};
fauxmoESP fauxmo;

// Wi-Fi Conexão
void wifiSetup() {
  // Define o esp como STA
  WiFi.mode(WIFI_STA);
  // Conecta
  Serial.printf("[WIFI] Conectado ao %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  // Espera
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  // Conectado
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

//FUNÇÕES DA INTERFACE E CONTROLE
void exibir_menu(){
  Serial.println("---- Menu ----");
  Serial.println("1 - Adicionar Dispositivo.");
  Serial.println("2 - Remover Dispositivo.");
  //Serial.println("3 - Alterar Dispositivo");
}

void exibir_disp(){
  for(contador=0;contador < 4;contador++){
    Serial.print(dispositivos[contador]);
    if(contador < 3){
      Serial.print(" - ");  
    }else{
      Serial.println();
    }
  }  
}

void LimparBuffer(){
  while(Serial.available() > 0){
    Serial.read();
  }
}

void setup() {
  // Inicia a Serial
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println();

  // Conexão Wi-Fi
  wifiSetup();

  // Declaração dos Dispositivos
  pinMode(RELAY_PIN[0], OUTPUT);
  digitalWrite(RELAY_PIN[0], HIGH);
  pinMode(RELAY_PIN[1], OUTPUT);
  digitalWrite(RELAY_PIN[1], HIGH);
  pinMode(RELAY_PIN[2], OUTPUT);
  digitalWrite(RELAY_PIN[2], HIGH);
  pinMode(RELAY_PIN[3], OUTPUT);
  digitalWrite(RELAY_PIN[3], HIGH);
  // Por padrão, fauxmoESP cria seu próprio servidor web na porta definida
  // A porta TCP deve ser 80 para dispositivos gen3 (o padrão é 1901)
  // Isso deve ser feito antes da chamada enable()
  fauxmo.createServer(true); // não é necessário, este é o valor padrão
  fauxmo.setPort(80); // Isso é necessário para dispositivos gen3

  // Você deve chamar enable(true) assim que tiver uma conexão WiFi
  // Desativá-lo impedirá que os dispositivos sejam descobertos e trocados
  fauxmo.enable(true);

  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
    // Retorno de chamada quando um comando da Alexa é recebido. 
    //Pode usar device_id ou device_name para escolher o elemento no qual realizar uma ação (relé, LED, ...)
    // O state é um booleano (ON / OFF) e value um número de 0 a 255.

    Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
    exibir_menu();
    for(contador=0;contador < 4;contador++){
      if((strcmp(device_name, dispositivos[contador]) == 0)){
        digitalWrite(RELAY_PIN[contador], !digitalRead(RELAY_PIN[contador]));
        if (state) {
          digitalWrite(RELAY_PIN[contador], LOW);
        } else {
          digitalWrite(RELAY_PIN[contador], HIGH);
        } 
      }
    }
  });
  exibir_menu();
}

void loop() {
  //a biblioteca fauxmoESP utiliza um servidor assíncrono, mas um servidor UDP sincronizado.
  //Portanto temos que pesquisar manualmente os pacotes UDP
  fauxmo.handle();
  menu = 0;
  controle = 0; 
  controle = Serial.read();
  /*ADICIONAR DISPOSITIVO 49 na tabela ASCII equivale ao 1*/  
  if(controle == 49){
    Serial.println("Qual dispositivo você deseja adicionar?");
    exibir_disp();
    LimparBuffer();
    delay(2000);
    menu = Serial.read();   
    if(disp_adic[menu-49] == 0){
      fauxmo.addDevice(dispositivos[menu-49]);
      Serial.println("Adicionado com sucesso");
      disp_adic[menu-49] = 1;
    }else{
      Serial.println("Esse dispositivo já foi adicionado");
    }
    exibir_menu();
  /*REMOVER DISPOSITIVO 50 na tabela ASCII equivale ao 2*/ 
  }else if(controle == 50){
    Serial.println("Qual dispositivo você deseja remover?");
    exibir_disp();
    LimparBuffer();
    delay(2000);
    menu = Serial.read();
    if(disp_adic[menu-49] == 1){
      fauxmo.removeDevice(dispositivos[menu-49]);
      Serial.println("Removido com sucesso");
      disp_adic[menu-49] = 0;
    }else{
          Serial.println("Esse dispositivo já foi removido.");
    }
    exibir_menu();   
  }
}
