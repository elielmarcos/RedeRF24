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

#define radioID MASTER   //Informar o ID ou NOME do dispositivo

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
 *                HIERAEQUIA
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

byte canal[][6] = {"A00ch","B00ch","C00ch","D00ch","E00ch","F00ch","G00ch","H00ch","I00ch","J00ch","K00ch","L00ch"};

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

#if (radioID == MASTER)
    #define CAMADAS 3           // Numero de camadas (3)
    #define MAIOR_NODES 4       // Maior quantidade de nodes em qualquer camada (4 nodes na ultima camada)
    int ativos[CAMADAS][MAIOR_NODES];   
    SimpleTimer timer;
    void MasterPing();
    void AtivarNode(int ID);
    bool NodeAtivo(int ID);
#endif

void setup() {

  Serial.begin(115200);           // Inicia comunicação serial
  
  //*************** Controle do RF ***********************
  radio.begin();                  // Inicia o RF
  radio.setPALevel(RF24_PA_HIGH); // Configura RF potência máxima 
  
  #if (radioID == MASTER)     // MASTER
      #define WRITE_INTERMEDIARIO0 radio.openWritingPipe(PIPE_B)
      #define WRITE_INTERMEDIARIO1 radio.openWritingPipe(PIPE_D)
      radio.openReadingPipe(1, PIPE_A);
      radio.openReadingPipe(2, PIPE_C);
      enderecoOrigem = MASTER;
      for (int i = 0; i < CAMADAS; i++)         // para o numero de camadas
        for (int j = 0; j < MAIOR_NODES; j++)   // para maior quantidade de nodes em qualquer camada
          ativos[i][j] = 0;                     // inicializa nodes desativados
      timer.setInterval(5000,MasterPing);       // tempo de intervalo para enviar o ping

      
  #elif (radioID == INTERMEDIARIO0)   // INTERMEDIARIO 0
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

      
  #elif (radioID == SENSOR0)   // SENSOR 0
      radio.openWritingPipe(PIPE_D);
      radio.openReadingPipe(1, PIPE_F);
      enderecoOrigem = SENSOR0;

      
  #elif (radioID == SENSOR1)   // SENSOR 1
      radio.openWritingPipe(PIPE_G);
      radio.openReadingPipe(1, PIPE_H);
      enderecoOrigem = SENSOR1;

      
  #elif (radioID == SENSOR2)   // SENSOR 2
      radio.openWritingPipe(PIPE_I);
      radio.openReadingPipe(1, PIPE_J);
      enderecoOrigem = SENSOR2;

      
  #elif (radioID == SENSOR3)   // SENSOR 3
      radio.openWritingPipe(PIPE_K);
      radio.openReadingPipe(1, PIPE_L);
      enderecoOrigem = SENSOR3;
      
  #endif


  radio.startListening();  

}


void loop() {
  
  PKG enviar;
  PKG receber;

  while (true) {
    
    /* SOURCE MASTER */ 
    #if (radioID == MASTER)

        timer.run(); // polling do SimpleTimer

        //verifica se esta recebendo dado       
        if (radio.available()) { 
          receber = ReceberPacote();
          String tipo = receber.tipo;
          String dado = receber.dado;          
          if (receber.enderecoDestino == enderecoOrigem) {
            if (tipo == "PONG") {
              AtivarNode(receber.enderecoOrigem); 
              Serial.println("COMUNICACAO ABERTA COM NODE: "+receber.enderecoOrigem);
            }
            else 
            if ((tipo == "SET") && NodeAtivo(receber.enderecoOrigem)) {
              if (receber.enderecoOrigem == SENSOR0)
                Serial.println("<SENSOR 0>: "+dado);
              else 
              if (receber.enderecoOrigem == SENSOR1)
                Serial.println("<SENSOR 1>: "+dado);
              else
              if (receber.enderecoOrigem == SENSOR2)
                Serial.println("<SENSOR 2>: "+dado);
              else
              if (receber.enderecoOrigem == SENSOR3)
                Serial.println("<SENSOR 3>: "+dado);
              else Serial.println("<ERRO>");
            }else Serial.println("<ERRO>"); 
          } else Serial.println("MENSAGEM PARA OUTRO NODE");
        }

        // Se I0 esta ativo, então se S0 esta ativo (solicita valor), então se S1 esta ativo (solicita valor) 
        if (NodeAtivo(INTERMEDIARIO0)) {
          WRITE_INTERMEDIARIO0;
          if (NodeAtivo(SENSOR0)) {
            enviar.enderecoOrigem = enderecoOrigem;
            enviar.enderecoDestino = SENSOR0;
            strcpy(enviar.tipo,"GET");
            strcpy(enviar.dado,"");
            EnviarPacote(enviar); 
          }
          if (NodeAtivo(SENSOR1)) {
            enviar.enderecoOrigem = enderecoOrigem;
            enviar.enderecoDestino = SENSOR1;
            strcpy(enviar.tipo,"GET");
            strcpy(enviar.dado,"");
            EnviarPacote(enviar); 
          }
        }

        // Se I1 esta ativo, então se S2 esta ativo (solicita valor), então se S3 esta ativo (solicita valor)
        if (NodeAtivo(INTERMEDIARIO1)) {
          WRITE_INTERMEDIARIO1;
          if (NodeAtivo(SENSOR2)) {
            enviar.enderecoOrigem = enderecoOrigem;
            enviar.enderecoDestino = SENSOR2;
            strcpy(enviar.tipo,"GET");
            strcpy(enviar.dado,"");
            EnviarPacote(enviar); 
          }
          if (NodeAtivo(SENSOR3)) {
            enviar.enderecoOrigem = enderecoOrigem;
            enviar.enderecoDestino = SENSOR3;
            strcpy(enviar.tipo,"GET");
            strcpy(enviar.dado,"");
            EnviarPacote(enviar); 
          }  
        } 
  
    
    /* SOURCE INTERMEDIARIO 0 */ 
    #elif (radioID == INTERMEDIARIO0)
  
        if (radio.available()) { 
          receber = ReceberPacote();  
          String tipo = receber.tipo;
          String dado = receber.dado;         
          if (receber.enderecoDestino == enderecoOrigem) {
            if (tipo == "PING") {
              WRITE_MASTER;             // ENVIAR MEU PONG PARA O MASTER
              enviar.enderecoOrigem = enderecoOrigem;
              enviar.enderecoDestino = receber.enderecoOrigem;
              EnviarPong(enviar); 

              WRITE_SENSOR0;            // REPASSAR PING PARA OS FILHOS
              enviar.enderecoOrigem = enderecoOrigem;
              enviar.enderecoDestino = SENSOR0;
              EnviarPing(enviar);

              WRITE_SENSOR1;            // REPASSAR PING PARA OS FILHOS
              enviar.enderecoOrigem = enderecoOrigem;
              enviar.enderecoDestino = SENSOR1;
              EnviarPing(enviar);
              
            }else
            if (tipo == "PONG"){
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
          String tipo = receber.tipo;
          String dado = receber.dado;      
          if (receber.enderecoDestino == enderecoOrigem) {
            if (tipo == "PING") {
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
            if (tipo == "PONG"){
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

    /* SOURCE SENSOR 0, 1, 2 ou 3 */ 
    #elif (radioID == SENSOR0 || radioID == SENSOR1 || radioID == SENSOR2 || radioID == SENSOR3)

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
              strcpy(enviar.dado,"123456");
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


#if (radioID == MASTER)

    void MasterPing() {
      PKG pacote;

      for (int i = 0; i < CAMADAS; i++)         // para o numero de camadas
        for (int j = 0; j < MAIOR_NODES; j++)   // para maior quantidade de nodes em qualquer camada
            if (ativos[i][j] > 0) 
              ativos[i][j]--;                  
      
      WRITE_INTERMEDIARIO0;
      pacote.enderecoOrigem = enderecoOrigem; 
      pacote.enderecoDestino = INTERMEDIARIO0;
      EnviarPing(pacote);
      
      WRITE_INTERMEDIARIO1;
      pacote.enderecoOrigem = enderecoOrigem; 
      pacote.enderecoDestino = INTERMEDIARIO1;
      EnviarPing(pacote);
    }


    void AtivarNode(int ID) {
      ativos[int(ID /10)][ID % 10] = 3; 
    }

    bool NodeAtivo(int ID) {
      return (ativos[int(ID /10)][ID % 10] > 0 ? true : false); 
    }
    
#endif
