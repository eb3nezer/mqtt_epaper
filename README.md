# mqtt_epaper

ESP8266 code to display weather on an ePaper display getting data from MQTT. This
uses Waveshare's universal e-paper driver board
https://www.waveshare.com/product/modules/oleds-lcds/e-paper/e-paper-esp8266-driver-board.htm

It displays the time and date in the top left (synchronised to NTP), the current
weather conditions on the left, and the system status in the bottom left.

The right side shows the indoor and outdoor temperature and humidity, along with the
barometric pressure.

The code is very dependent on the data that my home automation system sends,
but maybe you will find it useful in building your own ePaper display.

## Setup

In mqtt_epaper.ino I define 2 TimeChangeRule objects. Set these to be the start and end of
daylight saving time in your location.

The rest of the setup is web based. When you turn it on, if it fails to connect to a WiFi access
point using WiFiManager (e.g. because this is the very first time you turned it on, and it's not configured), it
will create its own access point, and show the access point name on the screen.

Connect to that access point (e.g. from your phone). Tap to scan for access points and select yours. Enter the password
for your access point. Configure the MQTT server, port, and password, and the topic that updates will be received on. Also
configure an NTP server that it can use to receive weather updates.

Once configured the status section (lower left of screen) will indicate if it is connected to the network, if it is
connected to MQTT, and if it has seen a weather update recently.

## MQTT Messages

For temperature data it is expecting a JSON blob on the configured topic that looks something like this:

```
{
  "outdoor_temp":"230",
  "indoor_temp":"217",
  "outdoor_hum":"431",
  "indoor_hum":"422",
  "pressure":"10074"
}
```

Note that the values are multiplied by 10, so 230 means 23.0 and 10074 means 1007.4.

For weather conditions the data it expects is like this:

```
{
  "weather":"cloudy"
}
```

I'm using Dark Sky to generate the conditions, so it should understand the terms that Dark Sky
uses.

## Home Assistant

To generate sendor updates in the form above you can use a Home Assistant automation rule like following (line breaks added for clarity):

```
- alias: Publish temperature stats to MQTT
  id: f35ed1d16a9b4faebad4fcb032f733
  trigger:
  - platform: time
    minutes: '/2'
    seconds: 30
  action:
    service: mqtt.publish
    data_template:
      topic: "my/epaper/temp"
      payload: "{\"outdoor_temp\":\"{{ (states('sensor.temperature_outdoor')|float * 10) | round(0) }}\",
	    \"indoor_temp\":\"{{ (states('sensor.temperature_indoor') | float * 10) | round(0) }}\",
		\"outdoor_hum\":\"{{ (states('sensor.humidity_outdoor') | float * 10) | round(0) }}\",
		\"indoor_hum\":\"{{ (states('sensor.humidity_indoor') | float * 10) | round(0) }}\",
		\"pressure\":\"{{ (states('sensor.pressure_outdoor')|float * 10) | round(0) }}\"}"
```

To send updates from the configured Dark Sky module, you can use an automation rule like:

```
- alias: Publish weather conditions
  id: f35ed1d16a9b4faebad4fcb032f744
  trigger:
  - platform: time
    minutes: '/1'
    seconds: 40
  action:
    service: mqtt.publish
    data_template:
      topic: "my/epaper/temp"
      payload: "{\"weather\":\"{{ states('weather.dark_sky') }}\"}"
```

## Driving ePaper

This project uses a 4.2" display from Waveshare. I used the GxEPD2 and Adafruit_GFX
libraries. The 4.2" display allows partial updates, and I found that it worked quite
well for this kind of project.

With the ESP8266 based ePaper driver board the hardware setup is extremely simple.
Connect the ribbon cable from the display to the board, and you're done.

## A Case

I have the display module inside a photo frame, with the driver board at the back.
I designed a small case in OpenSCAD that has screw holes to allow you to mount the
case onto the back. You can see the case on Thingiverse at https://www.thingiverse.com/thing:3576172

## Improvements

Still to do:
* Display daily min/max graphs in the bottom right
* Display whether the valus are increasing or decreasing with an arrow
* Test OTA update more (not sure it actually works)
* Change Home Assistant to only send updates when values change, and then do a partial update of the screen for only that figure.