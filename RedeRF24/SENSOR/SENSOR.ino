#include <RF24.h>
#include <SimpleTimer.h>
#include "DHT.h"

//*************** Controle do RF ***********************
#define SENSOR0 20
#define SENSOR1 21
#define SENSOR2 22
#define SENSOR3 23

//*************** Configuracoes DHT ********************
#define DHTPIN A0 // pino conectado no Arduino
#define DHTTYPE DHT11 // DHT 11
 
// Conecte pino 1 do sensor (esquerda) ao +5V
// Conecte pino 2 do sensor ao pino de dados definido em seu Arduino
// Conecte pino 4 do sensor ao GND
DHT dht(DHTPIN, DHTTYPE);

#define radioID SENSOR0   //Informar o ID ou NOME do dispositivo

/*             T  A  B  L  E        P  I  P  E  S
 *  
 *                          R E A D  
 *          |  M  | I0  | I1  | S0  | S1  | S2  | S3  |
 *     -----|-----|-----|-----|-----|-----|-----|-----|
 *       M  |     |  B  |  D  |     |     |     |     |
 *     -----|-----|-----|-----|-----|-----|-----|-----|
 *       I0 |  A  |     |     |  F  |  H  |     |     |
 *  W  -----|-----|-----|-----|-----|-----|-----|-----|
 *  R    I1 |  C  |     |     |     |     |  j  |  L  |         
 *  I  -----|-----|-----|-----|-----|-----|-----|-----|
 *  T    S0 |     |  E  |     |     |     |     |     |
 *  E  -----|-----|-----|-----|-----|-----|-----|-----|
 *       S1 |     |  G  |     |     |     |     |     |
 *     -----|-----|-----|-----|-----|-----|-----|-----|
 *       S2 |     |     |  I  |     |     |     |     |
 *     -----|-----|-----|-----|-----|-----|-----|-----|
 *       S3 |     |     |  K  |     |     |     |     |
 *     -----|-----|-----|-----|-----|-----|-----|-----|  
 *     
 *     
 *                HIERARQUIA
 *            
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
 *  
 */

RF24 radio(7,8);

byte canal[][6] = {"000ch","100ch","200ch","300ch","400ch","500ch","600ch","700ch","800ch","900ch","A00ch","B00ch"};

#define PIPE_A canal[0]
#define PIPE_B canal[1]
#define PIPE_C canal[2]
#define PIPE_D canal[3]
#define PIPE_E canal[4]
#define PIPE_F canal[5]
#define PIPE_G canal[6]
#define PIPE_H canal[7]
#define PIPE_I canal[8]
#define PIPE_J canal[9]
#define PIPE_K canal[10]
#define PIPE_L canal[11]

typedef struct PKG {
  char enderecoOrigem;
  char enderecoDestino;
  char tipo[20] = "";
  char dado[20] = "";
} PKG;

char caractere;
char enderecoOrigem;
char ACK = false;
bool validado = false;
bool transmite = false;
bool transmitido = true;

PKG ReceberPacote();
bool EnviarPacote(PKG pacote);
void EnviarPing(PKG pacote);
void EnviarPong(PKG pacote);

void setup() {

  Serial.begin(115200);           // Inicia comunicação serial
  
  //*************** Controle do RF ***********************
  radio.begin();                  // Inicia o RF
  radio.setPALevel(RF24_PA_HIGH); // Configura RF potência máxima 
  
  #if (radioID == SENSOR0)   // SENSOR 0
      radio.openWritingPipe(PIPE_D);
      radio.openReadingPipe(1, PIPE_F);
      enderecoOrigem = SENSOR0;
      dht.begin();
      
  #elif (radioID == SENSOR1)   // SENSOR 1
      radio.openWritingPipe(PIPE_G);
      radio.openReadingPipe(1, PIPE_H);
      enderecoOrigem = SENSOR1;
      dht.begin();
      
  #elif (radioID == SENSOR2)   // SENSOR 2
      radio.openWritingPipe(PIPE_I);
      radio.openReadingPipe(1, PIPE_J);
      enderecoOrigem = SENSOR2;
      dht.begin();
      
  #elif (radioID == SENSOR3)   // SENSOR 3
      radio.openWritingPipe(PIPE_K);
      radio.openReadingPipe(1, PIPE_L);
      enderecoOrigem = SENSOR3;
      dht.begin();
  #endif


  radio.startListening();  

}


void loop() {
  
  PKG enviar;
  PKG receber;

  while (true) {
    /* SOURCE SENSOR 0, 1, 2 ou 3 */ 
    #if (radioID == SENSOR0 || radioID == SENSOR1 || radioID == SENSOR2 || radioID == SENSOR3)

        if (radio.available()) { 
          receber = ReceberPacote(); 
          String tipo = receber.tipo;
          String dado = receber.dado;          
          if (receber.enderecoDestino == enderecoOrigem) {
            if (tipo == "PING") { // ENVIAR MEU PONG PARA QUEM ENVIOU PING
              enviar.enderecoOrigem = enderecoOrigem;
              enviar.enderecoDestino = receber.enderecoOrigem;
              EnviarPong(enviar);
            }
            else
            if (tipo == "GET") { // ENVIAR DADO PARA QUEM SOLICITOU O GET
              enviar.enderecoOrigem = enderecoOrigem;
              enviar.enderecoDestino = receber.enderecoOrigem;
              strcpy(enviar.tipo,"SET");
              //strcpy(enviar.dado, String(dht.readTemperature()));
              strcpy(enviar.dado, "0");
              EnviarPacote(enviar);
            } else Serial.println("<ERRO>"); 
          }else Serial.println("MENSAGEM PARA OUTRO NODE");      
        }  
        
    #endif
  
  
    delay(10);
  }
}

PKG ReceberPacote() {
  PKG receber;
  radio.read( &receber, sizeof(PKG)); 
  return receber;
}

bool EnviarPacote(PKG pacote) {
  bool trasmitido = false;
  radio.stopListening();               
  transmitido = radio.write( &pacote, sizeof(PKG) );
  radio.startListening();
  Serial.println("foi: " + String(pacote.tipo));
  return transmitido;
}

void EnviarPing(PKG pacote) { 
  strcpy(pacote.tipo,"PING");
  strcpy(pacote.dado,"");
  EnviarPacote(pacote);
}

void EnviarPong(PKG pacote) { 
  strcpy(pacote.tipo,"PONG");
  strcpy(pacote.dado,"");
  EnviarPacote(pacote);
}
