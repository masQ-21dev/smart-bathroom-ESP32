#include <ESP32Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "Tayowawik";
const char* password = "Wawiktayo";

// inisialisasi Bot Token
#define BOTtoken "6236249395:AAFYKmTJUcZmcOVPUIQdsSQf5O2mtv5yqVg"  // Bot Token dari BotFather

// chat id dari @myidbot
#define CHAT_ID "1365262211"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

LiquidCrystal_I2C lcd(0x27, 16, 2);

Servo myservo;

const int btn1 = 2;
const int btn2 = 4;

const int servo1Pin = 15;

const int relay1 = 25;
const int relay2 = 26;

const int TrigPin = 13;
const int echoPin = 12;

const int ldrPin = 14;

const int pirPin = 27;

// dalam bak mandi in cm 
int bakMandi = 12;

int lampValue = 0;
int pompavalue = 0;

int BtnlampValue = 0;
int Btnpompavalue = 0;

int pirStatus = 0;

float jarak;

// pesan status
String pesanAda = "sedang ada orang";
String pesanTidakAda = "tidak ada orang";

void setup(){
    myservo.attach(servo1Pin);

    lcd.init();
    lcd.backlight();                
    lcd.setCursor(0,1); 
    lcd.print("----------------");

    pinMode(btn1, INPUT);
    pinMode(btn2, INPUT);

    pinMode(relay1, OUTPUT);
    pinMode(relay2, OUTPUT);

    pinMode(TrigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    pinMode(ldrPin, INPUT);

    pinMode(pirPin, INPUT);

    Serial.begin(9600);

    // Koneksi Ke Wifi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    #ifdef ESP32
        client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
    #endif
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}

void onLamp(){
    digitalWrite(relay2, HIGH);
    lampValue = 1;
}
void offLamp(){
    digitalWrite(relay2, LOW);
    lampValue = 0;
}

void onPompa(){
    digitalWrite(relay1, HIGH);
    pompavalue = 1;
}

void offPompa(){
    digitalWrite(relay1, LOW);
    pompavalue = 0;
}


int btnValue(int input, String text){
    if (text == "lampu"){
        if(input == 1){
            BtnlampValue++;
            if(BtnlampValue > 1) {
                BtnlampValue = 0;
            }
            delay(250);
        }
        return lampValue;
    }else{
        if(input == 1){
            Btnpompavalue++;
            if(Btnpompavalue > 1) {
                Btnpompavalue = 0;
            }
            delay(250);
        }
        return pompavalue;
    }   
}

 void handleNewMessages(int numNewMessages) {
    Serial.println("handleNewMessages");
    Serial.println(String(numNewMessages));

    for (int i=0; i<numNewMessages; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        if (chat_id != CHAT_ID){
            bot.sendMessage(chat_id, "Unauthorized user", "");
            continue;
        }

        String text = bot.messages[i].text;
        Serial.println(text);
        String from_name = bot.messages[i].from_name;

        if (text == "/start") {
            String control = "Selamat Datang, " + from_name + ".\n";
            control += "Gunakan Commands Di Bawah Untuk Control Lednya.\n\n";
            control += "/cek_status Untuk mengecek ada orang atau tidak\n";
            control += "/isi_air untuk mengisi air ke bak mandi\n";
            control += "/cek_air Untuk memberhentikan proses pengisian \n";
            control += "/lampu_Mati Untuk Matikan lampu \n";
            control += "/lampu_Nyala Untuk Menyalakan lampu \n";
            bot.sendMessage(chat_id, control, "");
        }

        if (text == "/cek_status") {
            bot.sendMessage(chat_id, "status kamar mandi\n", "");
            if (pirStatus == 1) {
                bot.sendMessage(chat_id, pesanAda, "");
            } else {
                bot.sendMessage(chat_id, pesanTidakAda, "");
            }
            
        }
        
        if (text == "/cek_air") {
            bot.sendMessage(chat_id, "kapasitas air", "");
            bot.sendMessage(chat_id, String((jarak/bakMandi)* 100), "");
        }
        if (text == "/isi_air") {
            if(pompavalue == 0) {
                onPompa();
                bot.sendMessage(chat_id, "isi air", "");
            }
            
        }
        if (text == "/lampu_Mati") {
            if (lampValue != 1 ){
                offLamp();
                bot.sendMessage(chat_id, "lampu di matikan", "");
            } else {
                bot.sendMessage(chat_id, "lampu sudah matikan", "");
            }
            
        }
        if (text == "/lampu_Nyala") {
            if (lampValue == 1 ){
                offLamp();
                bot.sendMessage(chat_id, "lampu di dihidupkan", "");
            }
            else {
                bot.sendMessage(chat_id, "lampu sudah hidup", "");
            }
        }
    }
}
              

float range(){
    //jarak in cm
    long durasi = pulseIn(TrigPin, HIGH);
    float jarak = durasi * 0.034/2;

    return jarak;
}

void lampu(){
    if(analogRead(ldrPin)  3000){
        if(lampValue != 1) {
            onLamp();
        }
    } 
}

void pompa(int range) {
    if(range < bakMandi/5) {
        onPompa();
        Serial.print("pompa menyala");
    } if (range >= bakMandi/9) {
        offPompa();
        Serial.print("pompa mati");
    }
}

void loop(){
    //jarak in cm
    jarak = range();

    BtnlampValue = btnValue(digitalRead(btn1), "lampu");
    if(BtnlampValue == 1 && lampValue != 1){
        onLamp();
    }else if(BtnlampValue != 1 && lampValue == 1){
        onLamp();
    }
    Btnpompavalue = btnValue(digitalRead(btn2), "pompa");
    if(Btnpompavalue == 1 && pompavalue != 1){
        onPompa();
    }else if(Btnpompavalue != 1 && pompavalue == 1){
        onPompa();
    }

    pompa(jarak);

    if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
    }


    if (digitalRead(pirPin)) {
        pirStatus = 1;
        lcd.setCursor(0,0);
        lcd.print(pesanAda);
        myservo.write(90);

        lampu();
    } else {
        pirStatus = 0;
        lcd.setCursor(0,0);
        lcd.print(pesanTidakAda);
        myservo.write(0);
        if(lampValue == 1){
            offLamp();
        }
    }
}





