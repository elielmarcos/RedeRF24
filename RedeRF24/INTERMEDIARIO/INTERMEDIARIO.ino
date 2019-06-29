#include <RF24.h>
#include <SimpleTimer.h>

//*************** Controle do RF ***********************
#define MASTER 00
#define INTERMEDIARIO0 10
#define INTERMEDIARIO1 11
#define SENSOR0 20
#define SENSOR1 21
#define SENSOR2 22
#define SENSOR3 23

/* DEFINE O TIPO DA MENSAGEM */
#define PING 0
#define PONG 1 
#define GET  2
#define SET  3

#define radioID INTERMEDIARIO0   //Informar o ID ou NOME do dispositivo

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
  char tipo = 0;
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
  
  #if (radioID == INTERMEDIARIO0)   // INTERMEDIARIO 0
      #define WRITE_MASTER  radio.openWritingPipe(PIPE_A)
      #define WRITE_SENSOR0 radio.openWritingPipe(PIPE_F)
      #define WRITE_SENSOR1 radio.openWritingPipe(PIPE_H)
      radio.openReadingPipe(1, PIPE_B);
      radio.openReadingPipe(2, PIPE_E);
      radio.openReadingPipe(3, PIPE_G);
      enderecoOrigem = INTERMEDIARIO0;

      
  #elif (radioID == INTERMEDIARIO1)   // INTERMEDIARIO 1
      #define WRITE_MASTER  radio.openWritingPipe(PIPE_C)
      #define WRITE_SENSOR2 radio.openWritingPipe(PIPE_J)
      #define WRITE_SENSOR3 radio.openWritingPipe(PIPE_L)
      radio.openReadingPipe(1, PIPE_D);
      radio.openReadingPipe(2, PIPE_I);
      radio.openReadingPipe(3, PIPE_K);
      enderecoOrigem = INTERMEDIARIO1;
  #endif


  radio.startListening();  

}

void loop() {
  
  PKG enviar;
  PKG receber;

  while (true) {
 
    /* SOURCE INTERMEDIARIO 0 */ 
    #if (radioID == INTERMEDIARIO0)
  
        if (radio.available()) { 
          receber = ReceberPacote();  
          char tipo = receber.tipo;
          String dado = receber.dado;
          if (receber.enderecoDestino == enderecoOrigem) {
            if (tipo == PING) {
              Serial.println("PING");
              WRITE_MASTER;             // ENVIAR MEU PONG PARA O MASTER
              enviar.enderecoOrigem = enderecoOrigem;
              enviar.enderecoDestino = receber.enderecoOrigem;
              EnviarPong(enviar); 

              WRITE_SENSOR0;            // REPASSAR PING PARA OS FILHOS
              enviar.enderecoOrigem = enderecoOrigem;
              enviar.enderecoDestino = SENSOR0;
              //EnviarPing(enviar);
              
              WRITE_SENSOR1;            // REPASSAR PING PARA OS FILHOS
              enviar.enderecoOrigem = enderecoOrigem;
              enviar.enderecoDestino = SENSOR1;
              //EnviarPing(enviar);
              
            }else
            if (tipo == PONG){
              Serial.println("PONG");
              WRITE_MASTER;             // REPASSAR PONG PARA O MASTER 
              enviar.enderecoOrigem = receber.enderecoOrigem;
              enviar.enderecoDestino = MASTER;
              EnviarPong(enviar); 
            }
          } else
          if (receber.enderecoDestino == MASTER) {
            enviar = receber;
            WRITE_MASTER;
            EnviarPacote(enviar);
            Serial.println("<SENSOR -> MASTER>"); 
          }
          else
          if (receber.enderecoDestino == SENSOR0)
          {
            enviar = receber;
            WRITE_SENSOR0;
            EnviarPacote(enviar);
            Serial.println("<MASTER -> SENSOR0>"); 
          }
          else
          if (receber.enderecoDestino == SENSOR1)
          {
            enviar = receber;
            WRITE_SENSOR1;
            EnviarPacote(enviar);
            Serial.println("<MASTER -> SENSOR1>"); 
          }        
        }
  
    /* SOURCE INTERMEDIARIO 1 */ 
    #elif (radioID == INTERMEDIARIO1)
  
        if (radio.available()) { 
          receber = ReceberPacote();     
          char tipo = receber.tipo;
          String dado = receber.dado;      
          if (receber.enderecoDestino == enderecoOrigem) {
            if (tipo == PING) {
              WRITE_MASTER;             // ENVIAR MEU PONG PARA O MASTER
              enviar.enderecoOrigem = enderecoOrigem;
              enviar.enderecoDestino = receber.enderecoOrigem;
              EnviarPong(enviar); 

              WRITE_SENSOR2;            // REPASSAR PING PARA OS FILHOS
              enviar.enderecoOrigem = enderecoOrigem;
              enviar.enderecoDestino = SENSOR2;
              EnviarPing(enviar);

              WRITE_SENSOR3;            // REPASSAR PING PARA OS FILHOS
              enviar.enderecoOrigem = enderecoOrigem;
              enviar.enderecoDestino = SENSOR3;
              EnviarPing(enviar);
              
            }else
            if (tipo == PONG){
              WRITE_MASTER;             // REPASSAR PONG PARA O MASTER 
              enviar.enderecoOrigem = receber.enderecoOrigem;
              enviar.enderecoDestino = MASTER;
              EnviarPong(enviar); 
            }
          } else
          if (receber.enderecoDestino == MASTER) {
            enviar = receber;
            WRITE_MASTER;
            EnviarPacote(enviar);
            Serial.println("<SENSOR -> MASTER>"); 
          }
          else
          if (receber.enderecoDestino == SENSOR2)
          {
            enviar = receber;
            WRITE_SENSOR2;
            EnviarPacote(enviar);
            Serial.println("<MASTER -> SENSOR2>"); 
          }
          else
          if (receber.enderecoDestino == SENSOR3)
          {
            enviar = receber;
            WRITE_SENSOR3;
            EnviarPacote(enviar);
            Serial.println("<MASTER -> SENSOR3>"); 
          }        
        }
    #endif
    delay(10);
  }
}

PKG ReceberPacote() {
  PKG receber;
  radio.read(&receber, sizeof(PKG)); 
  return receber;
}

bool EnviarPacote(PKG pacote) {
  bool trasmitido = false;
  delay(50);
  radio.stopListening();  
  transmitido = radio.write( &pacote, sizeof(PKG));     
  //while(!radio.write( &pacote, sizeof(PKG)))
    //delay(20);
  radio.startListening();
  return transmitido;
}

void EnviarPing(PKG pacote) { 
  pacote.tipo = PING;
  strcpy(pacote.dado,"");
  EnviarPacote(pacote);
}

void EnviarPong(PKG pacote) { 
  pacote.tipo = PONG;
  strcpy(pacote.dado,"");
  EnviarPacote(pacote);
  Serial.println("I0 --> PONG");
}
