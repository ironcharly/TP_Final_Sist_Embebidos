#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// ----------------------------------------------------
// 1. DIRECCIONES I2C DE LOS ESCLAVOS
// ----------------------------------------------------
#define SLAVE_GARAGE_ADDR  0x01  // Esclavo Garage (Sirena/Servo)
#define SLAVE_LUCES_ADDR   0x02  // Esclavo Luces (LEDs Pasillos)

const int PIN_POT_TRIAJE = A0;   // Pin analógico selector de triaje

// Pantalla LCD 16x2 vía I2C (Dirección 0x27)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ----------------------------------------------------
// 2. CONFIGURACIÓN DEL TECLADO MATRICIAL 4x4
// ----------------------------------------------------
const byte FILAS = 4;
const byte COLUMNAS = 4;
char teclas[FILAS][COLUMNAS] = {
  {'7','8','9','A'},
  {'4','5','6','B'},
  {'1','2','3','C'},
  {'*','0','#','D'}
};

// Pines D9 a D2
byte pinesFilas[FILAS]       = {9, 8, 7, 6}; 
byte pinesColumnas[COLUMNAS] = {5, 4, 3, 2}; 

Keypad teclado = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

// ----------------------------------------------------
// 3. VARIABLES GLOBALES Y ESTADOS
// ----------------------------------------------------
String claveIngresada = "";
const String CLAVE_CORRECTA = "1739"; // Clave exigida por la consigna

int estadoTriaje = 0;           // 0 = VERDE, 1 = AMARILLO, 2 = ROJO
int estadoTriajeAnterior = -1;  // Fuerza la primera actualización
bool sistemaBloqueado = false;   // Retiene el estado ROJO hasta ingresar la clave

void setup() {
  Wire.begin(); // Inicializa I2C como Maestro

  lcd.init();
  lcd.backlight();

  // --- MENSAJE DE BIENVENIDA OBLIGATORIO ---
  lcd.setCursor(0, 0);
  lcd.print("Bienvenidos,");
  lcd.setCursor(0, 1);
  lcd.print("Est.BomberosUPSO");
  delay(1500); // Mínimo 1 segundo exigido

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Grupo 6");
  delay(1500);
  
  lcd.clear();
}

void loop() {
  // 1. Lectura analógica del potenciómetro
  int valorPot = analogRead(PIN_POT_TRIAJE);
  
  // 2. Determinar el triaje según el potenciómetro (si no está bloqueado por alarma)
  if (!sistemaBloqueado) {
    if (valorPot < 340) {
      estadoTriaje = 0; // VERDE
    } else if (valorPot < 680) {
      estadoTriaje = 1; // AMARILLO
    } else {
      estadoTriaje = 2; // ROJO
      sistemaBloqueado = true; // Bloquea hasta ingresar la clave '1739'
    }
  }

  // 3. Si el estado del triaje cambió, actualiza pantalla y notifica por I2C
  if (estadoTriaje != estadoTriajeAnterior) {
    enviarComandoI2C(SLAVE_GARAGE_ADDR, estadoTriaje);
    enviarComandoI2C(SLAVE_LUCES_ADDR, estadoTriaje);
    actualizarPantallaLCD();
    estadoTriajeAnterior = estadoTriaje;
  }

  // 4. Lectura e interacción con el Teclado Matricial
  char tecla = teclado.getKey();
  if (tecla) {
    if (tecla >= '0' && tecla <= '9') {
      if (claveIngresada.length() < 4) {
        claveIngresada += tecla; // Acumula dígitos
      }
      lcd.setCursor(7, 1);
      lcd.print(claveIngresada + "    ");
    } 
    else if (tecla == '#') { // Tecla '#' confirma la clave
      if (claveIngresada == CLAVE_CORRECTA) {
        // Clave correcta: Desbloquea y resetea a VERDE
        sistemaBloqueado = false;
        estadoTriaje = 0;
        claveIngresada = "";
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ACCESO CORRECTO");
        lcd.setCursor(0, 1);
        lcd.print("SISTEMA EN VERDE");
        
        enviarComandoI2C(SLAVE_GARAGE_ADDR, 0);
        enviarComandoI2C(SLAVE_LUCES_ADDR, 0);
        delay(1500);
        
        estadoTriajeAnterior = -1; // Fuerza el refresco en pantalla
      } else {
        // Clave errónea
        claveIngresada = "";
        lcd.setCursor(0, 1);
        lcd.print("CLAVE ERRONEA!   ");
        delay(1000);
        actualizarPantallaLCD();
      }
    }
    else if (tecla == '*') { // Tecla '*' borra lo ingresado
      claveIngresada = "";
      actualizarPantallaLCD();
    }
  }

  delay(100); // Pausa para estabilizar la simulación en Proteus
}

// ----------------------------------------------------
// 4. FUNCIONES AUXILIARES
// ----------------------------------------------------

void enviarComandoI2C(byte direccionEsclavo, byte comando) {
  Wire.beginTransmission(direccionEsclavo);
  Wire.write(comando);
  Wire.endTransmission();
}

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
      lcd.print("TRIAJE: ROJO!");
      lcd.setCursor(0, 1);
      lcd.print("Clave: ");
      lcd.print(claveIngresada);
      break;
  }
}