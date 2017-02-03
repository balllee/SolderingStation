/* by Balllee 20170118 */
#include <PID_v1.h>
#include <max6675.h>
#include <LedControl.h>

byte pin_Pot = A0;
byte pin_Heater = 3;  //PWM  led
int analogReadA3Buffer = 0;
byte zeroCrossing = 0;
unsigned long time[2] = {0,0};       //運行時間
volatile bool powerOnoff = 0;
 
int timeDelay[256] = {0,665,941,1153,1332,1491,1634,1766,1889,2005,2115,2220,2320,2417,2509,2599,2686,2771,2853,2933,3012,3088,3163,3237,3308,3379,3448,3517,3584,3650,3715,3779,3842,3904,3966,4027,4087,4146,4205,4263,4321,4378,4434,4490,4545,4600,4654,4708,4762,4815,4867,4919,4971,5023,5074,5125,5175,5225,5275,5324,5374,5422,5471,5519,5568,5615,5663,5710,5758,5805,5851,5898,5944,5990,6036,6082,6127,6173,6218,6263,6308,6353,6397,6442,6486,6530,6575,6618,6662,6706,6750,6793,6836,6880,6923,6966,7009,7052,7095,7137,7180,7223,7265,7308,7350,7392,7434,7477,7519,7561,7603,7645,7687,7729,7771,7812,7854,7896,7938,7979,8021,8063,8104,8146,8188,8229,8271,8313,8354,8396,8437,8479,8521,8562,8604,8646,8687,8729,8771,8812,8854,8896,8938,8980,9022,9064,9106,9148,9190,9232,9274,9317,9359,9402,9444,9487,9529,9572,9615,9658,9701,9744,9787,9830,9874,9917,9961,10004,10048,10092,10136,10180,10225,10269,10314,10359,10404,10449,10494,10539,10585,10631,10676,10723,10769,10815,10862,10909,10956,11004,11051,11099,11147,11196,11244,11293,11342,11392,11442,11492,11542,11593,11644,11695,11747,11799,11852,11905,11958,12012,12067,12122,12177,12233,12289,12346,12403,12462,12520,12580,12640,12701,12762,12825,12888,12952,13017,13083,13150,13218,13288,13358,13430,13503,13578,13655,13733,13813,13896,13980,14067,14157,14250,14347,14447,14552,14661,14777,14901,15033,15176,15334,15514,15726,16002,16667};

LedControl lc=LedControl(6,5,4,1); //SI,SCK,CS
MAX6675 thermocouple(9, 8, 7);//SCK,CS,SO

//PID
double Setpoint, Input, Output;
double consKp=0.7, consKi=0.01, consKd=3;
PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT);

void setup() {
//  Serial.begin(9600);
  lc.shutdown(0,false);
  lc.setIntensity(0,8);
  lc.clearDisplay(0);
  segmentSetpoint(10);
  pinMode(pin_Heater, OUTPUT);
  myPID.SetMode(AUTOMATIC);
  initTimer();  //初始化Timer1
}

void loop() {
  time[0] = millis();
  if(time[1] != time[0]){
    time[1] = time[0];

    if(time[0] % 2 == 0){  //每2ms執行一次
      static unsigned int val_Pot = 0;
      static byte num = 0;
      num += 1;
      val_Pot += analogRead(A0);  //Pot
      if(num >= 40){
        val_Pot /= num;  //Pot值 算數平均
        Setpoint = map(val_Pot,0,1015,0,50);  //調整輸入溫度範圍 (0~100)*5=(0~500) 
        Setpoint *= 10;   //以5為單位
        segmentSetpoint(Setpoint);  //顯示目標溫度
        num = 0;
        val_Pot = 0;
      }
      
      if(time[0] % 250 == 0){  //Temp
        Input = thermocouple.readCelsius();  //熱電耦溫度
        
        if(isnan(Input)){   //檢查熱電耦是否接上
          TIMSK1 &= ~_BV(OCIE1A);  //Timer1A關閉
          TIMSK1 &= ~_BV(OCIE1B);  //Timer1B關閉
          for(int i = 0;i < 8;i++){
            lc.setDigit(0,i,0,false);
          }
          while(1){}
//          do{
//            delay(500);
//            Input = thermocouple.readCelsius();
//          }while(isnan(Input));
        }
        
        myPID.Compute();   //PID運算
        int out = Output;  //double轉換成int
        if(Input > Setpoint-4){out = 0;}  //input溫度大於設定溫度時不輸出
        if(Input < Setpoint - 20){out = 255;}  //input溫度小於設定溫度時，全力輸出(瞬熱)
        if(out < 2){      //輸出忽略
          digitalWrite(3,0);
          OCR1B = 16670;   //相當於關閉Timer1B
          powerOnoff = 0;
        }
        else if(out == 255){
          digitalWrite(3,1);
          OCR1B = 16670;  //相當於關閉Timer1B
          powerOnoff = 1;
        }
        else{
          OCR1B = timeDelay[out];
          powerOnoff = 1;
        }
        segmentTemp(Input);  //顯示熱電耦溫度
//        Serial.println(out);
//        Serial.println(Output);

        if(time[0] >= 3600000){  //自動關機
          TIMSK1 &= ~_BV(OCIE1A);  //Timer1A關閉
          TIMSK1 &= ~_BV(OCIE1B);  //Timer1B關閉
          digitalWrite(3,0);
          lc.setDigit(0,2,0,false);    //右邊顯示"OFF"
          lc.setDigit(0,1,15,false);
          lc.setDigit(0,0,15,false);
          while(1){
            segmentTemp(thermocouple.readCelsius());  //顯示熱電耦溫度
            delay(250);
          }
        }
      }
    }
  }

  analogReadA3Buffer = analogRead(A3);  //Zero Crossing點偵測
  if(analogReadA3Buffer != 0){zeroCrossing = 1;}  //避免0V時重複計算
  if(analogReadA3Buffer == 0 && zeroCrossing != 0){
    TCNT1 = 0;  //計數器Timer1歸零
    if(powerOnoff == 0){digitalWrite(3,0);}
    else{digitalWrite(3,1);}
    zeroCrossing = 0;
  }
}
