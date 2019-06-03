#include <RF24.h>

//*************** Controle do RF ***********************
#define radioID 2 // Master: 0, Intermediario: 1, Sensor: 2
#define MASTER 0
#define INTERMEDIARIO 1
#define SENSOR 2

RF24 radio(7,8);

byte canal[][6] = {"000ch","100ch","200ch","300ch"};

typedef struct MSG {
  char enderecoDestino;
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
  
  #if radioID == SENSOR
      radio.openWritingPipe(canal[3]);// modify canal[0] to canal[3]
      radio.openReadingPipe(1, canal[1]);
      endereco = SENSOR;
      enviar.enderecoDestino = MASTER;
  #elif radioID == MASTER
      radio.openWritingPipe(canal[2]);
      radio.openReadingPipe(1, canal[3]);
      endereco = MASTER;
      enviar.enderecoDestino = SENSOR;
  #elif radioID == INTERMEDIARIO
      #define WRITEMASTER radio.openWritingPipe(canal[3])
      #define WRITESENSOR radio.openWritingPipe(canal[1])
      radio.openReadingPipe(1, canal[0]);
      radio.openReadingPipe(2, canal[2]);
      endereco = INTERMEDIARIO;
  #endif

  radio.startListening();  

}


void loop() {
    // limpa enviar.mensagem
    for (i = 0; i < 20; i++)
        enviar.mensagem[i] = "";
    i = 0;
    
#if (radioID == SENSOR) 
    int num_transmitidos = 0;
    enviar.mensagem[0] = 't'; enviar.mensagem[1] = 'e'; enviar.mensagem[2] = 's';
    enviar.mensagem[3] = 't'; enviar.mensagem[4] = 'e'; enviar.mensagem[5] = '\0';
    radio.stopListening();
    for (i = 0; i < 100; i++) {
        if (radio.write(&enviar, sizeof(MSG)))
            num_transmitidos++;
        delay(100);
    }
    radio.startListening();
    Serial.println("Pacotes recebidos/enviados: " + String(num_transmitidos) + "/100");
    
    if (radio.available()) { //verifica se esta recebendo mensagem    
        radio.read(&receber, sizeof(MSG)); 
        if (receber.enderecoDestino == endereco)
            Serial.println("Mensagem recebida: " + String(receber.mensagem));
        else
            Serial.println("MENSAGEM PARA OUTRO NODE");
    }
    
#elif radioID == MASTER
    while(Serial.available() > 0) { // Enquanto receber algo pela serial
        caractere = Serial.read(); // Lê byte a byte
        if (caractere == '\n') { // Quando encontrar quebra de linha
            enviar.mensagem[i++] = '\0'; // Finaliza mensagem
            transmitido = false;
            break;
        }
        enviar.mensagem[i++] = caractere;
    }

    radio.stopListening();
    while (!transmitido) { // Permance enviando enquanto nao receber ack
        Serial.println("Enviando para Sensor: " + String(enviar.mensagem));
        transmitido = radio.write(&enviar, sizeof(MSG));
        delay(100);
    }
    radio.startListening();
    
    if (radio.available()) { //verifica se esta recebendo mensagem    
        radio.read(&receber, sizeof(MSG)); 
        if (receber.enderecoDestino == endereco)
            Serial.println("Mensagem recebida: " + String(receber.mensagem));
        else
            Serial.println("MENSAGEM PARA OUTRO NODE");
    }
  
#elif radioID == INTERMEDIARIO

    //verifica se esta recebendo mensagem       
    if (radio.available()) {           
        radio.read(&receber, sizeof(MSG));
        if (receber.enderecoDestino == MASTER) {
            enviar = receber;
            WRITEMASTER;
            Serial.print("SENSOR -> MASTER: " + String(receber.mensagem));
            transmitido = false;
        } else if (receber.enderecoDestino == SENSOR) {
            enviar = receber;
            WRITESENSOR;
            Serial.print("MASTER -> SENSOR: " + String(receber.mensagem));
            transmitido = false;
        }
    }
    
    radio.stopListening();
    while (!transmitido) {
        transmitido = radio.write(&enviar, sizeof(MSG));
        delay(100);
    }
    radio.startListening();
    
#endif


  delay(100);
}
