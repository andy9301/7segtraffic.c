#include <EEPROM.h>
#include "TimerOne.h"
#include "Adafruit_NeoPixel.h"
#include  <SoftwareSerial.h>

//LED * 8 - 74HC595
#define PIN_595_SDI    A1
#define PIN_595_LATCH  A2
#define PIN_595_CLK    A3
//7SEG
const int seg7[] = {2, 3, 4, 5, 6, 7, 8}; //宣告顯示信號所連接的接腳
//const int DD = A0;      //宣告閃秒信號所連接的接腳
const int scanP[] = {10, 11, 12, 13}; //宣告掃瞄信號所連接的接腳
byte disp[] = {8, 0, 0, 0}; //宣告顯示緩衝區
const byte mySeg7[] =     //宣告編碼陣列
{ 0x3F, 0x06, 0x5B, 0x4F,  0x66, 0x6D, 0x7D, 0x07, //0~7
  0x7F, 0x6F, 0xA0, 0x83,  0xA7, 0xA1, 0x86, 0x8E, 0x38, 0x76
}; //8~17

#define PIN_DATA_LC    8 //非實體port
//WS2812
#define LED_PIN        8 //非實體port
#define LED_COUNT 8      //非實體port
//SW
#define PIN_SW1        A4
#define PIN_SW2        A5
#define PIN_SW3        A0

int eeprom_addr = 10; //eepRom位址,非實體port

//Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned char led_index, pw1,pw2,flag_led_mode1, flag_led_mode2, led_bri, led_rgb_mode, flag_led_mode4, flag_led_mode3;
unsigned char SW1, SW1_last, SW2, SW2_last, SW3, SW3_last;
int timer_200ms, timer_20ms, timer1, timer2, k;
unsigned char uart_receive_buf[12], uart_send_buf[9];
unsigned char SerialAnalyzeActive, SerialReceiveCnt;
unsigned char flag_link;
unsigned char t_hh, t_mm, t_ss, t_index, led_ct, t_led, t_led2;
int adcVal = 0; //讀取ADC變數,初始值為0
float adc_f;
unsigned long t0=0,t1=0,t2=0;    //宣告計時變數
int hs=0,second=0;      //宣告時間變數
void setup()
{

     pinMode(PIN_595_CLK, OUTPUT);
     pinMode(PIN_595_SDI, OUTPUT);
     pinMode(PIN_595_LATCH, OUTPUT);
  for (int i = 0; i < 7; i++) //設定seg7[0]~seg7[6]為輸出埠
    pinMode(seg7[i], OUTPUT);
  for (int i = 0; i < 4; i++) //設定scanP[0]~scanP[3]為輸出埠
    pinMode(scanP[i], OUTPUT);
 // pinMode(DD, OUTPUT);        //設定pP為輸出埠
  for (int i = 0; i < 4; i++) //關閉掃瞄線(不顯示)
    digitalWrite(scanP[i], 1);

//按鈕
  pinMode(PIN_SW1, INPUT_PULLUP);
  pinMode(PIN_SW2, INPUT_PULLUP);
  pinMode(PIN_SW3, INPUT_PULLUP);

  digitalWrite(PIN_595_LATCH, LOW);
  digitalWrite(PIN_DATA_LC, LOW);

//  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
//  strip.show();            // Turn OFF all pixels ASAP
//  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
//  SoftwareSerial BTSerial(2, 3);
   Serial.begin(9600);                   // 啟動串列通信
   delay(2000);                          //設定藍芽名稱,只需執行一次 
   Serial.println("AT+NAME=andy-55");     
   delay(1000);
   Serial.println("AT+RESET");     
   
  light(0);
  // byte disp[] = {0x38, 0x76, 8, 8}; //宣告顯示緩衝區                //clear

  Timer1.attachInterrupt(myHandler);
  Timer1.initialize(10000);          // Calls every 10ms


  //變數初始化
  SerialAnalyzeActive = 0;
  SerialReceiveCnt    = 0;
  timer_200ms = 20;     //20* 10ms = 200ms
  timer_20ms  = 2;
  timer1      = 0;
  flag_led_mode1 = 0;   //start = 0x01
  flag_led_mode2 = 0;   //start = 0x80
  flag_led_mode3=0x41;
  flag_led_mode4 =0;
  flag_link      = 0;   // = 0 離線狀態, = 1 連線狀態

 pw1 = (EEPROM.read(eeprom_addr)) ;
 pw2 = (EEPROM.read(eeprom_addr+1));

  t_hh = 0; t_mm = 0; t_ss = 0; t_led=0; //預設7段
  t_index = 0;
  led_rgb_mode = 0;
//     disp[3]=pw1%10;
//     disp[2]=pw1/10;
//     disp[1]=pw2%10;
//     disp[0]=pw2/10;
}


void loop()
{
 

     scanSeg7();              //掃瞄七節顯示器
  if (timer_200ms == 0) //連線時間                 //單色LED & 彩色LED & 七段顯示器 控制
  {
    timer_200ms = 50;//系統時間

    if (flag_link == 0)                 //離線模式
    {
      if (flag_led_mode1 > 0)
      {
        light(flag_led_mode1);
        if (flag_led_mode1 != 0xff)
        {
//          flag_led_mode1 = flag_led_mode1 << 1;
//
//
//          flag_led_mode1 = flag_led_mode1 | 0x01;
//     disp[3]=0;
//     disp[2]=0;
//     disp[1]=1;
//     disp[0]=2;
//               t1=millis();        //查詢時間
//          t2=t1-t0;       //計算時間差
//          scanSeg7();             //掃瞄七節顯示器
//          if(t2>=500)       //若計算時間差0.5秒
//          { 
//           t0=t1;        //儲存目前時間
//           hs=!hs;       //切換閃秒信號
//   
//           if(hs)        //計時
//   
//            if(disp[0]!=0)      
//            {   
//              disp[0]-- ; 
//              }  
//              else if(disp[0]==0)      
//              { 
//               disp[0]=9;
////               disp[1]--;
//               if (disp[1]!=0) 
//                { 
//                  disp[1]--;
//                }
//            else if(disp[1]==0)      
//            { 
//           disp[1]=0;
//           disp[0]=0;
//           
//          }
//          
//                
//        }
//      
//  }
  t1=millis();        //查詢時間
  t2=t1-t0;       //計算時間差
  scanSeg7();             //掃瞄七節顯示器
  if(t2>=500)       //若計算時間差0.5秒
  { t0=t1;        //儲存目前時間
   hs=!hs;       //切換閃秒信號
   
    if(hs)        //計時
   
     if(disp[0]!=0){   
        disp[0]-- ; 
        }  
      else if(disp[0]==0){ 
           disp[0]=9;
          // disp[1]--;
           if (disp[1]!=0){
            disp[1]--;
            }
            else if(disp[1]==0){ 
           disp[1]=0;
           disp[0]=0;
           
           }
      }
} 
      
 if(disp[1]!=0 | disp[0]!=0){//直排紅燈
  flag_led_mode1=0x41;
  if(disp[1]==0 && disp[0]==4){
    flag_led_mode1=0x42;
     if(disp[1]==0 && disp[0]==2){
      flag_led_mode1=0x44;
      //設定橫排紅燈秒數disp[3][2]
     }
   }
 }


//  else{ 開啟橫排
//  t1=millis();        //查詢時間
//  t2=t1-t0;       //計算時間差
//  scanSeg7();             //掃瞄七節顯示器
//  if(t2>=500)       //若計算時間差0.5秒
//  { t0=t1;        //儲存目前時間
//   hs=!hs;       //切換閃秒信號
//   
//    if(hs)        //計時
//   
//     if(disp[2]!=0){   
//        disp[2]-- ; 
//        }  
//      else if(disp[2]==0){ 
//           disp[2]=9;
//          // disp[3]--;
//           if (disp[1]!=0){
//            disp[3]--;
//            }
//            else if(disp[3]==0){ 
//           disp[3]=0;
//           disp[2]=0;
//           
//           }
//      }
//}  
//  flag_led_mode1=0x41;        //橫排紅燈
//  if(disp[3]==0 && disp[2]==4){
//    flag_led_mode1=0x42;
//     if(disp[3]==0 && disp[2]==2){
//      flag_led_mode1=0x44;
//      //設定直排紅燈秒數disp[1][0]
//      }
//    }
//   }

        }
        else
        {
          flag_led_mode1 = 0;

        }
      }
      else if (flag_led_mode2 > 0)
      {
//        t1=millis();        //查詢時間
//        t2=t1-t0;       //計算時間差
//        scanSeg7();             //掃瞄七節顯示器
//        if(t2>=500)       //若計算時間差0.5秒
//        { 
//          t0=t1;        //儲存目前時間
//          hs=!hs;       //切換閃秒信號
//   
//          if(hs) 
//     if(hs<5){


        light(0x41);
        delay(4000);
//          }
//        flag_led_mode3=0;
//    else if(hs<7){
        light(0x42);
        delay(1500);
        light(0x44);
        delay(1500);
//    }
//    else if(hs<12){
        light(0x14);
        delay(4000);
        light(0x44);
        delay(1500);
//    }
//    else if(hs<15){
        light(0x4c);
        delay(4000);
        light(0x44);
        delay(1500);
//    } 
//    else if(hs<18){
        light(0xc4);
        delay(4000);
        disp[3]=16;
     disp[2]=17;
     disp[1]=8;
     disp[0]=8;
//    }
     flag_led_mode2=0;
      }
    
//        light(flag_led_mode2);
//        if (flag_led_mode2 != 0xff)
//        {
//
//          flag_led_mode2 = flag_led_mode2 >> 1;
//          flag_led_mode2 = flag_led_mode2 | 0x80;
//        }
//        else
//        {
//          flag_led_mode2 = 0;
//        }
//      }
//        else if (flag_led_mode3 > 0)
//      {
//        light(flag_led_mode3);
//        delay(4000);
//        flag_led_mode3=0;
//        light(0x22);
//        delay(2000);
//        light(0x14);
//        delay(10000);
//        flag_led_mode3=0;
//                
//      }    
      else
        light(flag_led_mode1);     
    }


    
    else                                //連線模式
    {
      light(led_ct);

      if (t_index == 0)                 //顯示秒
      {
        disp[0]= t_ss % 10;
        disp[1]=t_ss / 10;
        disp[2]=t_mm % 10;
        disp[3]=t_mm / 10; //千
        
        
      }
      else if (t_index == 1)            //顯示分
      {
        disp[0]=t_mm % 10;
        disp[1]=t_mm / 10;
        
        
      }
      else if (t_index == 2)            //顯示時
      {
      
        disp[0]=t_hh % 10;
        disp[1]=t_hh / 10;
       
      }
      else if (t_index == 3)
      {
        if(t_led2>0){
          disp[0]= t_led2 % 10;
       disp[1]=t_led2 / 10;
        }
        else{
       disp[0]= t_led % 10;
       disp[1]=t_led / 10; 
        }
      }
    
    }
  }

  if (timer_20ms == 0)                   //SW
  {
    timer_20ms = 2;

    SW1 = digitalRead(PIN_SW1);
    if ((SW1_last == 1) && (SW1 == 0)) //負緣
    {
      if (flag_link == 0)
      {
        flag_led_mode1 = 0x01;
        t1=millis();        //查詢時間
        t2=t1-t0;       //計算時間差
        scanSeg7();             //掃瞄七節顯示器
  if(t2>=500)       //若計算時間差0.5秒
  { 
   t0=t1;        //儲存目前時間
   hs=!hs;       //切換閃秒信號
   
   if(hs)        //計時
   
     if(disp[3]!=0)      
      {   disp[3]-- ; }  
      else if(disp[3]==0)      
        { 
           disp[3]=9;
          // disp[2]--;
           if (disp[2]!=0) 
           { disp[2]--;}
            else if(disp[2]==0)      
        { 
           disp[2]=0;
           disp[3]=0;
           
          }
          
                
        }
      
  }
      
     scanSeg7();              //掃瞄七節顯示器

        
      }
      else
      {
        if (t_index == 0)
          t_index = 1;
        else if (t_index == 1)
          t_index = 2;
        else if (t_index == 2)
          t_index = 3;
        else if (t_index == 3)
          t_index = 0;

      }

     // led_rgb_mode = 1;
    }


    SW2 = digitalRead(PIN_SW2);
    if ((SW2_last == 1) && (SW2 == 0)) //負緣
    {
      if (flag_link == 0)
      {
        flag_led_mode2 = 0x80;
      }

     // led_rgb_mode = 2;
    }

//    SW3 = digitalRead(PIN_SW3);
//    if ((SW3_last == 1) && (SW3 == 0)) //負緣
//    {
//      if (flag_link==0)
//      {
//        
//      }
//    }

    SW1_last = SW1;
    SW2_last = SW2;
//    SW3_last = SW3;
  }

  if (Serial.available())                //UART
  {
    char inChar = (char)Serial.read();       // get the new byte:

//**********************接收/發送*************************

    if (SerialAnalyzeActive == 1)                   //開始分析接收的檔案
    {
      uart_receive_buf[SerialReceiveCnt] = inChar;
      SerialReceiveCnt++;

      if (SerialReceiveCnt >= 8)
      {
        if (uart_receive_buf[0] == 1)                                       //TIME & LED SET
        {
          t_hh = uart_receive_buf[1];
          t_mm = uart_receive_buf[2];
          t_ss = uart_receive_buf[3];
          led_ct = uart_receive_buf[4];
          t_led = uart_receive_buf[5];
          t_led2 = uart_receive_buf[6];

          uart_send_buf[0] = 0x3a;                                         //回傳資料給電腦
          uart_send_buf[1] = digitalRead(PIN_SW1) ^ 0x01;                  //按鈕狀態
          uart_send_buf[2] = digitalRead(PIN_SW2) ^ 0x01;
          uart_send_buf[3] = digitalRead(PIN_SW3) ^ 0x01;

                                     //回傳VR電壓
          Serial.write(uart_send_buf, 9);

          timer2    = 200;         //2秒內沒收到 則判斷為斷線
        }
        else if (uart_receive_buf[0] == 2)                                  //STATION NUMBER SET
        {
               EEPROM.write(eeprom_addr,uart_receive_buf[1]);  
               EEPROM.write(eeprom_addr+1,uart_receive_buf[2]);  
               EEPROM.write(eeprom_addr+2,uart_receive_buf[3]);  
               EEPROM.write(eeprom_addr+3,uart_receive_buf[4]);  
                  
         
          timer2    = 200;         //2秒內沒收到 則判斷為斷線
        }
//        else if (uart_receive_buf[0] == 3)                                  //People LED count
//        {
//               t_led = uart_receive_buf[1];  
//                  
//         
//          timer2    = 200;         //2秒內沒收到 則判斷為斷線
//        }

        SerialAnalyzeEnd();
      }
    }
    else
    {
      if (inChar = 0x3A)	                    //等待0x30 起始封包
      {
        SerialAnalyzeActive = 1;
        SerialReceiveCnt    = 0;
        timer1      = 10;                    //100ms  錯誤資料等待時間

      }
    }

  }
  else if ((SerialAnalyzeActive == 1) && (timer1 == 0))
    SerialAnalyzeEnd();

  if (timer2 > 0)                        //連線判斷
    flag_link = 1;
  else
    flag_link = 0;
}

void SerialAnalyzeEnd(void)
{
  SerialAnalyzeActive = 0;
  SerialReceiveCnt    = 0;
  timer1 = 0;
}

/*------------------ TIMER 中斷處理-----------------------*/
void myHandler()         //TIMER 10ms
{
  if (timer_200ms > 0)
    timer_200ms--;

  if (timer_20ms > 0)
    timer_20ms--;

  if (timer1 > 0)
    timer1--;

  if (timer2 > 0)
    timer2--;
}



void light(int led)
{
  shiftOut(PIN_595_SDI, PIN_595_CLK, MSBFIRST, led);        // �ϥ� shiftOut �禡�e�X���
  digitalWrite(PIN_595_LATCH, HIGH);                         // �e����ƫ�n�� latchPin �Ԧ^�����q��
  digitalWrite(PIN_595_LATCH, LOW);                          // �e��ƫe�n���� latchPin �Ԧ��C�q��
}

void writeSeg7(byte x)
{ for (int i = 0; i < 7; i++) //輸出7個位元
  { if ((x & (1 << i)) == 0) digitalWrite(seg7[i], 0);
    else digitalWrite(seg7[i], 1);
  }
}
//=== 掃瞄七節顯示器函數============
void scanSeg7(void)
{ for (int i = 0; i < 4; i++) //掃瞄4位數
  { for (int j = 0; j < 4; j++) //關閉掃瞄線(防殘影)
      digitalWrite(scanP[j], 1);
    writeSeg7(mySeg7[disp[i]]); //輸出顯示信號
    digitalWrite(scanP[i], 0); //輸出掃瞄線
    delay(1);       //持續顯示1ms
  }
}
