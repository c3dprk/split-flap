### makes the flaps flip without the original electronics. working prototype.

Using a common ESP8266 based device like the Wemos D1, this can control a single split-flap device, useful if you have only one or two.

It comes with a simple webserver that requires SPIFFS. See: [Arduino ESP8266FS tool](http://esp8266.github.io/Arduino/versions/2.3.0/doc/filesystem.html#uploading-files-to-file-system)


You will need:

An ESP8266, a 5V supply, something that can level shift from 3.3V to 5V, an AC source at 24(?) - 48 V.

Some soldering required.

In order to reliably trigger the TRIAC, a 5V level is required.

AC Voltage for the motor is required.



TODO: some css would be nice



