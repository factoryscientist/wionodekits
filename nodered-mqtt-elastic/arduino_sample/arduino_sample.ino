#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

//ランダムにclientIDを生成するための配列
static const char alphanum[] = "0123456789"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz";

unsigned long prev; //前回実行時間の記録
int fail_count = 0; //失敗の回数をカウント
WiFiClient espClient;
PubSubClient client(espClient);
char jsonOutput[128]; //publishするjsonを格納

//====================================================
const char *ssid = "yourSSID";
const char *password = "yourPW";
const char *mqtt_server = "youerIPAddress"; // Ex. 192.168.1.168
const int port = 1883;

const char *Topic = "TwoData"; //OneData

unsigned long interval = 300000; //300000ms = 5min

const char *device = "yourdevicename";

//それぞれのセンサーの名前  小文字のみ
const char *sensor_a = "yoursensorname";
const char *sensor_b = "yoursensorname";

//MQTTbrokerで指定したユーザー名とパスワード
const char *mqtt_username = "";
const char *mqtt_password = "";

//====================================================

//セットアップ
void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();

  setup_wifi();
  client.setServer(mqtt_server, port);
  client.setCallback(callback);

  delay(1000);
  Serial.print(" Topic:");
  Serial.println(Topic);
  Serial.print(" interval:");
  Serial.println(interval);
  prev = 0;
}

//ループ
void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    setup_wifi();
    prev = millis();
  }
  if (!client.connected())
  {
    reconnect();
    prev = millis();
  }
  client.loop();

  if ((millis() - prev) >= interval)
  {
    float a = 1.0; //センサーAの値
    float b = 2.0; //センサーBの値
    publishJsonTwoData(a, sensor_a, b, sensor_b);
    /*
     *  publishJsonOneData(a,sensor_a);
     *  publishJsonOneData(b,sensor_b);
     */
    prev += interval;
  }
  delay(1);
}

//送るデータによってfloatから変更する
//JSON生成してPublishする
//
/*
{
  "device":"device",
  "sensor_One":"sensor_One",
  "data_One":sensor_data_One,
}
*/
void publishJsonOneData(float sensor_data_One, String sensor_One)
{
  char Topic = "OneData"; //
  const size_t CAPACITY = JSON_OBJECT_SIZE(4);
  StaticJsonDocument<CAPACITY> doc;
  JsonObject object = doc.to<JsonObject>();
  object["device"] = device;
  object["sensor"] = sensor_One;
  object["data"] = sensor_data_One;
  serializeJson(doc, jsonOutput);

  Serial.println(String(jsonOutput));
  client.publish(Topic, jsonOutput);
  fail_count = 0;
}

//送るデータによってfloatから変更する
//JSON生成してPublishする
/*
{
  "device":"device",
  "sensor_One":"sensor_One",
  "data_One":sensor_data_One,
  "sensor_Two":"humidity",
  "data_Two":sensor_data_Two
}
*/
void publishJsonTwoData(float sensor_data_One, String sensor_One, float sensor_data_Two, String sensor_Two)
{
  char Topic = "TwoData";
  const size_t CAPACITY = JSON_OBJECT_SIZE(7);
  StaticJsonDocument<CAPACITY> doc;
  JsonObject object = doc.to<JsonObject>();
  object["device"] = device;
  object["sensor_One"] = sensor_One;
  object["data_One"] = sensor_data_One;
  object["sensor_Two"] = sensor_Two;
  object["data_Two"] = sensor_data_Two;
  serializeJson(doc, jsonOutput);

  Serial.println(String(jsonOutput));
  client.publish(Topic, jsonOutput);
  fail_count = 0;
}

//失敗回数をカウント
void restart()
{
  if (120 <= fail_count++)
  {
    ESP.restart();
  }
}

//WiFiの接続
void setup_wifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
    restart();
  }
  Serial.println();
  Serial.println("WiFi connected");
  //IPaddressを出力
  Serial.print("IPaddress");
  Serial.println(WiFi.localIP());
}

//コールバック
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

//再接続
void reconnect()
{
  char clientID[10];

  //再接続までループ
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    // ClientID生成
    for (int i = 0; i < 8; i++)
    {
      clientID[i] = alphanum[random(51)];
    }
    Serial.print("clientID : ");
    Serial.println(clientID);

    if (client.connect(clientID, mqtt_username, mqtt_password))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
    delay(10);
    restart()
  }
}
