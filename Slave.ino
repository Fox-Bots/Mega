/*
  SLAVE OBR - 4 HC-SR04 + A2a
  Publica distancias em centimetros nas variaveis A2a:
  0 = frente, 1 = esquerda, 2 = direita, 3 = tras.

  I2C:
  - Mega 2560: SDA 20, SCL 21
  - Uno/Nano:   SDA A4, SCL A5
  Todos os GNDs devem estar em comum.
*/

#include <A2a.h>

#define ENDERECO_ESCRAVA 8

#define TRIG_FRENTE    2
#define ECHO_FRENTE    3
#define TRIG_ESQUERDA  4
#define ECHO_ESQUERDA  5
#define TRIG_DIREITA   6
#define ECHO_DIREITA   7
#define TRIG_TRAS      8
#define ECHO_TRAS      9

#define VAR_FRENTE     0
#define VAR_ESQUERDA   1
#define VAR_DIREITA    2
#define VAR_TRAS       3

const byte DISTANCIA_SEM_ECO_CM = 250;
const unsigned long TIMEOUT_ECHO_US = 25000UL;
const unsigned long INTERVALO_ENTRE_PINGS_MS = 60UL;

A2a a2a;

byte distanciaFrente = DISTANCIA_SEM_ECO_CM;
byte distanciaEsquerda = DISTANCIA_SEM_ECO_CM;
byte distanciaDireita = DISTANCIA_SEM_ECO_CM;
byte distanciaTras = DISTANCIA_SEM_ECO_CM;

byte proximoSensor = 0;
unsigned long ultimoPingMs = 0;

byte medirDistanciaCm(byte trig, byte echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  unsigned long duracao = pulseIn(echo, HIGH, TIMEOUT_ECHO_US);

  if (duracao == 0) {
    return DISTANCIA_SEM_ECO_CM;
  }

  unsigned long distancia = duracao / 58UL;

  if (distancia > DISTANCIA_SEM_ECO_CM) {
    return DISTANCIA_SEM_ECO_CM;
  }

  return (byte)distancia;
}

void publicarDistancias() {
  a2a.varWireWrite(VAR_FRENTE, distanciaFrente);
  a2a.varWireWrite(VAR_ESQUERDA, distanciaEsquerda);
  a2a.varWireWrite(VAR_DIREITA, distanciaDireita);
  a2a.varWireWrite(VAR_TRAS, distanciaTras);
}

void receberDados(int quantidade) {
  a2a.receiveData();
}

void enviarDados() {
  a2a.sendData();
}

void setup() {
  pinMode(TRIG_FRENTE, OUTPUT);
  pinMode(ECHO_FRENTE, INPUT);
  pinMode(TRIG_ESQUERDA, OUTPUT);
  pinMode(ECHO_ESQUERDA, INPUT);
  pinMode(TRIG_DIREITA, OUTPUT);
  pinMode(ECHO_DIREITA, INPUT);
  pinMode(TRIG_TRAS, OUTPUT);
  pinMode(ECHO_TRAS, INPUT);

  a2a.begin(ENDERECO_ESCRAVA);
  a2a.onReceive(receberDados);
  a2a.onRequest(enviarDados);

  publicarDistancias();
}

void loop() {
  if (millis() - ultimoPingMs < INTERVALO_ENTRE_PINGS_MS) {
    return;
  }

  ultimoPingMs = millis();

  // Um sensor por vez reduz a interferencia entre os HC-SR04.
  switch (proximoSensor) {
    case 0:
      distanciaFrente = medirDistanciaCm(TRIG_FRENTE, ECHO_FRENTE);
      break;
    case 1:
      distanciaEsquerda = medirDistanciaCm(TRIG_ESQUERDA, ECHO_ESQUERDA);
      break;
    case 2:
      distanciaDireita = medirDistanciaCm(TRIG_DIREITA, ECHO_DIREITA);
      break;
    default:
      distanciaTras = medirDistanciaCm(TRIG_TRAS, ECHO_TRAS);
      break;
  }

  proximoSensor = (proximoSensor + 1) % 4;
  publicarDistancias();
}
