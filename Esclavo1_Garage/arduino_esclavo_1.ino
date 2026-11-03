#include <Wire.h>
#include <Servo.h>

#define MI_DIRECCION 0x01

// Pines
const int PIN_SERVO = 9;
const int PIN_SIRENA = 8;
const int PIN_TRIG = 6;
const int PIN_ECHO = 7;

Servo porton;
int estadoTriaje = 0; // 0: Verde, 1: Amarillo, 2: Rojo

void setup() {
  Wire.begin(MI_DIRECCION);
  Wire.onReceive(recibirDatosI2C); // Función que se ejecuta al recibir datos del Maestro
  
  porton.attach(PIN_SERVO);
  porton.write(0); // Portón cerrado inicialmente
  
  pinMode(PIN_SIRENA, OUTPUT);
  digitalWrite(PIN_SIRENA, LOW);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
}

void loop() {
  // Acción según el nivel de triaje recibido
  if (estadoTriaje == 2) { // NIVEL ROJO (Emergencia Alta)
    porton.write(90);             // Abre el portón a 90°
    digitalWrite(PIN_SIRENA, HIGH); // Activa la sirena
  } else { // NIVEL VERDE o AMARILLO
    porton.write(0);              // Mantiene/Cierra portón
    digitalWrite(PIN_SIRENA, LOW);  // Desactiva sirena
  }
  
  delay(100);
}

// Función callback I2C
void recibirDatosI2C(int cuantosBytes) {
  while (Wire.available()) {
    estadoTriaje = Wire.read(); // Recibe el entero enviada por el Maestro
  }
}