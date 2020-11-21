/*******************************************************************************
Example 01 m5pow IP5306 for M5Stack

                                          Copyright (c) 2020 Wataru KUNINO
********************************************************************************
【参考文献】

Arduino IDE 開発環境イントール方法：
    https://github.com/m5stack/M5Stack/blob/master/docs/getting_started_ja.md
    https://docs.m5stack.com/#/en/related_documents/Arduino_IDE

M5Stack Arduino Library API 情報：
    https://docs.m5stack.com/#/ja/api
    https://docs.m5stack.com/#/en/arduino/arduino_api
    
Datasheet; injoinic Crop. IP5306 寄存器文档 V1.21
    SYS_CTL0
        7:6 "10"                // reserved ※
        5   Boost enable        // 通常は"1"
        4   Charger enable      // Start charging
        3   "1"                 // reserved ※
        2   AutoBootOnLoad      // auto start function.
        1   PowerBoostKeepOn    // always boost output mode.
        0   PowerBtnEn          // to accept the power button.
    SYS_CTL1
        7   PowerBoostOnOff     // Press and hold to turn on / off.
        6   PowerWLEDSet        // to turn on the power LED
        5   PowerBoostSet       // ON / OFF in one short press.
        4:3 "11"                // reserved ※
        2   PowerVin            // the supply off, to turn on the power again.
        1   "0"                 // reserved ※
        0   LowPowerShutdown    // Enable energy saving shutdown function.
    SYS_CTL2
        7:4 "0000"              // reserved ※
        3:2 LowPowerShutdownTime// waiting time until the power is turned off.
        1:0 "00"                // reserved ※

    ※注意：reserved については工場出荷時の内容から書き換えないこと。
    　また、上記が全てのM5Stackで共通とは限りません。
*******************************************************************************/

#include <M5Stack.h>                            // M5Stack用ライブラリ
#include <Wire.h>                               // I2C通信用ライブラリ

bool PowerBoostOnOff  = false;  // Press and hold to turn on / off.
bool PowerBoostSet    = false;  // ON / OFF in one short press.
bool PowerBoostKeepOn = true;   // always boost output mode.
bool PowerVin         = true;   // the supply off, to turn on the power again.
bool PowerWLEDSet     = false;  // to turn on the power LED
bool PowerBtnEn       = false;  // to accept the power button.
bool AutoBootOnLoad   = true;   // auto start function.
bool Charge           = true;   // Start charging
bool KeepLightLoad    = true;   // [deprecated] the current is small, not automatically shutdown
bool LowPowerShutdown = true;   // [deprecated] Enable energy saving shutdown function.

uint8_t progressBar = 100;

void printb(byte reg){
    for(int i = 7; i >= 0; i--){
        M5.Lcd.printf("%0d",(reg >> i) & 0x01);
    }
    M5.Lcd.print(' ');
}

int8_t IP5306read(byte reg){
    byte ret = 0xFF;
    Wire.beginTransmission(0x75);               // IP5306(0x75)との通信を開始
    Wire.write(reg);                            // レジスタ番号を指定
    if(Wire.endTransmission()==0){              // IP5306(0x75)との通信を継続
        Wire.requestFrom(0x75, 1);              // 2バイトのデータを要求
        if(Wire.available() >= 1){              // 2バイト以上を受信
            ret = (int16_t)Wire.read();         // 1バイト目を変数tempの下位へ
        }
    }
    return ret;
}

void IP5306write(byte reg, byte data){
    Wire.beginTransmission(0x75);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

void setup(){                                   // 起動時に一度だけ実行する関数
    M5.begin();                                 // M5Stack用ライブラリの起動
    
    M5.Power.begin();
    if(!M5.Power.canControl()) {
        M5.Lcd.print("The older IP5306 doesn't support I2C.");
        while(1) delay(100);
    }
    M5.Lcd.clear(BLACK);                        // LCDを消去
    M5.Lcd.setTextColor(WHITE,BLACK);
    M5.Lcd.setCursor(0, 0, 2);
    M5.Lcd.printf("m5pow IP5306 monitor for M5Stack");
    
    /***************************************************************************
    下記のコメント化を解除するとIP5306を初期設定に戻します
    【注意】M5Stackのバージョンによっては動作不具合が生じるかもしれません
    　　　　設定値を十分に検証のうえ、自己責任で利用してください
    ***************************************************************************/
    // IP5306write(0x00,0xBE);                  // SYS_CTL0
    // IP5306write(0x01,0x1D);                  // SYS_CTL1
    // IP5306write(0x02,0x00);                  // SYS_CTL2
    
    /* IP5306の設定をM5StackのAPIを使って書き込みます */
    M5.Power.setPowerBoostOnOff(PowerBoostOnOff);
    M5.Power.setPowerBoostSet(PowerBoostSet);
    M5.Power.setPowerBoostKeepOn(PowerBoostKeepOn);
    M5.Power.setPowerVin(PowerVin);
    M5.Power.setPowerWLEDSet(PowerWLEDSet);
    M5.Power.setPowerBtnEn(PowerBtnEn);
    M5.Power.setAutoBootOnLoad(AutoBootOnLoad);
    M5.Power.setCharge(Charge);
    // [deprecated] M5.Power.setKeepLightLoad(KeepLightLoad);
    // [deprecated] M5.Power.setLowPowerShutdown(LowPowerShutdown);
}

void loop(){                                    // 繰り返し実行する関数
    M5.Lcd.setCursor(0, 16, 2);
    printb(IP5306read(0x00));
    printb(IP5306read(0x01));
    printb(IP5306read(0x02));
    printb(IP5306read(0x20));
    M5.Lcd.println();
    printb(IP5306read(0x21));
    printb(IP5306read(0x22));
    printb(IP5306read(0x23));
    printb(IP5306read(0x24));
    M5.Lcd.println();
    char onoff[2][4]={"Off","On"};
    char yesno[2][4]={"no","yes"};
//  M5.Lcd.printf("canControl        = %s  \n",yesno[M5.Power.canControl()]);
    M5.Lcd.printf("isChargeFull      = %s  \n",yesno[M5.Power.isChargeFull()]);
    M5.Lcd.printf("isCharging        = %s  \n",onoff[M5.Power.isCharging()]);
    uint8_t BatteryLevel = M5.Power.getBatteryLevel();
    M5.Lcd.printf("BatteryLevel      = ");
    if(Charge){
        M5.Lcd.printf("--   \n");
        progressBar += 9;
        if(progressBar > 99){
            progressBar = 0;
            M5.Lcd.fillRect(168, 80 , 150, 16, WHITE);  // erase progressBar
        }
    }else{
        M5.Lcd.printf("%d   \n",BatteryLevel);
        if(progressBar > BatteryLevel){
            M5.Lcd.fillRect(168, 80 , 150, 16, WHITE);  // erase progressBar
        }
        progressBar = BatteryLevel;
    }
    if(progressBar > 99) progressBar = 99; 
    M5.Lcd.progressBar(169, 81, 148, 14, progressBar);
    
    /* コメント化している項目を表示したい場合は//を消して有効化してください */
//  M5.Lcd.printf("isResetbySoftware = %s  \n",yesno[M5.Power.isResetbySoftware()]);
//  M5.Lcd.printf("isResetbyWatchdog = %s  \n",yesno[M5.Power.isResetbyWatchdog()]);
//  M5.Lcd.printf("isResetbyDeepsleep= %s  \n",yesno[M5.Power.isResetbyDeepsleep()]);
    M5.Lcd.printf("isResetbyPowerSW  = %s  \n",yesno[M5.Power.isResetbyPowerSW()]);
    M5.Lcd.printf("PowerBoostOnOff   = %s  \n",onoff[PowerBoostOnOff]);
    M5.Lcd.printf("PowerBoostSet     = %s  \n",onoff[PowerBoostSet]);
    M5.Lcd.printf("PowerBoostKeepOn  = %s  \n",onoff[PowerBoostKeepOn]);
//  M5.Lcd.printf("PowerVin          = %s  \n",onoff[PowerVin]);
    M5.Lcd.printf("PowerBtnEn        = %s  \n",onoff[PowerBtnEn]);
//  M5.Lcd.printf("AutoBootOnLoad    = %s  \n",onoff[AutoBootOnLoad]);
//  M5.Lcd.printf("Charge            = %s  \n",onoff[Charge]);
    
    M5.update();                            // ボタン情報を更新
    if(M5.BtnA.wasPressed()){
        PowerVin = !PowerVin;
        M5.Power.setPowerVin(PowerVin);
    }
    if(M5.BtnB.wasPressed()){
        AutoBootOnLoad = !AutoBootOnLoad;
        M5.Power.setAutoBootOnLoad(AutoBootOnLoad);
    }
    if(M5.BtnC.wasPressed()){
        Charge = !Charge;
        M5.Power.setCharge(Charge);
    }

    char btn[2][2]={" ","v"};
    M5.Lcd.setCursor(40, 224, 2);
    M5.Lcd.print("["+ String(btn[PowerVin]) +"] PowerVin ");
    M5.Lcd.setCursor(132, 224, 2);
    M5.Lcd.print("["+ String(btn[AutoBootOnLoad]) +"] AutoBoot ");
    M5.Lcd.setCursor(226, 224, 2);
    M5.Lcd.print("["+ String(btn[Charge]) +"] Charge ");
    delay(100);
}
