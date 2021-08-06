# Simple TCP client for the ESP-32

## Developed with
* ESP-IDF v4.2.1
## Tested on
* ESP-32 Dev Kit C
## Build instructions
Make sure to run [install.sh](https://github.com/espressif/esp-idf/blob/master/install.sh) in [esp-idf](https://github.com/espressif/esp-idf) to
setup the environment.
### using make
`make menuconfig` Select "Example Configuration" and enter WIFI SSID, WIFI Password,
TCP Server IP and TCP Server Port. Save the changes by pressing the 'S' Key.<br />
`make` <br />
connect the ESP-32<br />
`make flash` <br />
to view stdout and the errorlog of the ESP run <br />
`make monitor`
### using idf.py
`idf.py menuconfig` <br />
`idf.py flash` port can be specified using -p (default is /dev/tty/USB0) see 
`idf.py --help` <br />
`idf.py monitor`
