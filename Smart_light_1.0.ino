/*
 * Smart_Light_V1.0
 * Designed by ZhangHao
 * 2018.8.16
 * 
 * Note:
 * 1.实现MQTT连接，协议定义与解析。
 * 2.实现OTA升级功能
 * 3.实现白光灯和暖白灯的PWM调节
 * 4.RGB灯驱动程序
 * 5.光敏电阻测环境光亮度
 * 6.声音传感器驱动程序
 * 7.继电器驱动程序
 * 
 * 
 * 详见ReadMe.md文件
 */

/*
include files
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#define SW1_pin 5
#define SW2_Pin 12
#define DIN_Pin 4
#define PWM_1_Pin 16
#define PWM_2_Pin 14
#define Sound_Pin 13
#define Key_Pin 2

//WIFI信息，可由配置按键修改
const char *ssid = "TP123";
const char *password = "123456789";
const char *mqtt_server = "118.89.221.19"; //EMQ

//Global variables 全局变量
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
static char wbuf[128];
char MAC_adress[20] = {'\0'}; //不知道如何读MAC地址，待完成
int A00 = 127;                                 //白光默认一半亮度
int A11 = 127;                                 //暖白默认一半亮度
int A22 = 0;                                   //光敏电阻
int A33 = 0;                                   //继电器1默认关闭
int A44 = 0;                                   //继电器2默认关闭
int A55 = 0;                                   //RGB-R
int A66 = 0;                                   //RGB-G
int A77 = 0;                                   //RGB-B
int A88 = 0;                                   //RGB-Model
char data_aa[64];                              //透传数据
short D00 = 0;                                 //默认不主动上报数据
short V0 = 10;                                 //10秒上报间隔

//Wifi初始化，连接到路由器
void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//OTA升级  参数为要升级的文件路径
void OTA_Update(char *url)
{
  Serial.print("OTA_URL:");
  Serial.println(url);
  if(url[0]==0){
    sprintf(url,"http://39.106.187.215/Smart_light/Smart_light_V1.0.bin");
    }
  t_httpUpdate_return ret = ESPhttpUpdate.update(url);
  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    Serial.println("[update] Update failed.");
    break;
  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("[update] Update no Update.");
    break;
  case HTTP_UPDATE_OK:
    Serial.println("[update] Update ok."); // may not called we reboot the ESP
    break;
  }
}

void report_data()
{
  String data = "{";
  if (D00 && 0x01)
  {
    data += "A0=";
    data += A00;
  }
  if (D00 && 0x02)
  {
    if (data.length() > 1)
    {
      data += ",";
    }
    data += "A1=";
    data += A11;
  }
  if (D00 && 0x04)
  {
    if (data.length() > 1)
    {
      data += ",";
    }
    data += "A2=";
    data += A22;
  }
  if (D00 && 0x08)
  {
    if (data.length() > 1)
    {
      data += ",";
    }
    data += "A3=";
    data += A33;
  }
  if (D00 && 0x10)
  {
    if (data.length() > 1)
    {
      data += ",";
    }
    data += "A4=";
    data += A44;
  }
  if (D00 && 0x20)
  {
    if (data.length() > 1)
    {
      data += ",";
    }
    data += "A5=";
    data += A55;
  }
  if (D00 && 0x40)
  {
    if (data.length() > 1)
    {
      data += ",";
    }
    data += "A6=";
    data += A66;
  }
  if (D00 && 0x80)
  {
    if (data.length() > 1)
    {
      data += ",";
    }
    data += "A7=";
    data += A77;
  }
  data += "}";
  if (D00)
  {
    int ii = 0;
    char datata[64] = {'\n'};
    int len = data.length();
    for (ii = 0; ii < len; ii++)
    {
      datata[ii] = data[ii];
    }
    client.publish("SL_OUT_8266", datata); //主动推送被查询的消息
  }
}

//用户信息处理
int usr_process_command_call(char *ptag, char *pval, char *pout)
{
  int val;
  int ret = 0;
  val = atoi(pval);
  //控制命令解析
  if (0 == strcmp("CD0", ptag))
  {              //若检测到CD0指令
    D00 &= ~val; //关闭主动上报
  }
  if (0 == strcmp("OD0", ptag))
  {             //若检测到OD0指令
    D00 |= val; //开启主动上报
  }
  if (0 == strcmp("D0", ptag))
  { //若检测到D0指令
    if (0 == strcmp("?", pval))
    {                                    //若检测到询问指令
      ret = sprintf(pout, "D0=%u", D00); //向pout以"D0=%u"赋值输出D0参数
    }
  }
  if (0 == strcmp("A0", ptag))
  { //若检测到A0指令
    if (0 == strcmp("?", pval))
    {
      ret = sprintf(pout, "A0=%u", A00);
    }
    else
    {
      //在此修改白光LED亮度
      A00 = val;
    }
  }
  if (0 == strcmp("A1", ptag))
  { //若检测到A1指令
    if (0 == strcmp("?", pval))
    {
      ret = sprintf(pout, "A1=%u", A11);
    }
    else
    {
      //在此修改暖白LED亮度
      A11 = val;
    }
  }
  if (0 == strcmp("A2", ptag))
  { //若检测到A2指令
    //在此检测光敏电阻
    ret = sprintf(pout, "A2=%u", A22);
  }

  if (0 == strcmp("A3", ptag))
  {
    if (0 == strcmp("?", pval))
    {
      ret = sprintf(pout, "A3=%u", A33);
    }
    else
    {
      //如果1写继电器1为高，else low
      A33 = val;
    }
  }

  if (0 == strcmp("A4", ptag))
  {
    if (0 == strcmp("?", pval))
    {
      ret = sprintf(pout, "A4=%u", A44);
    }
    else
    {
      //如果1写继电器2为高，else low
      A44 = val;
    }
  }

  if (0 == strcmp("A5", ptag))
  {
    if (0 == strcmp("?", pval))
    {
      ret = sprintf(pout, "A5=%u", A55);
    }
    else
    {
      //RGB-R  写入刷新
      A55 = val;
    }
  }

  if (0 == strcmp("A6", ptag))
  {
    if (0 == strcmp("?", pval))
    {
      ret = sprintf(pout, "A6=%u", A66);
    }
    else
    {
      //RGB-G  写入刷新
      A66 = val;
    }
  }

  if (0 == strcmp("A7", ptag))
  {
    if (0 == strcmp("?", pval))
    {
      ret = sprintf(pout, "A7=%u", A77);
    }
    else
    {
      //RGB-B  写入刷新
      A77 = val;
    }
  }

  if (0 == strcmp("A8", ptag))
  {
    if (0 == strcmp("?", pval))
    {
      ret = sprintf(pout, "A8=%u", A88);
    }
    else
    {
      //a function with switch case -- model
      A88 = val;
    }
  }

  if (0 == strcmp("AA", ptag))
  {
    if (0 == strcmp("?", pval))
    {
      ret = sprintf(pout, "AA=%s", data_aa);
    }
    else
    {
      //透传数据
      sprintf(data_aa, pval);
    }
  }

  if (0 == strcmp("V0", ptag))
  { //若检测到V0指令
    if (0 == strcmp("?", pval))
    {                                   //若检测到询问指令
      ret = sprintf(pout, "V0=%u", V0); //向pout以"V0=%u"赋值输出V0参数
    }
    else
    {
      if (val > 0 && val < 99)
      {
        V0 = val;
      } //否则更新数据上传时间间隔
    }
  }

  return ret;
}

//处理通用消息
static int _process_command_call(char *ptag, char *pval, char *pout)
{
  int val;
  int ret = 0;

  val = atoi(pval);
  if (0 == strcmp("ECHO", ptag))
  { //ECHO测试连通性，数据原样返回
    ret = sprintf(pout, "ECHO=%s", pval);
  }
  else if (0 == strcmp("MAC", ptag))
  { //查询模块MAC地址
    ret = sprintf(pout, "MAC=%s", MAC_adress);
  }
  else if (0 == strcmp("IP", ptag))
  { //查询模块IP地址
    ret = sprintf(pout, "IP=%s", "0.0.0.0");
  }
  else if (0 == strcmp("TYPE", ptag))
  { //设备串号
    if (0 == strcmp("!", pval))
    {
      ret = sprintf(pout, "TYPE=hbber");
    }
  }
  else if (0 == strcmp("U", ptag))
  { //设备OTA升级
    OTA_Update(pval);
  }
  else
  {
    ret = usr_process_command_call(ptag, pval, pout);
  }
  return ret;
}

//MQTT消息的回调函数，用来处理收到的消息
void callback(char *topic, byte *payloadbyte, unsigned int length)
{
  char *p;
  char *ptag = NULL;
  char *pval = NULL;
  char *pwbuf = wbuf + 1;
  char payload[200] = {'\0'};
  //向串口打印收到的消息
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("--");
  Serial.print(length);
  Serial.print("] ");
  if(length>200)return;
  for (int i = 0; i < length; i++)
  {
    payload[i] = (char)payloadbyte[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();

  //处理消息
  if (payload[0] != '{' || payload[length - 1] != '}')
  {
    Serial.println("Error MQTT CMD.");
  }

  payload[length - 1] = 0;
  p = payload + 1;
  do
  {
    ptag = p;
    p = strchr(p, '=');
    if (p != NULL)
    {
      *p++ = 0;
      pval = p;
      p = strchr(p, ',');
      if (p != NULL)
      {
        *p++ = 0;
      }
      int ret;
      ret = _process_command_call(ptag, pval, pwbuf);
      if (ret > 0)
      {
        pwbuf += ret;
        *pwbuf++ = ',';
      }
    }
  } while (p != NULL);
  if ((pwbuf - wbuf) > 1)
  { //如果有返回值，则构建返回值数据
    wbuf[0] = '{';
    pwbuf[0] = 0;
    pwbuf[-1] = '}';
    client.publish("SL_OUT_8266", wbuf); //推送被查询的消息
  }
}

//MQTT自动重连函数
void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Smart_light_Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("infoTopic", "Smart_light Online now.");
      // ... and resubscribe
      client.subscribe("SL_IN_8506");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//获取当前光照值
int Get_light(){
  return analogRead(A0);
}

//继电器1控制
void SW1_control(int val){
  if(val){
    digitalWrite(SW1_pin,HIGH);
  }else{
    digitalWrite(SW1_pin,LOW);
  }
}

//继电器2控制
void SW2_control(int val){
  if(val){
    digitalWrite(SW2_Pin,HIGH);
  }else{
    digitalWrite(SW2_Pin,LOW);
  }
}

//白光亮度控制
void PWM1_control(int val){
  if(val<0||val>1023)
  analogWrite(PWM_1_Pin, 0);
  else
  analogWrite(PWM_2_Pin, val);
}

//暖白亮度控制
void PWM2_control(int val){
  if(val<0||val>1023){
    analogWrite(PWM_2_Pin, 0);
  }else{
    analogWrite(PWM_2_Pin, val);
  }
}

//按键检测
 void Key_scan(){
   if(digitalRead(Key_Pin)==0){
     delay(10);
     if(digitalRead(Key_Pin)==0){
       //转为AP模式，获取SSID和password之后重新连接WiFi
       //TODO
     }
   }
 }

void setup()
{
  //初始化IO口
  pinMode(PWM_1_Pin, OUTPUT); 
  pinMode(PWM_2_Pin, OUTPUT); 
  pinMode(SW1_pin, OUTPUT); 
  pinMode(SW2_Pin, OUTPUT); 
  pinMode(DIN_Pin, OUTPUT); 
  pinMode(Sound_Pin, OUTPUT); //之后升级再使用此功能
  pinMode(Key_Pin, INPUT_PULLUP); 

  //初始化串口  输出日志
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883); //连接服务器
  client.setCallback(callback);        //设置回调函数
}

void loop()
{
  if (!client.connected())
  { //检查是否掉线，自动重连
    reconnect();
  }

  client.loop();
}
