// This #include statement was automatically added by the Particle IDE.
#include "application.h" //Notwendig damit die externe RGB-LED schon beim Booten leuchtet!
#include "HX711.h"
#include "Particle.h"  
#include "Adafruit_DHT.h"
#include "SparkFunMAX17043.h" // Include the SparkFun MAX17043 library


//********************************************************************
// Automatically mirror the onboard RGB LED to an external RGB LED
// No additional code needed in setup() or loop()

class ExternalRGB {
  public:
    ExternalRGB(pin_t r, pin_t g, pin_t b) : pin_r(r), pin_g(g), pin_b(b) {
      pinMode(pin_r, OUTPUT);
      pinMode(pin_g, OUTPUT);
      pinMode(pin_b, OUTPUT);
      RGB.onChange(&ExternalRGB::handler, this);
    }

    void handler(uint8_t r, uint8_t g, uint8_t b) {

      analogWrite(pin_r, r);
      analogWrite(pin_g, g);
      analogWrite(pin_b, b);
    }

private:
      pin_t pin_r;
      pin_t pin_g;
      pin_t pin_b;
};

// Connect an external RGB LED to D0, D1 and D2 (R, G, and B)
ExternalRGB myRGB(D0, D1, D2);

//********************************************************************

// DHT humidity/temperature sensors
#define DHTPIN3 3     // what pin we're connected to
#define DHTPIN4 4

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11		// DHT 11
#define DHTTYPE3 DHT22		// DHT 22 (AM2302)
#define DHTTYPE4 DHT22		// DHT 22 (AM2302)
//#define DHTTYPE DHT21		// DHT 21 (AM2301)

DHT dht_pin3(DHTPIN3, DHTTYPE3);
DHT dht_pin4(DHTPIN4, DHTTYPE4);

//HX711 Wägezellenverstärker
#define DOUT  A0
#define CLK  A1

HX711 scale(DOUT, CLK);

String str_scalefactor = "";
String str_offset = "";

float offset = 0;
float scalefactor = 1;

float floatGewicht = 0;
String stringGewicht = "";

float floatHumidity3 = 0;
String stringHumidity3 = "";

float floatTemperature3 = 0;
String stringTemperature3 ="";

float floatHumidity4 = 0;
String stringHumidity4 = "";

float floatTemperature4 = 0;
String stringTemperature4 ="";

double soc = 0; // Variable to keep track of LiPo state-of-charge (SOC)
String stringSOC = "";


void setup() {
  // put your setup code here, to run once:
  // Begin serial communication
    Serial.begin(115200);

    // Listen for the webhook response, and call gotWeatherData()
    Particle.subscribe("hook-response/get_scalefactor", gotScalefactor, MY_DEVICES);
    Particle.subscribe("hook-response/get_offset", gotOffset, MY_DEVICES);

    // Give ourselves 10 seconds before we actually start the
    // program.  This will open the serial monitor before
    // the program sends the request
    for(int i=0;i<10;i++) {
        Serial.println("waiting " + String(10-i) + " seconds before we publish");
        delay(1000);
    }

    // publish the event that will trigger our Webhook
    Particle.publish("get_scalefactor");
    delay(5000);
    Particle.publish("get_offset");
    delay(5000);
    scalefactor = str_scalefactor.toFloat();
    offset = str_offset.toFloat();

    scale.set_scale(scalefactor);                      //this value is obtained by calibrating the scale with known weights;
                                                 /*   How to Calibrate Your Scale
                                                      1.Call set_scale() with no parameter.
                                                      2.Call set_tare() with no parameter.
                                                      3.Place a known weight on the scale and call get_units(10).
                                                      4.Divide the result in step 3 to your known weight. You should get about the parameter you need to pass to set_scale.
                                                      5.Adjust the parameter in step 4 until you get an accurate reading.
                                                  */


}

void loop() {

    scale.power_up();
    delay(5000);
    //scale.get_units(10) returns the medium of 10 measures
    floatGewicht = (scale.get_units(10) - offset);
    stringGewicht =  String(floatGewicht, 2);

    scale.power_down();

    //Begin DHT communication
      dht_pin3.begin();
      dht_pin4.begin();

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a
    // very slow sensor)
      delay(5000);
    	floatHumidity3 = dht_pin3.getHumidity();
      stringHumidity3 = String(floatHumidity3, 2),
    // Read temperature as Celsius
    	floatTemperature3 = dht_pin3.getTempCelcius();
      stringTemperature3 = String(floatTemperature3, 2);

      floatHumidity4 = dht_pin4.getHumidity();
      stringHumidity4 = String(floatHumidity4, 2),
    // Read temperature as Celsius
    	floatTemperature4 = dht_pin4.getTempCelcius();
      stringTemperature4 = String(floatTemperature4, 2);

    // Set up the MAX17043 LiPo fuel gauge:
      lipo.begin(); // Initialize the MAX17043 LiPo fuel gauge

    // Quick start restarts the MAX17043 in hopes of getting a more accurate
    // guess for the SOC.
      lipo.quickStart();
      delay(1000);

    // lipo.getSOC() returns the estimated state of charge (e.g. 79%)
	    soc = lipo.getSOC();
      stringSOC = String(soc);
      delay(1000);

      Particle.publish("cloud4bees", JSON(), PRIVATE); // Send JSON Particle Cloud

      //delay(30000);

      System.sleep(SLEEP_MODE_DEEP, 600);

}

// This function will get called when scalefactor comes in
void gotScalefactor(const char *name, const char *data) {
    str_scalefactor = String(data);
}

// This function will get called when offset comes in
void gotOffset(const char *name, const char *data) {
    str_offset = String(data);
}


String JSON() {
 String ret = "&field1=";
  ret.concat(stringGewicht);
  ret.concat("&field2=");
  ret.concat(stringTemperature3);
  ret.concat("&field3=");
  ret.concat(stringHumidity3);
  ret.concat("&field4=");
  ret.concat(stringTemperature4);
  ret.concat("&field5=");
  ret.concat(stringHumidity4);
  ret.concat("&field6=");
  ret.concat(stringSOC);


  return ret;
}
