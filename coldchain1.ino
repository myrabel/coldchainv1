#include <String.h>
#include <TinyGPS++.h>
#include "DHT.h"
#include <SoftwareSerial.h>
#include <Adafruit_MPU6050.h>
#include<Adafruit_Sensor.h>
#include <Wire.h>
#include <DHT.h>


const int MPU = 0x68; // MPU6050 I2C address
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll, pitch, yaw;
float elapsedTime, currentTime, previousTime;
int c = 0;

#define DHTPIN 2
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

Adafruit_MPU6050 mpu;

TinyGPSPlus gps;

SoftwareSerial myGsm(4,3);
SoftwareSerial myGps(6,5);

double latitude,longitude;

float humidity, temperature;

int hour, minute, second, day, month, year;

String timestamp, temp_threshold, humid_threshold;

void setup()
{
  myGsm.begin(9600);   // the GPRS baud rate   
  Serial.begin(9600);    // the USB baud rate
  myGps.begin(9600);   // the GPS baud rate
  dht.begin();
  Wire.begin();                      // Initialize comunication
  Wire.beginTransmission(MPU);       // Start communication with MPU6050 // MPU=0x68
  Wire.write(0x6B);                  // Talk to the register 6B
  Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
  Wire.endTransmission(true);        //end the transmission

  // Call this function if you need to get the IMU error values for your module
  calculate_IMU_error();
 
  delay(1000);
}
 
void loop()
{
  delay(2000);

 while(myGps.available()) {
  GpsData();
 }  

DhtReading();
ReadAccelerom();
                  
  if (myGsm.available())
    Serial.write(myGsm.read());
    
Gsm();

SendSms();


// delay(600000);
} 

// void ShowSerialData()
// {
//   while(myGsm.available()!=0)
//   Serial.write(myGsm.read());
//   delay(5000); 
  
// }

void GpsData() {
  int data = myGps.read();

  if (gps.encode(data))

  {

     latitude = (gps.location.lat());

     longitude = (gps.location.lng());

      day = (gps.date.day());

      month = (gps.date.month());

      year = (gps.date.year()) ;    

      hour = (gps.time.hour());

      minute = (gps.time.minute());

      second = (gps.time.second());
     

  // Serial.print("date: ");

  // Serial.print(day);

  // Serial.print("/");

  // Serial.print(month);

  // Serial.print("/");

  // Serial.println(year);  

  // Serial.print("time:");

  // Serial.print(hour);

  // Serial.print(":");  

  // Serial.print(minute);

  // Serial.print(":");  

  // Serial.println(second);

  // Serial.print ("lattitude: ");

  // Serial.println (latitude, 6);

  // Serial.print ("longitude: ");

  // Serial.println (longitude, 6);


   }

  timestamp = String(year) +"-"+String(month) +"-"+String(day) +"T"+String(hour) +":"+String(minute) +":"+String(second);
}

void DhtReading() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

// Serial.print("Temperature = ");
// Serial.print(temperature);
// Serial.println(" °C");
// Serial.print("Humidity = ");
// Serial.print(humidity);
// Serial.println(" %");

  delay(2000);   
}


void Gsm() {
  myGsm.println("AT");
  delay(1000);
 
  myGsm.println("AT+CPIN?");
  delay(1000);
 
  myGsm.println("AT+CREG?");
  delay(1000);
 
  myGsm.println("AT+CGATT?");
  delay(1000);
 
  myGsm.println("AT+CIPSHUT");
  delay(1000);
 
  myGsm.println("AT+CIPSTATUS");
  delay(2000);
 
  myGsm.println("AT+CIPMUX=0");
  delay(2000);
 
  // ShowSerialData();
 
  myGsm.println("AT+CSTT=\"safaricom\",\"saf\",\"data\"");//start task and setting the APN,
  delay(1000);
 
  // ShowSerialData();
 
  myGsm.println("AT+CIICR");//bring up wireless connection
  delay(3000);
 
  // ShowSerialData();
 
  myGsm.println("AT+CIFSR");//get local IP adress
  delay(2000);
 
  // ShowSerialData();

  myGsm.println("AT+CIPSPRT=0");
  delay(3000);
 
  // ShowSerialData();
  
  myGsm.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");//start up the connection
  delay(6000);
 
  // ShowSerialData();
 
  myGsm.println("AT+CIPSEND");//begin send data to remote server
  delay(4000);
  // ShowSerialData();
  
  String str="GET https://api.thingspeak.com/update?api_key=WKTCXF02PZEWF30X&field1=" + String(temperature) +"&field2="+String(humidity) +"&field3="+String(latitude,6) +"&field4="+String(longitude,6) +"&field5="+timestamp;
  Serial.println(str);
  myGsm.println(str);//begin send data to remote server
  
  delay(4000);
  // ShowSerialData();
 
  myGsm.println((char)26);//sending
  delay(5000);//waitting for reply, important! the time is base on the condition of internet 
  myGsm.println();
 
  // ShowSerialData();
 
  myGsm.println("AT+CIPSHUT");//close the connection
  delay(100);
  // ShowSerialData();
}

void SendSms() {
  // if(minute > 28 && minute < 42) {
    if(temperature > 20) {
      temp_threshold = "High";
    }

    else if(temperature > 10 && temperature < 20) {
      temp_threshold = "Medium";
    }

    else {
      temp_threshold = "Low";
    }

    if(humidity > 80) {
      humid_threshold = "High";
    }

    else if(humidity > 60 && humidity < 80) {
      humid_threshold = "Medium";
    }

    else {
      humid_threshold = "Low";
    }
    const String PHONE_One = "+254797341602";
    myGsm.print("AT+CMGF=1\r");
    delay(1000);
    myGsm.print("AT+CMGS=\""+PHONE_One+"\"\r");
    delay(1000);
    String message = "Temp Threshold: " +String(temp_threshold) +"\n" +"Temp: "  +String(temperature) +"\n" +"Hum Threshold: " +String(humid_threshold) +"\n" +"Hum: " +String(humidity) +"\n" +"Time: " +String(timestamp) +"\n" +"Roll: " +String(roll) + " " +"Yaw: " +String(yaw) + " " +"Pitch: " +String(pitch);
    //  + " " + "Yaw: " +String(yaw);
    myGsm.println(message);
    delay(100);
    myGsm.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
    delay(5000);
    // Serial.println(message);

  // }  
}

void calculate_IMU_error() {
  // We can call this funtion in the setup section to calculate the accelerometer and gyro data error. From here we will get the error values used in the above equations printed on the Serial Monitor.
  // Note that we should place the IMU flat in order to get the proper values, so that we then can the correct values
  // Read accelerometer values 200 times
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    // Sum all readings
    c++;
  }
  //Divide the sum by 200 to get the error value
  c = 0;
  // Read gyro values 200 times
  while (c < 200) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    GyroX = Wire.read() << 8 | Wire.read();
    GyroY = Wire.read() << 8 | Wire.read();
    GyroZ = Wire.read() << 8 | Wire.read();
    // Sum all readings
    c++;
  }

}

void ReadAccelerom() {
  // === Read acceleromter data === //
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  //For a range of +-2g, we need to divide the raw values by 16384, according to the datasheet
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis value
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0; // Y-axis value
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value
  // Calculating Roll and Pitch from the accelerometer data
  accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - 0.58; // AccErrorX ~(0.58) See the calculate_IMU_error()custom function for more details
  accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) + 1.58; // AccErrorY ~(-1.58)
  // === Read gyroscope data === //
  previousTime = currentTime;        // Previous time is stored before the actual time read
  currentTime = millis();            // Current time actual time read
  elapsedTime = (currentTime - previousTime) / 1000; // Divide by 1000 to get seconds
  Wire.beginTransmission(MPU);
  Wire.write(0x43); // Gyro data first register address 0x43
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 4 registers total, each axis value is stored in 2 registers
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0; // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
  // Correct the outputs with the calculated error values
  GyroX = GyroX + 0.56; // GyroErrorX ~(-0.56)
  GyroY = GyroY - 2; // GyroErrorY ~(2)
  GyroZ = GyroZ + 0.79; // GyroErrorZ ~ (-0.8)
  // Currently the raw values are in degrees per seconds, deg/s, so we need to multiply by sendonds (s) to get the angle in degrees
  gyroAngleX = gyroAngleX + GyroX * elapsedTime; // deg/s * s = deg
  gyroAngleY = gyroAngleY + GyroY * elapsedTime;
  yaw =  yaw + GyroZ * elapsedTime;
  // Complementary filter - combine acceleromter and gyro angle values
  roll = 0.96 * gyroAngleX + 0.04 * accAngleX;
  pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;
  
  // Print the values on the serial monitor
  // Serial.print("Roll:");
  // Serial.println(roll);
  // Serial.print("Pitch:");
  // Serial.println(pitch);
  // Serial.print("Yaw:");
  // Serial.println(yaw);
  delay(5000);
}
