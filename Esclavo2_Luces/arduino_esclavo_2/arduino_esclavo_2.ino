#include <Wire.h>

#define MI_DIRECCION 0x02 // Dirección I2C asignada al Esclavo de Luces

// --- Asignación de Pines para los LEDs de Triaje ---
const int LED_VERDE    = 10;
const int LED_AMARILLO = 11;
const int LED_ROJO     = 12;

int estadoTriaje = 0; // 0 = VERDE, 1 = AMARILLO, 2 = ROJO

void setup() {
  // Inicialización del Bus I2C con dirección 0x02
  Wire.begin(MI_DIRECCION);
  Wire.onReceive(recibirDatosI2C); // Callback cuando el Maestro transmite
  
  // Configuración de pines de salida
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
}

void loop() {
  // 1. Apagar todos los LEDs antes de refrescar el estado
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_ROJO, LOW);

  // 2. Encender únicamente el indicador correspondiente al triaje actual
  switch (estadoTriaje) {
    case 0: // VERDE (Estado Normal / Reposo)
      digitalWrite(LED_VERDE, HIGH);
      break;

    case 1: // AMARILLO (Alerta Media)
      digitalWrite(LED_AMARILLO, HIGH);
      break;

    case 2: // ROJO (Emergencia Alta)
      digitalWrite(LED_ROJO, HIGH);
      break;
  }
  
  delay(100);
}

// Función callback que captura la orden enviada por el Maestro por I2C
void recibirDatosI2C(int cuantosBytes) {
  while (Wire.available()) {
    estadoTriaje = Wire.read(); // Recibe 0, 1 o 2
  }
}