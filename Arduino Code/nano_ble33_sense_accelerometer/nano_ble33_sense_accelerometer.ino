/* Edge Impulse Arduino examples
 * Copyright (c) 2021 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Includes ---------------------------------------------------------------- */
#include <CEN598_finalproject_v3_inferencing.h>
#include <Arduino_LSM9DS1.h>
#include <ArduinoBLE.h>


BLEService sendingInt("16150f38-e7a9-4fe1-ae08-48464bf25b2");
BLEStringCharacteristic sendInt("50b837d1-db86-4d6d-bb4d-e0b725ba0932", BLERead | BLEWrite, 100);

/* Constant defines -------------------------------------------------------- */
#define CONVERT_G_TO_MS2    9.80665f
#define MAX_ACCEPTED_RANGE  2.0f        // starting 03/2022, models are generated setting range to +-2, but this example use Arudino library which set range to +-4g. If you are using an older model, ignore this value and use 4.0f instead

/* Private variables ------------------------------------------------------- */
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal

/**
* @brief      Arduino setup function
*/
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
      pinMode(LEDR, OUTPUT);
      pinMode(LEDG, OUTPUT);
      pinMode(LEDB, OUTPUT);
      digitalWrite(LEDR, HIGH);   //set LED RGB off
      digitalWrite(LEDG, HIGH); //set LED RGB off
      digitalWrite(LEDB, HIGH); //set LED RGB off
      Serial.begin(115200);
      digitalWrite(LED_BUILTIN, HIGH);   // if powered on, make Arduino Board blink once
      delay(1000);                       //
      digitalWrite(LED_BUILTIN, LOW);    //
      delay(1000);
      if(!IMU.begin()) {
        Serial.println("Failed to initialize IMU!");
        while(1);
      }
      if (!BLE.begin()) {
        Serial.println("Starting BLE failed!");
        while (1);
      }
    
      // set advertise local name and service UUID
      BLE.setLocalName("SendInteger");
      BLE.setAdvertisedService(sendingInt);
    
      // add the characteristic to the service
      sendingInt.addCharacteristic(sendInt);
      BLE.addService(sendingInt);
          
      //start advertising
      BLE.advertise();
      Serial.println("Bluetooth device active waiting for connections!");

      
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println("Edge Impulse Inferencing Demo");

    if (!IMU.begin()) {
        ei_printf("Failed to initialize IMU!\r\n");
    }
    else {
        ei_printf("IMU initialized\r\n");
    }

    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 3) {
        ei_printf("ERR: EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME should be equal to 3 (the 3 sensor axes)\n");
        return;
    }
}

/**
* @brief      Printf function uses vsnprintf and output using Arduino Serial
*
* @param[in]  format     Variable argument list
*/
void ei_printf(const char *format, ...) {
   static char print_buf[1024] = { 0 };

   va_list args;
   va_start(args, format);
   int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
   va_end(args);

   if (r > 0) {
       Serial.write(print_buf);
   }
}

/**
 * @brief Return the sign of the number
 * 
 * @param number 
 * @return int 1 if positive (or 0) -1 if negative
 */
float ei_get_sign(float number) {
    return (number >= 0.0) ? 1.0 : -1.0;
}

/**
* @brief      Get data and run inferencing
*
* @param[in]  debug  Get debug info if true
*/
void loop()
{

    int currTime=millis();
  if((currTime%1000)>500){  //make Blue LED blink when bluetooth not connected to the base station
    digitalWrite(LEDB, HIGH);   
  }
  else if((currTime%1000)<=500){
    digitalWrite(LEDB, LOW);   
  }
  BLEDevice central = BLE.central();
  if (central) {
    // print the central' MAC address
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    
    // while central is still connected to peripheral
    while (central.connected()) {
      digitalWrite(LEDB, LOW);  //when bluetooth connected to the base station, Blue LED always turned on

    ei_printf("Sampling...\n");

    // Allocate a buffer here for the values we'll read from the IMU
    float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

    for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 3) {
        // Determine the next tick (and then sleep later)
        uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);

        IMU.readAcceleration(buffer[ix], buffer[ix + 1], buffer[ix + 2]);

        for (int i = 0; i < 3; i++) {
            if (fabs(buffer[ix + i]) > MAX_ACCEPTED_RANGE) {
                buffer[ix + i] = ei_get_sign(buffer[ix + i]) * MAX_ACCEPTED_RANGE;
            }
        }

        buffer[ix + 0] *= CONVERT_G_TO_MS2;
        buffer[ix + 1] *= CONVERT_G_TO_MS2;
        buffer[ix + 2] *= CONVERT_G_TO_MS2;

        delayMicroseconds(next_tick - micros());
    }

    // Turn the raw buffer in a signal which we can the classify
    signal_t signal;
    int err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
        ei_printf("Failed to create signal from buffer (%d)\n", err);
        return;
    }

    // Run the classifier
    ei_impulse_result_t result = { 0 };

    err = run_classifier(&signal, &result, debug_nn);
    if (err != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
        return;
    }

    // print the predictions
    ei_printf("Predictions ");
    ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
        result.timing.dsp, result.timing.classification, result.timing.anomaly);
    ei_printf(": \n");

    float maxnum=0.6;   //20220331edit
    String res= "uncertain";  //20220331edit
    
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
        if(result.classification[ix].value > maxnum){ // get the prediction result which has the highest value
          maxnum=result.classification[ix].value;
          res=result.classification[ix].label;
        }
    }
    sendInt.writeValue(res + String(maxnum) + "end of line"); //send the prediction result using bluetooth
    
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("    anomaly score: %.3f\n", result.anomaly);
#endif

    }
    // turn off the BLE after the BLE reader is disconnected
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: "); Serial.println(central.address());    
  }  
  //delay(50);
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_ACCELEROMETER
#error "Invalid model for current sensor"
#endif
