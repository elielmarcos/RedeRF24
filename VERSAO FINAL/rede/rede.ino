#include <RF24.h>
#include "DHT.h"
#include <LM35.h>
/*
 *            ARQUITETURA DA REDE
 *               
 *                   M(00) 
 *                 /      \
 *              A|B        C|D
 *              /            \
 *           I0(10)         I1(11) 
 *           /  \            /  \
 *        E|F   G|H        I|J   K|L 
 *        /       \        /       \
 *     S0(20)   S1(21)  S2(22)   S3(23)
*/

// CODIGO DO SENSOR DHT
//***************************** Controle do RF ***************************
#define radioID 0 // Master: 0 (TX) , Intermediario: 1 (RX021), Sensor_1: 2 (DHT), Sensor_2: 3 (LM35)
//***************************** NOS DA REDE ******************************
#define MASTER 0
#define INTERMEDIARIO 1
#define SENSOR0 2
#define SENSOR1 3

//*************** Configuracoes DHT ********************
#define DHTPIN A0 // pino conectado no Arduino
#define DHTTYPE DHT11 // DHT 11
// Conecte pino 1 do sensor (esquerda) ao +5V
// Conecte pino 2 do sensor ao pino de dados definido em seu Arduino
// Conecte pino 4 do sensor ao GND
DHT dht(DHTPIN, DHTTYPE);
//************** Configuracoes LM35 *********************
#define PINLM35 A0 // pino conectado no Arduino
// Conecte pino 1 do sensor (esquerda) ao +5V
// Conecte pino 2 do sensor ao pino de dados definido em seu Arduino
// Conecte pino 3 do sensor ao GND
LM35 sensor(A0);

RF24 radio(7,8);

byte canal[][6] = {"000ch","100ch","200ch","300ch","400ch","500ch"};

typedef struct MSG {
  char enderecoDestino;
  char enderecoOrigem;
  char mensagem[20] = "";
} MSG;

char caractere;
char endereco;
bool transmitido = true;

MSG enviar;
MSG receber;

int i = 0;

void setup() {

  Serial.begin(115200);
  //*************** Controle do RF ***********************
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  
  #if radioID == MASTER
      radio.openWritingPipe(canal[2]);
      radio.openReadingPipe(1, canal[3]);
      endereco = MASTER;
      enviar.enderecoOrigem = endereco;
      enviar.enderecoDestino = INTERMEDIARIO;
      
  #elif radioID == INTERMEDIARIO
      #define WRITEMASTER radio.openWritingPipe(canal[3])
      #define WRITESENSOR0 radio.openWritingPipe(canal[1])
      #define WRITESENSOR1 radio.openWritingPipe(canal[4])
      radio.openReadingPipe(1, canal[0]);
      radio.openReadingPipe(2, canal[2]);
      radio.openReadingPipe(3, canal[5]);
      endereco = INTERMEDIARIO;
      
  #elif radioID == SENSOR0
      radio.openWritingPipe(canal[0]);
      radio.openReadingPipe(1, canal[1]);
      endereco = SENSOR0;
      enviar.enderecoOrigem = endereco;
      enviar.enderecoDestino = MASTER;
      dht.begin();
      
  #elif radioID == SENSOR1 
      radio.openWritingPipe(canal[5]);
      radio.openReadingPipe(1, canal[4]);
      endereco = SENSOR1;
      enviar.enderecoOrigem = endereco;
      enviar.enderecoDestino = MASTER;
      //dht.begin(); 
  
  #endif

  radio.startListening();  

}


void loop() {
    // limpa enviar.mensagem
    for (i = 0; i < 20; i++)
        enviar.mensagem[i] = "";
    i = 0;
    
#if radioID == MASTER
    //verifica se esta recebendo mensagem
    if (radio.available()) {     
        radio.read(&receber, sizeof(MSG)); 
        if (receber.enderecoDestino == endereco){
            if(receber.enderecoOrigem == SENSOR0){
              Serial.print("sensor_1 ");
              Serial.println(receber.mensagem[0],DEC);
            }else if(receber.enderecoOrigem == SENSOR1){
              Serial.print("sensor_2 ");
              Serial.println(receber.mensagem[0],DEC);
            }
        }
    }
  
#elif radioID == INTERMEDIARIO

    //verifica se esta recebendo mensagem       
    if (radio.available()) {           
        radio.read(&receber, sizeof(MSG));
        if (receber.enderecoDestino == MASTER) {
            enviar = receber;
            WRITEMASTER;
            if(receber.enderecoOrigem == SENSOR0){
              Serial.print("SENSOR0 -> MASTER: ");
              Serial.println(receber.mensagem[0],DEC);
            }else if(receber.enderecoOrigem == SENSOR1){
              Serial.print("SENSOR1 -> MASTER: ");
              Serial.println(receber.mensagem[0],DEC);
            }
            transmitido = false;
            
        } else if (receber.enderecoDestino == SENSOR0) {
            enviar = receber;
            WRITESENSOR0;
            Serial.print("MASTER -> SENSOR0: " + String(receber.mensagem));
            transmitido = false;
        } else if (receber.enderecoDestino == SENSOR1) {
            enviar = receber;
            WRITESENSOR1;
            Serial.print("MASTER -> SENSOR1: " + String(receber.mensagem));
            transmitido = false;
        }
    }
    
    radio.stopListening();
    while (!transmitido) {
        transmitido = radio.write(&enviar, sizeof(MSG));
        delay(100);
    }
    radio.startListening();
        
#elif (radioID == SENSOR0) 
    float t = dht.readTemperature();
    while(isnan(t)){
      t = dht.readTemperature();
    }
    
    enviar.mensagem[0] = char(int(t));
    enviar.mensagem[1] = '\0';
    Serial.println(enviar.mensagem[0],DEC);
    radio.stopListening();
   
    radio.write(&enviar, sizeof(MSG));
    radio.startListening();
    
    if (radio.available()) { //verifica se esta recebendo mensagem    
        radio.read(&receber, sizeof(MSG)); 
        if (receber.enderecoDestino == endereco)
            Serial.println("Mensagem recebida: " + String(receber.mensagem));
        else
            Serial.println("MENSAGEM PARA OUTRO NODE");
    }

#elif (radioID == SENSOR1) 
    
    enviar.mensagem[0] = char(int(sensor.readCelsius()));
    enviar.mensagem[1] = '\0';
    Serial.println(enviar.mensagem[0],DEC);
    
    radio.stopListening();
    radio.write(&enviar, sizeof(MSG));
    radio.startListening();
    
    
    if (radio.available()) { //verifica se esta recebendo mensagem    
        radio.read(&receber, sizeof(MSG)); 
        if (receber.enderecoDestino == endereco)
            Serial.println("Mensagem recebida: " + String(receber.mensagem));
        else
            Serial.println("MENSAGEM PARA OUTRO NODE");
    }
    
#endif

  delay(100);
}
