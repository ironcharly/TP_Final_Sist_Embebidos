#include <Wire.h>

#define MI_DIRECCION 0x02

// Pines de LEDs para el Semaforo de Triaje
const int LED_VERDE = 10;
const int LED_AMARILLO = 11;
const int LED_ROJO = 12;

int estadoTriaje = 0;

void setup() {
  Wire.begin(MI_DIRECCION);
  Wire.onReceive(recibirDatosI2C);
  
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
}

void loop() {
  // Apaga todos antes de actualizar
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_ROJO, LOW);

  switch (estadoTriaje) {
    case 0: // VERDE
      digitalWrite(LED_VERDE, HIGH);
      break;
    case 1: // AMARILLO
      digitalWrite(LED_AMARILLO, HIGH);
      break;
    case 2: // ROJO
      digitalWrite(LED_ROJO, HIGH);
      break;
  }
  delay(100);
}

void recibirDatosI2C(int cuantosBytes) {
  while (Wire.available()) {
    estadoTriaje = Wire.read();
  }
}