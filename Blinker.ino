/*功能:WiFi配网，设备控制，定时重启，获取时间，18-24点闪灯，设备状态反馈，Siri控制*/

#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT
#define BLINKER_PRINT Serial//串行链接协议库
#include <Blinker.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>//配网库文件


#define cesuo  12  //D6  
#define zoulang 14  //D5  
#define light 2  // D4
#define rst     13  //d7
#define wat1 5  //D1
#define wat2 4  //D2

#define BUTTON_1 "btn-abc"
#define BUTTON_2 "btn-def"
#define BUTTON_3 "btn-rst"
#define BUTTON_4 "btn-lig"
#define BUTTON_5 "btn-wat"
char auth[] = "";
char ssid[] = "";//自动配网留空
char pswd[] = "";//自动配网留空 
BlinkerButton Button1(BUTTON_1);
BlinkerButton Button2(BUTTON_2);
BlinkerButton Button3(BUTTON_3);
BlinkerButton Button4(BUTTON_4);
BlinkerButton Button5(BUTTON_5);
Ticker ticker1; 
Ticker ticker2;
WiFiServer server(80); //服务器端口号

uint8_t wsMode = BLINKER_CMD_MIOT_DAY;
int8_t hours;
int8_t minn;
int16_t sec=0;
int Lastmill=0;
bool flag;
void Flash_Light()
{
  int state = digitalRead(light);  // get the current state of GPIO1 pin
  digitalWrite(light, !state);     // set pin to the opposite state
}

void Flash_WAT_Light()
{
  int state = digitalRead(light);  // get the current state of GPIO1 pin
  digitalWrite(light, !state);     // set pin to the opposite state
}

void Breath_light()//呼吸灯程序
{
  for (int i = 0; i < 1024; i++)
 
  {//电平升高，从明到暗
 
    analogWrite(light, i);
 
    Blinker.delay(2);
 
  }
 
  for (int i = 1024; i >= 0; i--)
 
  {//电平降低，从暗到明
 
    analogWrite(light, i);
 
    Blinker.delay(2);
 
  }
}

void Auto_Stop()
{
   sec++;
   Flash_WAT_Light();
   Serial.printf("sec %d\n",sec);
   if(sec==360)
   {
    sec=0;
     digitalWrite(wat1, HIGH);
     digitalWrite(wat2, HIGH);
     ticker2.detach();
    flag=1;
   }

}

 
void SetButton(char state)
{
   if (state == 1)
    {
      Button1.color("#00DB00");
      Button1.text("开");
      Button1.print("on");
    }
    else if(state == 2)
    {
      Button1.color("#FF0000");
      Button1.text("关");
      Button1.print("off");
    }
    else if(state == 3)
    {
       Button2.color("#00DB00");
       Button2.text("开");
       Button2.print("on");

    }
    else if(state == 4)
    {
      Button2.color("#FF0000");
      Button2.text("关");
      Button2.print("off");
    }
    else if(state == 5)
    {
       Button5.color("#00DB00");
       Button5.text("开");
       Button5.print("on");
    }
    else
    {
      Button5.color("#FF0000");
      Button5.text("关");
      Button5.print("off");
    }
}

 
  void heartbeat()
{
 
   BLINKER_LOG("状态同步!");
   if (digitalRead(cesuo)==HIGH)
        SetButton(1);
   else 
        SetButton(0);
   if (digitalRead(zoulang )== HIGH)
        SetButton(3);
   else 
        SetButton(4);
    if(digitalRead(wat1) == LOW )
    {
         SetButton(5);
    }
    else
    {
         SetButton(6);
    }
}


void button1_callback(const String & state)
{
    BLINKER_LOG("get button state: ", state);
    if(state=="on")
    {
      digitalWrite(cesuo, HIGH);
      SetButton(1);
    }
    else if(state=="off")
    {
      digitalWrite(cesuo, LOW);
      SetButton(2);
    }
    else 
     Button1.print("ok");
}

void button2_callback(const String & state)
{
    BLINKER_LOG("get button state: ", state);
    if(state=="on")
    {
      digitalWrite(zoulang, HIGH);
      SetButton(3);
    }
    else if(state=="off")
    {
      digitalWrite(zoulang, LOW);
      SetButton(4);
    }
    else 
     Button2.print("ok");
   
}
void button3_callback(const String & state)
{
    BLINKER_LOG("get button state: ", state);
    if(state=="rst")
    {
      digitalWrite(rst, LOW);
    }
    else 
     digitalWrite(rst, HIGH);
   
}

void button4_callback(const String & state)
{
   BLINKER_LOG("get button state: ", state);
   if(state=="on")
    {
       ticker1.attach(1.0, Flash_Light);   //每0.5秒调用callback1
    }
    else if(state=="off")
    {
       ticker1.detach();
       
    }
    else 
     Button4.print("ok");
    
}
void button5_callback(const String & state)
{
    BLINKER_LOG("get button state: ", state);
    if(state=="on")
    {
      digitalWrite(wat1, LOW);
      digitalWrite(wat2, LOW);
       ticker1.detach();
      ticker2.attach(1.0, Auto_Stop);   //每0.1秒调用callback1
      SetButton(5);
    }
    else if(state=="off")
    {
      digitalWrite(wat1, HIGH);
      digitalWrite(wat2, HIGH);
      flag=1;
      ticker2.detach();
      sec=0;
      SetButton(6);
    }
    else 
     Button2.print("ok");
   
}
void Watch_Dog(void)
{
   ESP.wdtFeed();
}

void miotMode(uint8_t mode)
{
    BLINKER_LOG("need set mode: ", mode);

    if (mode == BLINKER_CMD_MIOT_DAY)//日光
    {
        digitalWrite(cesuo, HIGH);  
        SetButton(1);

    } 
    else if (mode == BLINKER_CMD_MIOT_NIGHT)//月光
    {
        digitalWrite(cesuo, LOW);  
        SetButton(2);
    }
    else if (mode == BLINKER_CMD_MIOT_COLOR) //彩光
    {
         digitalWrite(zoulang, HIGH); 
        SetButton(3);
    }
    else if (mode == BLINKER_CMD_MIOT_WARMTH)//w温馨 
    {
        digitalWrite(zoulang, LOW); 
        SetButton(4);
    }
    else if (mode == BLINKER_CMD_MIOT_TV)//电视模式 
    {
      digitalWrite(wat1, LOW);
      digitalWrite(wat2, LOW);
      ticker1.detach();
      ticker2.attach(1.0, Auto_Stop);   //每0.1秒调用callback1
       SetButton(5);
    }
    else if (mode == BLINKER_CMD_MIOT_READING)//阅读模式
    {
      digitalWrite(wat1, HIGH);
      digitalWrite(wat2, HIGH);
      flag=1;
       SetButton(6);
    }
    else if (mode == BLINKER_CMD_MIOT_COMPUTER) //电脑模式
    {
        // Your mode function
    }

    wsMode = mode;

    BlinkerMIOT.mode(mode);
    BlinkerMIOT.print();
}




void setup()
{
    // 初始化串口
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);//自动配网开始
    int cnt = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        Blinker.delay(500);
        Serial.print(".");
        if (cnt++ >= 10) 
        {
          WiFi.beginSmartConfig();
          while (1)
          {
              Blinker.delay(1000);
              if (WiFi.smartConfigDone())
              {
              Serial.println();
              Serial.println("SmartConfig: Success");
                break;
              }
          }
        }
    }//配网结束
    BLINKER_DEBUG.stream(Serial);
    server.begin();
    pinMode(cesuo, OUTPUT);
    digitalWrite(cesuo, LOW);
    
    pinMode(zoulang, OUTPUT);
    digitalWrite(zoulang, LOW);


    pinMode(light, OUTPUT);
     digitalWrite(light, HIGH);

     
    pinMode(rst, OUTPUT);
    digitalWrite(rst, HIGH);

     pinMode(wat1, OUTPUT);
     digitalWrite(wat1, HIGH);
     
   pinMode(wat2, OUTPUT);
   digitalWrite(wat2, HIGH);
    flag=1;
// 初始化blinker
   //初始化定时器 
    Blinker.begin(auth, ssid, pswd);
    delay(100);
    Blinker.setTimezone(8);  
    BlinkerMIOT.attachMode(miotMode);
    Button1.attach(button1_callback);
    Button2.attach(button2_callback);
    Button3.attach(button3_callback);
    Button4.attach(button4_callback);
    Button5.attach(button5_callback);
    Blinker.attachHeartbeat(heartbeat);
}
 
void loop() 
{
    Blinker.run();
    if(millis()-Lastmill>=30000)//30s获取一次时间
    {
      Lastmill=millis();
      hours=Blinker.hour();
      delay(100);
      minn=Blinker.minute();
      delay(100);
      BLINKER_LOG("hour ", hours);
      BLINKER_LOG("min ", minn);
    }
    if(hours!=-1)
    {
      //Serial.print("step0");
        if(hours==18||hours==0||hours==12)
        {
          if(minn==3)
          {
          digitalWrite(rst, LOW);
           Serial.print("step1\n");
          }
        }
        if(flag==1)
        {
          if((hours>=18)&&(hours<=23))
          {
            ticker1.attach(1.0, Flash_Light);
             flag=0;
              
          }
        }
        
    }
    WiFiClient client = server.available();
  if (!client) 
  {
    delay(100);
    return;
  }
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();        
  if (req.indexOf("/gpio1/on") != -1)
  {
    digitalWrite(zoulang, HIGH);
  }
  else if (req.indexOf("/gpio1/off") != -1)
  {
    digitalWrite(zoulang, LOW);
  }
  else if (req.indexOf("/gpio2/on") != -1)
  {
    digitalWrite(cesuo, HIGH);
  }
  else if (req.indexOf("/gpio2/off") != -1)
  {
    digitalWrite(cesuo, LOW);
  }
   else if (req.indexOf("/wat/on") != -1)
  {
    digitalWrite(wat1, LOW);
    digitalWrite(wat2, LOW);
    ticker1.detach();
    ticker2.attach(1.0, Auto_Stop);   //每0.1秒调用callback1
    flag=1;
  }
   else if (req.indexOf("/wat/off") != -1)
  {
    digitalWrite(wat1,HIGH);
    digitalWrite(wat2,HIGH);
    flag=1;
  }
  else
  {
    Serial.println("invalid request");
    client.print("HTTP/1.1 404\r\n");
    client.stop();
    return;
  }
  client.flush();
  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nWelcome wenzheng space! ";
  s += "</html>\n";

  client.print(s);
  delay(1);
  Serial.println("Client disonnected");
}
