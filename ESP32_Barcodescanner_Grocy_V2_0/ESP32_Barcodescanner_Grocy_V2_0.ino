//Librarys
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DFRobot_GM60.h>
DFRobot_GM60_UART gm60;
#include <ArduinoMqttClient.h>
#include <SoftwareSerial.h>

//difine the Payload size for MQTT messages
#define TX_PAYLOAD_BUFFER_SIZE 2048

//Setup

//Wifi SSID
const char* ssid = "WIFI-SSID";

//Wifi Password
const char* password = "WIFI-Password";

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.???.???:9192/";


//Grocy API Key
String API_key = "Grocy API Key";

//Set your Mode
//(by enabling MQTT it only will be used as default mode after restart)
// 1 = scan and consume
// 2 = scan and add to stock
int mode = 1;

//MQTT Setup (optimized for Homeassistant)
bool MQTT_enable = true;                   //if you want to use MQTT set true if not false
const char* mqttServer = "192.168.???.???";  //The IP of your MQTT broker
const int mqttPort = 1883;                 //MQTT Port
const char* mqttUser = "MQTT-User";        //MQTT User
const char* mqttPassword = "MQTT-Password";  //MQTT Password



unsigned long check_connection;
String paltform = "homeassistant";

//Scanned Barcode
String Barcode = "0";

WiFiClient wifiClient;

MqttClient mqttClient(wifiClient);

HTTPClient http;

unsigned long lastTime = 0;
unsigned long mqtt_ref_time = 0;
unsigned long timerDelay = 5000;
String mqtt_mode = "";
bool ref_state = 0;

class grocy {
public:
  String error_message = "";

  struct barcode_data {
    String id = "";
    String product_id = "";
    String barcode = "";
    String qu_id = "";
    String shopping_location_id = "";
    float amount = 0.0;
  };

  barcode_data Barcode;

  struct product_data {
    String id = "";
    String name = "";
    String description = "";
    String location_id = "";
    String qu_id_purchase = "";
    String qu_id_stock = "";
    String min_stock_amount = "";
    String default_best_before_days = "";
    String row_created_timestamp = "";
    String group_id = "";
    String picture_file_name = "";
    String default_best_before_days_after_open = "";
    String enable_tare_weight_handling = "";
    String tare_weight = "";
    String not_check_stock_fulfillment_for_recipes = "";
    String last_shopping_location_id = "";
    String stock_amount = "";
  };

  product_data Product;


  int barcode_load_data(String BC) {
    //############################
    //Read Barcode
    String serverPath = serverName + "api/stock/products/by-barcode/" + BC;
    http.begin(serverPath.c_str());
    http.addHeader("Content-Type", "application/json");
    http.addHeader("GROCY-API-KEY", API_key);
    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {

      String payload = http.getString();

      DynamicJsonDocument doc(6144);
      deserializeJson(doc, payload);


      bool found = 0;
      bool not_found = 0;
      int counter = 0;

      //store the Product data
      String id = doc["product"]["id"];
      String name = doc["product"]["name"];
      String description = doc["product"]["description"];
      String location_id = doc["product"]["location_id"];
      String qu_id_purchase = doc["product"]["qu_id_purchase"];
      String qu_id_stock = doc["product"]["qu_id_stock"];
      String min_stock_amount = doc["product"]["min_stock_amount"];
      String default_best_before_days = doc["product"]["default_best_before_days"];
      String row_created_timestamp = doc["product"]["row_created_timestamp"];
      String group_id = doc["product"]["group_id"];
      String picture_file_name = doc["product"]["picture_file_name"];
      String default_best_before_days_after_open = doc["product"]["default_best_before_days_after_open"];
      String enable_tare_weight_handling = doc["product"]["enable_tare_weight_handling"];
      String tare_weight = doc["product"]["tare_weight"];
      String not_check_stock_fulfillment_for_recipes = doc["product"]["not_check_stock_fulfillment_for_recipes"];
      String last_shopping_location_id = doc["product"]["last_shopping_location_id"];
      String stock_amount = doc["stock_amount"];


      Product.id = id;
      Product.name = name;
      Product.description = description;
      Product.location_id = location_id;
      Product.qu_id_purchase = qu_id_purchase;
      Product.qu_id_stock = qu_id_stock;
      Product.min_stock_amount = min_stock_amount;
      Product.default_best_before_days = default_best_before_days;
      Product.row_created_timestamp = row_created_timestamp;
      Product.group_id = group_id;
      Product.picture_file_name = picture_file_name;
      Product.default_best_before_days_after_open = default_best_before_days_after_open;
      Product.enable_tare_weight_handling = enable_tare_weight_handling;
      Product.tare_weight = tare_weight;
      Product.not_check_stock_fulfillment_for_recipes = not_check_stock_fulfillment_for_recipes;
      Product.last_shopping_location_id = last_shopping_location_id;
      Product.stock_amount = stock_amount;

      //Search and Store the barcode data;
      while (!found && !not_found) {

        if (doc["product_barcodes"][counter]["barcode"] == BC) {
          found = 1;

          String id = doc["product_barcodes"][counter]["id"];
          String product_id = doc["product_barcodes"][counter]["product_id"];
          String barcode = doc["product_barcodes"][counter]["barcode"];
          String qu_id = doc["product_barcodes"][counter]["qu_id"];
          String shopping_location_id = doc["product_barcodes"][counter]["shopping_location_id"];
          String amount = doc["product_barcodes"][counter]["amount"];

          Barcode.id = id;
          Barcode.product_id = product_id;
          Barcode.barcode = barcode;
          Barcode.qu_id = qu_id;
          Barcode.shopping_location_id = shopping_location_id;
          Barcode.amount = amount.toFloat();
        }
        if (counter > 10) {
          not_found = 1;
        }
        // Serial.println(counter);
        counter++;
      }

      Serial.print("Product ");
      Serial.print(Product.name);
      Serial.println(" data loadet");
      error_message = "";
      //Serial.println(payload);
      delay(100);
    } else {
      beep_error();
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);

      DynamicJsonDocument err(6144);
      deserializeJson(err, payload);
      String error = err["error_message"];
      error_message = error;

      Product.id = "";
      Product.name = "";
      Product.description = "";
      Product.location_id = "";
      Product.qu_id_purchase = "";
      Product.qu_id_stock = "";
      Product.min_stock_amount = "";
      Product.default_best_before_days = "";
      Product.row_created_timestamp = "";
      Product.group_id = "";
      Product.picture_file_name = "";
      Product.default_best_before_days_after_open = "";
      Product.enable_tare_weight_handling = "";
      Product.tare_weight = "";
      Product.not_check_stock_fulfillment_for_recipes = "";
      Product.last_shopping_location_id = "";
      Product.stock_amount = "";

      Barcode.id = "";
      Barcode.product_id = "";
      Barcode.barcode = "";
      Barcode.qu_id = "";
      Barcode.shopping_location_id = "";
      Barcode.amount = 0;
    }
    http.end();
    return httpResponseCode;
  };

  int barcode_add(String BC, float amount) {
    //##################################
    //ADD Barcode

    String serverPath = serverName + "api/stock/products/by-barcode/" + BC + "/add";
    http.begin(serverPath.c_str());
    http.addHeader("Content-Type", "application/json");
    http.addHeader("GROCY-API-KEY", API_key);

    //generate json data
    DynamicJsonDocument doc(1024);
    doc["amount"] = amount;
    doc["transaction_type"] = "purchase";
    String json;
    serializeJson(doc, json);

    // Send HTTP POST
    int httpResponseCode = http.POST(json);

    if (httpResponseCode == 200) {
      beep_succes();
      Serial.print("Product ");
      Serial.print(Product.name);
      Serial.println(" addet to stock");
      error_message = "";
    } else {
      beep_error();
      Serial.print("Errorx code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);

      DynamicJsonDocument err(6144);
      deserializeJson(err, payload);
      String error = err["error_message"];
      error_message = error;
    }
    http.end();
    return httpResponseCode;
  };

  int barcode_consume(String Barcode, float amount) {
    //##################################
    //consume Barcode

    String serverPath = serverName + "api/stock/products/by-barcode/" + Barcode + "/consume";
    http.begin(serverPath.c_str());
    http.addHeader("Content-Type", "application/json");
    http.addHeader("GROCY-API-KEY", API_key);

    DynamicJsonDocument doc(1024);
    doc["amount"] = amount;
    doc["transaction_type"] = "consume";

    String json;
    serializeJson(doc, json);

    // Send HTTP POST
    int httpResponseCode = http.POST(json);

    if (httpResponseCode == 200) {
      beep_succes();
      Serial.print("Product ");
      Serial.print(Product.name);
      Serial.println(" counsumed");
      error_message = "";


    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      beep_error();

      DynamicJsonDocument err(6144);
      deserializeJson(err, payload);
      String error = err["error_message"];
      error_message = error;
    }
    http.end();
    return httpResponseCode;
  };


private:

  void beep_error() {
    digitalWrite(14, LOW);
    delay(30);
    digitalWrite(14, HIGH);
    delay(100);
    digitalWrite(14, LOW);
    delay(30);
    digitalWrite(14, HIGH);
    delay(100);
    digitalWrite(14, LOW);
    delay(30);
    digitalWrite(14, HIGH);
    delay(100);
    digitalWrite(14, LOW);
    delay(30);
    digitalWrite(14, HIGH);
  };
  void beep_succes() {
    digitalWrite(14, LOW);
    delay(40);
    digitalWrite(14, HIGH);
  };
};

grocy grocy_client;

class mqtt_dvice {
public:

  struct mqtt_states {
    String mode;
    String barcode;
    String product_name;
    String product_id;
    String product_stock_amount;
    int barcode_amount;
    int error_code;
    String error_message;
  };
  mqtt_states states;

  String mode = "";
  void send_discovery() {
    send_discovery_ip();
    delay(50);
    send_discovery_mode();
    delay(50);
    send_discovery_barcode();
    delay(50);
    send_discovery_barcode_amount();
    delay(50);
    send_discovery_product_name();
    delay(50);
    send_discovery_product_id();
    delay(50);
    send_discovery_product_stock_amount();
    delay(50);
    send_discovery_error_code();
    delay(50);
    send_discovery_error_message();
  };

  void send_state() {
    String MAC = WiFi.macAddress();
    MAC.replace(":", "");
    String mqttName = "Easy_Scanner_" + MAC;
    String stateTopic = mqttName + "/state";

    DynamicJsonDocument doc(2048);
    char buffer[2048];

    doc["mode"] = states.mode;
    doc["barcode"] = states.barcode;
    doc["product_name"] = states.product_name;
    doc["product_id"] = states.product_id;
    doc["product_stock_amount"] = states.product_stock_amount;
    doc["barcode_amount"] = states.barcode_amount;
    doc["error_code"] = states.error_code;
    doc["error_message"] = states.error_message;
    doc["ip"] = WiFi.localIP();
    size_t n = serializeJson(doc, buffer);

    mqttClient.beginMessage(stateTopic);
    mqttClient.print(buffer);
    mqttClient.endMessage();

    //    stateTopic = mqttName + "/avty";
    //    mqttClient.beginMessage(stateTopic);
    //    mqttClient.print("online");
    //    mqttClient.endMessage();
  };

private:

  void send_discovery_ip() {

    String MAC = WiFi.macAddress();
    MAC.replace(":", "");
    String mqttName = "Easy_Scanner_" + MAC;
    String discoveryTopic = "homeassistant/sensor/" + mqttName + "/ip/config";
    String stateTopic = mqttName + "/state";

    DynamicJsonDocument doc(2048);
    char buffer[2048];

    doc["name"] = "IP";
    doc["unique_id"] = mqttName + "/ip";
    doc["state_topic"] = stateTopic;
    doc["value_template"] = "{{ value_json.ip}}";
    doc["icon"] = "mdi:check-network";
    //######### Device ########
    doc["dev"]["ids"] = WiFi.macAddress();
    doc["dev"]["name"] = "ESP-32 Easy-Scanner";
    doc["dev"]["manufacturer"] = "XDROLLOXD";
    doc["dev"]["model"] = "ESP-32 Barcode Sanner V2.0";
    doc["dev"]["configuration_url"] = "https://github.com/XDROLLOXD/ESP32_Grocy_Barcode_Scanner";

    size_t n = serializeJson(doc, buffer);

    mqttClient.beginMessage(discoveryTopic);
    mqttClient.print(buffer);
    mqttClient.endMessage();
  };

  void send_discovery_mode() {

    String MAC = WiFi.macAddress();
    MAC.replace(":", "");
    String mqttName = "Easy_Scanner_" + MAC;
    String discoveryTopic = "homeassistant/select/" + mqttName + "/mode/config";
    String stateTopic = mqttName + "/state";

    DynamicJsonDocument doc(2048);
    char buffer[2048];

    doc["name"] = "mode";
    doc["unique_id"] = mqttName + "/mode";
    doc["command_topic"] = mqttName + "/mode";
    doc["state_topic"] = stateTopic;
    doc["value_template"] = "{{ value_json.mode}}";
    JsonArray options = doc.createNestedArray("options");
    options.add("scan");
    options.add("scan and consume");
    options.add("scan and add");


    //doc["icon"] = "mdi:check-network";
    //######### Device ########
    doc["dev"]["ids"] = WiFi.macAddress();

    size_t n = serializeJson(doc, buffer);
    if (MQTT_enable == true) {
      mqttClient.beginMessage(discoveryTopic);
      mqttClient.print(buffer);
      mqttClient.endMessage();
    };
  };

  void send_discovery_barcode() {

    String MAC = WiFi.macAddress();
    MAC.replace(":", "");
    String mqttName = "Easy_Scanner_" + MAC;
    String discoveryTopic = "homeassistant/sensor/" + mqttName + "/barcode/config";
    String stateTopic = mqttName + "/state";

    DynamicJsonDocument doc(2048);
    char buffer[2048];

    doc["name"] = "barcode";
    doc["unique_id"] = mqttName + "/barcode";
    doc["state_topic"] = stateTopic;
    doc["value_template"] = "{{ value_json.barcode}}";
    doc["icon"] = "mdi:barcode";
    //######### Device ########
    doc["dev"]["ids"] = WiFi.macAddress();

    size_t n = serializeJson(doc, buffer);

    mqttClient.beginMessage(discoveryTopic);
    mqttClient.print(buffer);
    mqttClient.endMessage();
  };

  void send_discovery_product_name() {

    String MAC = WiFi.macAddress();
    MAC.replace(":", "");
    String mqttName = "Easy_Scanner_" + MAC;
    String discoveryTopic = "homeassistant/sensor/" + mqttName + "/product_name/config";
    String stateTopic = mqttName + "/state";

    DynamicJsonDocument doc(2048);
    char buffer[2048];

    doc["name"] = "product_name";
    doc["unique_id"] = mqttName + "/product_name";
    doc["state_topic"] = stateTopic;
    doc["value_template"] = "{{ value_json.product_name}}";
    doc["icon"] = "mdi:rename-outline";
    //######### Device ########
    doc["dev"]["ids"] = WiFi.macAddress();

    size_t n = serializeJson(doc, buffer);

    mqttClient.beginMessage(discoveryTopic);
    mqttClient.print(buffer);
    mqttClient.endMessage();
  };

  void send_discovery_product_id() {

    String MAC = WiFi.macAddress();
    MAC.replace(":", "");
    String mqttName = "Easy_Scanner_" + MAC;
    String discoveryTopic = "homeassistant/sensor/" + mqttName + "/product_id/config";
    String stateTopic = mqttName + "/state";

    DynamicJsonDocument doc(2048);
    char buffer[2048];

    doc["name"] = "product_id";
    doc["unique_id"] = mqttName + "/product_id";
    doc["state_topic"] = stateTopic;
    doc["value_template"] = "{{ value_json.product_id}}";
    doc["icon"] = "mdi:rename-outline";
    //######### Device ########
    doc["dev"]["ids"] = WiFi.macAddress();

    size_t n = serializeJson(doc, buffer);

    mqttClient.beginMessage(discoveryTopic);
    mqttClient.print(buffer);
    mqttClient.endMessage();
  };

  void send_discovery_product_stock_amount() {

    String MAC = WiFi.macAddress();
    MAC.replace(":", "");
    String mqttName = "Easy_Scanner_" + MAC;
    String discoveryTopic = "homeassistant/sensor/" + mqttName + "/product_stock_amount/config";
    String stateTopic = mqttName + "/state";

    DynamicJsonDocument doc(2048);
    char buffer[2048];

    doc["name"] = "product_stock_amount";
    doc["unique_id"] = mqttName + "/product_stock_amount";
    doc["state_topic"] = stateTopic;
    doc["value_template"] = "{{ value_json.product_stock_amount}}";
    doc["icon"] = "mdi:counter";
    //######### Device ########
    doc["dev"]["ids"] = WiFi.macAddress();

    size_t n = serializeJson(doc, buffer);

    mqttClient.beginMessage(discoveryTopic);
    mqttClient.print(buffer);
    mqttClient.endMessage();
  };

  void send_discovery_barcode_amount() {

    String MAC = WiFi.macAddress();
    MAC.replace(":", "");
    String mqttName = "Easy_Scanner_" + MAC;
    String discoveryTopic = "homeassistant/sensor/" + mqttName + "/barcode_amount/config";
    String stateTopic = mqttName + "/state";

    DynamicJsonDocument doc(2048);
    char buffer[2048];

    doc["name"] = "barcode_amount";
    doc["unique_id"] = mqttName + "/barcode_amount";
    doc["state_topic"] = stateTopic;
    doc["value_template"] = "{{ value_json.barcode_amount}}";
    doc["icon"] = "mdi:counter";
    //######### Device ########
    doc["dev"]["ids"] = WiFi.macAddress();

    size_t n = serializeJson(doc, buffer);

    mqttClient.beginMessage(discoveryTopic);
    mqttClient.print(buffer);
    mqttClient.endMessage();
  };

  void send_discovery_error_code() {

    String MAC = WiFi.macAddress();
    MAC.replace(":", "");
    String mqttName = "Easy_Scanner_" + MAC;
    String discoveryTopic = "homeassistant/sensor/" + mqttName + "/error_code/config";
    String stateTopic = mqttName + "/state";

    DynamicJsonDocument doc(2048);
    char buffer[2048];

    doc["name"] = "error_code";
    doc["unique_id"] = mqttName + "/error_code";
    doc["state_topic"] = stateTopic;
    doc["value_template"] = "{{ value_json.error_code}}";
    doc["icon"] = "mdi:alert-circle";
    //######### Device ########
    doc["dev"]["ids"] = WiFi.macAddress();

    size_t n = serializeJson(doc, buffer);

    mqttClient.beginMessage(discoveryTopic);
    mqttClient.print(buffer);
    mqttClient.endMessage();
  };

  void send_discovery_error_message() {

    String MAC = WiFi.macAddress();
    MAC.replace(":", "");
    String mqttName = "Easy_Scanner_" + MAC;
    String discoveryTopic = "homeassistant/sensor/" + mqttName + "/error_message/config";
    String stateTopic = mqttName + "/state";

    DynamicJsonDocument doc(2048);
    char buffer[2048];

    doc["name"] = "error_message";
    doc["unique_id"] = mqttName + "/error_message";
    doc["state_topic"] = stateTopic;
    doc["value_template"] = "{{ value_json.error_message}}";
    doc["icon"] = "mdi:alert-circle";
    //######### Device ########
    doc["dev"]["ids"] = WiFi.macAddress();

    size_t n = serializeJson(doc, buffer);

    mqttClient.beginMessage(discoveryTopic);
    mqttClient.print(buffer);
    mqttClient.endMessage();
  };
};

mqtt_dvice homeassistant;

void setup() {


  Serial.begin(115200);
  pinMode(14, OUTPUT);
  digitalWrite(14, HIGH);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());



  Serial1.begin(9600, SERIAL_8N1, 25, 26);


  //GM60 Setup
  gm60.begin(Serial1);
  gm60.encode(/*encode = */ gm60.eUTF8);
  gm60.setupCode(/*on =*/true, /*content=*/true);
  gm60.setIdentify(/*berCode = */ gm60.eEnableAllBarcode);
  Serial.println("Start to recognize");

  if (MQTT_enable == true) {
    // 1 = scan and consume
    // 2 = scan and add to stock
    if (mode == 1) {
      homeassistant.mode = "scan and consume";
    };
    if (mode == 2) {
      homeassistant.mode = "scan and add";
    };

    ref_state = 1;

    //##################### connect MQTT ##################
    mqttClient.setUsernamePassword(mqttUser, mqttPassword);
    Serial.println("Connecting to MQTT");

    //client.setServer(mqttServer, mqttPort);
    //client.connect(mqttName.c_str(), mqttUser, mqttPassword);
    while (!mqttClient.connected()) {
      Serial.print(".");

      if (mqttClient.connect(mqttServer, mqttPort)) {

        Serial.println("Connected to MQTT");
        delay(1000);

        homeassistant.send_discovery();

        //subscribing IO Topics
        String MAC = WiFi.macAddress();
        MAC.replace(":", "");
        String mqttName = "Easy_Scanner_" + MAC;
        String ioTopic = mqttName + "/mode";
        mqttClient.subscribe(ioTopic);

      } else {

        Serial.println("failed with state ");
        //Serial.print(client.state());
        delay(1000);
      }
    }
  };
}

void loop() {

  Barcode = gm60.detection();
  //Barcode = "6666";

  //delay time  = 1000ms
  if (Barcode != "null" && lastTime + 1000 <= millis()) {

    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("Barcode scanned: ");
      Serial.println(Barcode);

      //Load Barcode Data and store them
      //you can use them as you like
      //for example grocy_client.Barcode.amount
      //or grocy_client.Product.name

      float amount = 0.0;
      homeassistant.states.error_code = grocy_client.barcode_load_data(Barcode);
      if (homeassistant.states.error_code == 200) {

        if (mode == 1 && !MQTT_enable || MQTT_enable && homeassistant.mode == "scan and consume") {
          //function for consuming Barcodes
          grocy_client.barcode_consume(Barcode, grocy_client.Barcode.amount);
        };
        if (mode == 2 && !MQTT_enable || MQTT_enable && homeassistant.mode == "scan and add") {
          //function for adding Products to Stock
          grocy_client.barcode_add(Barcode, grocy_client.Barcode.amount);
        };
      };

      //homeassistant.states.mode;
      homeassistant.states.barcode = Barcode;
      homeassistant.states.product_name = grocy_client.Product.name;
      homeassistant.states.product_id = grocy_client.Product.id;
      homeassistant.states.product_stock_amount = grocy_client.Product.stock_amount;
      homeassistant.states.barcode_amount = grocy_client.Barcode.amount;
      //homeassistant.states.error_code;
      homeassistant.states.error_message = grocy_client.error_message;

      ref_state = 1;
      lastTime = millis();

    } else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }


  //Receiving Messages
  String MAC = WiFi.macAddress();
  MAC.replace(":", "");
  String mqttName = "Easy_Scanner_" + MAC;

  String message;
  int messageSize = mqttClient.parseMessage();
  if (messageSize) {
    // we received a message, print out the topic and contents
    Serial.println("message recived");
    String masseage_topic = mqttClient.messageTopic();
    message = "";
    while (mqttClient.available()) {
      message = message + (char)mqttClient.read();
    };


    //Mode received
    String ioTopic = mqttName + "/mode";
    if (masseage_topic == ioTopic) {
      Serial.println("mode recived");
      if (homeassistant.mode != message) {
        Serial.println(homeassistant.mode);
        Serial.println("changed to");
        Serial.println(message);
        homeassistant.mode = message;
      }
    }

    ref_state = 1;
  }


  if (MQTT_enable && (mqtt_ref_time + 60000 < millis() || ref_state)) {

    if (mqtt_ref_time + 60000 < millis()) {

      homeassistant.states.barcode = "";
      homeassistant.states.product_name = "";
      homeassistant.states.product_id = "";
      homeassistant.states.product_stock_amount = "";
      homeassistant.states.barcode_amount = 0;
      homeassistant.states.error_code = 0;
      homeassistant.states.error_message = "";
    }

    homeassistant.states.mode = homeassistant.mode;
    homeassistant.send_state();

    mqtt_ref_time = millis();
    ref_state = 0;
  }
}
