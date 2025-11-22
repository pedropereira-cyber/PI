// Bibliotecas necessárias 
#include <SPI.h> 
#include <MFRC522.h> 
#include <Preferences.h> 
#include <ESP32Servo.h> 
#include <LiquidCrystal_I2C.h> 
// Definições de pinos 
#define SS_PIN 5 
#define RST_PIN 4 
#define LED_VERDE 12 
#define LED_VERMELHO 13 
#define SERVO_PIN 26 
#define BUTTON_PIN 14 
// Constantes 
#define ACESSO_COMUM 1 
#define ACESSO_ADMIN 2 
// Instâncias de objetos 
MFRC522 rfid(SS_PIN, RST_PIN); 
Preferences memoria; 
Servo servo; 
LiquidCrystal_I2C lcd(0x27, 16, 2); 
// Variáveis globais 
String cardID = ""; 
int tentativas = 0; 
// Funções auxiliares 
bool RFIDRead() { 
if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return false; 
cardID = ""; 
for (byte i = 0; i < rfid.uid.size; i++) { 
cardID += String(rfid.uid.uidByte[i] < 0x10 ? "0" : ""); 
cardID += String(rfid.uid.uidByte[i], HEX); 
} 
cardID.toUpperCase(); 
rfid.PICC_HaltA(); 
return true; 
} 
int verificarCartao(String id) { 
String valor = memoria.getString(id.c_str(), ""); 
if (valor == "1") return ACESSO_COMUM; 
if (valor == "2") return ACESSO_ADMIN; 
return 0; 
} 
void SaveCardID(String id, String nivel) { 
memoria.putString(id.c_str(), nivel); 
} 
void DeleteCardID(String id) { 
memoria.remove(id.c_str()); 
} 
void concederAcesso() { 
digitalWrite(LED_VERDE, HIGH); 
lcd.clear(); 
lcd.print("Acesso Liberado"); 
servo.write(180); 
delay(2000); 
servo.write(0); 
digitalWrite(LED_VERDE, LOW); 
lcd.clear(); 
} 
void negarAcesso() { 
digitalWrite(LED_VERMELHO, HIGH); 
lcd.clear(); 
lcd.print("Acesso Negado"); 
delay(2000); 
digitalWrite(LED_VERMELHO, LOW); 
lcd.clear(); 
} 
void modoAdministrador() { 
lcd.clear(); 
lcd.print("Modo ADMIN"); 
lcd.setCursor(0, 1); 
lcd.print("Aprox. um cartao"); 
while (true) { 
    if (!RFIDRead()) continue; 
    int nivel = verificarCartao(cardID); 
 
    if (nivel == ACESSO_COMUM || nivel == ACESSO_ADMIN) { 
      DeleteCardID(cardID); 
      lcd.clear(); 
      lcd.print("Cartao removido"); 
    } else { 
      if (digitalRead(BUTTON_PIN)) { 
        SaveCardID(cardID, "2"); 
        lcd.clear(); 
        lcd.print("Admin cadastrado"); 
      } else { 
        SaveCardID(cardID, "1"); 
        lcd.clear(); 
        lcd.print("Comum cadastrado"); 
      } 
    } 
    delay(2000); 
    lcd.clear(); 
    break; 
  } 
} 
 
void setup() { 
  Serial.begin(115200); 
  SPI.begin(); 
  rfid.PCD_Init(); 
  memoria.begin("Cards", false); 
  pinMode(LED_VERDE, OUTPUT); 
  pinMode(LED_VERMELHO, OUTPUT); 
  pinMode(BUTTON_PIN, INPUT); 
  servo.attach(SERVO_PIN); 
  servo.write(0); 
  lcd.init(); 
  lcd.backlight(); 
  lcd.setCursor(0, 0); 
  lcd.print("Sistema Pronto"); 
  delay(2000); 
  lcd.clear(); 
} 
 
void loop() { 
  if (digitalRead(BUTTON_PIN)) { 
    lcd.clear(); 
    lcd.print("Verif. Admin..."); 
    while (!RFIDRead()); 
    if (verificarCartao(cardID) == ACESSO_ADMIN) { 
      modoAdministrador(); 
    } else { 
      lcd.clear(); 
      lcd.print("Sem permissao"); 
      delay(2000); 
      lcd.clear(); 
    } 
  } 
if (!RFIDRead()) return; 
int nivel = verificarCartao(cardID); 
if (nivel != 0) { 
concederAcesso(); 
tentativas = 0; // zera tentativas 
} else { 
negarAcesso(); 
tentativas++; 
} 
} 
