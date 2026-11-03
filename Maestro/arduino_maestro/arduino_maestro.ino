#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// ----------------------------------------------------
// 1. CONFIGURACIÓN DE PINES Y DIRECCIONES I2C
// ----------------------------------------------------
#define SLAVE_GARAGE_ADDR  0x01  // Dirección I2C Esclavo 1 (Porton, Sirena, Sensor)
#define SLAVE_LUCES_ADDR   0x02  // Dirección I2C Esclavo 2 (Luces/Pasillos)

const int PIN_POT_TRIAJE = A0;   // Potenciometro selector de triaje en Sala de Radio

// Configuracion LCD I2C (Direccion estandar 0x27, 16 columnas, 2 filas)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ----------------------------------------------------
// 2. CONFIGURACION DEL TECLADO MATRICIAL 4x4
// ----------------------------------------------------
const byte FILAS = 4;
const byte COLUMNAS = 4;
char teclas[FILAS][COLUMNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
// Conexion de pines en Arduino
byte pinesFilas[FILAS]       = {9, 8, 7, 6}; 
byte pinesColumnas[COLUMNAS] = {5, 4, 3, 2}; 

Keypad teclado = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

// ----------------------------------------------------
// 3. VARIABLES GLOBALES DE ESTADO
// ----------------------------------------------------
String claveIngresada = "";
const String CLAVE_CORRECTA = "1739";

// Niveles de Triaje: 0 = VERDE (Reposo), 1 = AMARILLO (Alerta Media), 2 = ROJO (Emergencia Alta)
int estadoTriaje = 0; 
int estadoTriajeAnterior = -1;

void setup() {
  // Inicialización del Bus I2C como
  Wire.begin(); 

  // Inicialización de la Pantalla LCD
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Bienvenidos,");
  lcd.setCursor(0, 1);
  lcd.print("Estacion Bomberos");
  delay(1500); // Mínimo 1 segundo exigido por consigna

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Grupo 6");
  delay(1500);
  
  lcd.clear();
}

void loop() 
  int valorPot = analogRead(PIN_POT_TRIAJE);
  
  // Mapeo del potenciometro (0-1023) a los 3 niveles
  if (valorPot < 340) {
    estadoTriaje = 0; // VERDE
  } else if (valorPot < 680) {
    estadoTriaje = 1; // AMARILLO
  } else {
    estadoTriaje = 2; // ROJO
  }

  if (estadoTriaje != estadoTriajeAnterior) {
    enviarComandoI2C(SLAVE_GARAGE_ADDR, estadoTriaje);
    enviarComandoI2C(SLAVE_LUCES_ADDR, estadoTriaje);
    actualizarPantallaLCD();
    estadoTriajeAnterior = estadoTriaje;
  }

  // Lectura del Teclado Matricial (Desactivación / Reset)
  char tecla = teclado.getKey();
  if (tecla) {
    if (tecla >= '0' && tecla <= '9') {
      claveIngresada += tecla;
      lcd.setCursor(0, 1);
      lcd.print("Clave: " + claveIngresada + "   ");
    } 
    else if (tecla == '#') { // '#' funciona como ENTER para confirmar
      if (claveIngresada == CLAVE_CORRECTA) {
        // Contraseña correcta: Resetea el triaje a VERDE
        estadoTriaje = 0;
        claveIngresada = "";
        lcd.clear();
        lcd.print("ACCESO CORRECTO");
        delay(1000);
        enviarComandoI2C(SLAVE_GARAGE_ADDR, 0);
        enviarComandoI2C(SLAVE_LUCES_ADDR, 0);
        actualizarPantallaLCD();
      } else {
        // Contraseña incorrecta
        claveIngresada = "";
        lcd.setCursor(0, 1);
        lcd.print("CLAVE INCORRECTA");
        delay(1000);
        actualizarPantallaLCD();
      }
    }
    else if (tecla == '*') { // '*' funciona como CLEAR
      claveIngresada = "";
      actualizarPantallaLCD();
    }
  }
}

// ----------------------------------------------------
// 4. FUNCIONES AUXILIARES
// ----------------------------------------------------

// Transmite una trama de datos a un esclavo I2C especifico
void enviarComandoI2C(byte direccionEsclavo, byte comando) {
  Wire.beginTransmission(direccionEsclavo);
  Wire.write(comando);
  Wire.endTransmission();
}

// Actualiza el texto en la pantalla segun el Triaje actual
void actualizarPantallaLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  switch (estadoTriaje) {
    case 0:
      lcd.print("TRIAJE: VERDE");
      lcd.setCursor(0, 1);
      lcd.print("Estado: Reposo");
      break;
    case 1:
      lcd.print("TRIAJE: AMARILLO");
      lcd.setCursor(0, 1);
      lcd.print("Alerta Media!");
      break;
    case 2:
      lcd.print("TRIAJE: ROJO");
      lcd.setCursor(0, 1);
      lcd.print("EMERGENCIA ALTA!");
      break;
  }
}