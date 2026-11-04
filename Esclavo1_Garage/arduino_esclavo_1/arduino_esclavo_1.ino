#include <Wire.h>
#include <Servo.h>

#define MI_DIRECCION 0x01 // Dirección I2C Esclavo Garage

// --- Asignación de Pines ---
const int PIN_SERVO  = 9;  // Control del Portón
const int PIN_SIRENA = 8;  // Sirena de Emergencia
const int PIN_TRIG   = 6;  // Trigger Ultrasonido
const int PIN_ECHO   = 7;  // Echo Ultrasonido

Servo porton;
int estadoTriaje = 0; // 0: Verde, 1: Amarillo, 2: Rojo

void setup() {
  Wire.begin(MI_DIRECCION); // Unirse al bus I2C como esclavo 0x01
  Wire.onReceive(recibirDatosI2C); // Callback de recepción
  
  porton.attach(PIN_SERVO);
  porton.write(0); // Portón cerrado inicialmente
  
  pinMode(PIN_SIRENA, OUTPUT);
  digitalWrite(PIN_SIRENA, LOW);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
}

void loop() {
  // 1. Lectura del Sensor de Distancia Ultrasonido
  long distancia = medirDistanciaCM();

  // 2. Acción según el nivel de triaje recibido del Maestro
  if (estadoTriaje == 2) { // NIVEL ROJO (Emergencia Alta)
    porton.write(90);               // Abre el portón a 90°
    digitalWrite(PIN_SIRENA, HIGH); // Activa la sirena
  } 
  else if (estadoTriaje == 1) { // NIVEL AMARILLO (Alerta Media)
    digitalWrite(PIN_SIRENA, LOW);  // Sin sirena
    if (distancia > 0 && distancia < 15) {
      porton.write(90);
    } else {
      porton.write(0);
    }
  } 
  else { // NIVEL VERDE (Reposo)
    porton.write(0);                // Portón cerrado
    digitalWrite(PIN_SIRENA, LOW);  // Sirena apagada
  }
  
  delay(100);
}

long medirDistanciaCM() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  
  long duracion = pulseIn(PIN_ECHO, HIGH, 25000);
  if (duracion == 0) return -1;
  return duracion * 0.034 / 2;
}

void recibirDatosI2C(int cuantosBytes) {
  while (Wire.available()) {
    estadoTriaje = Wire.read();
  }
}