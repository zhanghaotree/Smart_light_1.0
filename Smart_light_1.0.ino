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
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//WIFI信息，可由配置按键修改
const char* ssid = "TP123";
const char* password = "123456789";
const char* mqtt_server = "118.89.221.19"; //EMQ

//Global variables 全局变量
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//Wifi初始化，连接到路由器
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//MQTT消息的回调函数，用来处理收到的消息
void callback(char* topic, byte* payload, unsigned int length) {
  //向串口打印收到的消息
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  //处理消息
  if((char)payload[1] == 'U'){
    //检测到OTA升级消息
    //TODO
  } else{
    //调用解析函数
  }

}

//MQTT自动重连函数
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Smart_light_Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("infoTopic", "Smart_light Online now.");
      // ... and resubscribe
      client.subscribe("SL_IN_8506");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883); //连接服务器
  client.setCallback(callback);//设置回调函数

}

void loop() {
  if (!client.connected()) {//检查是否掉线，自动重连
    reconnect();
  }

  client.loop();

}
