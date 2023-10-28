//Librarys
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DFRobot_GM60.h>
DFRobot_GM60_UART gm60;
#include <SoftwareSerial.h>



//Setup

//Wifi SSID
const char* ssid = "SSID";

//Wifi Password
const char* password = "Password";

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.???.???:9192/";

//Grocy API Key
String API_key = "Grocy API KEY";





//Scanned Barcode
String Barcode = "0";

HTTPClient http;

unsigned long lastTime = 0;

unsigned long timerDelay = 5000;

class grocy {

public:

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

    } else {
      beep_error();
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
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
    } else {
      beep_error();
      Serial.print("Errorx code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
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


    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      beep_error();
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
      if (grocy_client.barcode_load_data(Barcode) == 200) {

        //function for adding Products to Stock
        //grocy_client.barcode_add(Barcode, grocy_client.Barcode.amount);

        //function for consuming Barcodes
        grocy_client.barcode_consume(Barcode, grocy_client.Barcode.amount);
      };


      lastTime = millis();

    } else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
