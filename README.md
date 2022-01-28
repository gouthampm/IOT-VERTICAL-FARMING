# IOT-VERTICAL-FARMING
the tds sensor is connected to pin A0 of esp8266
the temperature and humidity sensor is connected to pin D3 of the esp8266
the relay for the pump is connected to pin D2 of esp8266 and provided with a 5v extenal power supply
millis function is used for the pump instead of delay to prevent the blocking of the sensor data
the pump is turned on for 3 minutes every 12minute interval as for a standard aeroponic system
however the as esp8266 is a single core microcontroller so at the time of 3minute delay there is a break in live sensor data for 3 minutes which is only a minor issue
the data from the dht11 temperature and humidity sensor and the tds sensor is updated to thingspeak.com where these parameters are closley monitored
we are also planning to include a ph sensor in the future and also a notification and automatic control system for the ph and tds control.
