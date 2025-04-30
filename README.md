# Always on-line
I have a self hosted NAS/server at home and I want to always have ability to connect to it. However, for that to happen I need my modem and my router to be always online. However, if there is a power surge or an issue with the device, the modem or router might need a reset. This project uses a Wi-Fi connected Arduino to monitor the internet connect and a relay reset devices connected to it.

The board pings the router, remote page (google dns), and local device on interval. If it cannot connect to the router or the remote server it will shutdown the relay which should be mounted between the power outlet and the modem/router. 

## Requirements and setup 

### Arduino board setup
If you are using a 3rd party Arduino board you may need to install the drivers for the computer to recognize the device. Download the drivers [here](https://ftdichip.com/drivers/vcp-drivers/).

### Arduino IDE Setup  
1. Install Arduino IDE
1. In Arduino IDE open settings and under "Additional Boards Manager URLs" add `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
1. In Arduino IDE package manager add:
    - ESPping

### Hardware 
- Arduino board with an ESP8266 Wi-Fi chip 
- A 3-5V relay
- Electric cables (salvaged from a lamp for instance)
- Arduino jumper cables 
- Male and female power plugs (ones you can wire)

### Hardware Connection 
1. Use electric cable to connect the neutral slot (for American style plugs it is the long slot) of the male power plug and the female power plug. 
1. Use electric cable to connect the hot slot of the male power plug to the `NO` terminal on the relay. 
1. Use electric cable to connect the hot slot of the female power plug to `C` terminal on the relay.
1. Connect a jumper cable from Arduino `Vin` to `VCC`
1. Connect a jumper cable from Arduino `GND` to `GND`
1. Connect a jumper cable from Arduino `D4` to `IN` 

Note that the pin outs labels on the ESP8266 boards do not correspond to the labelled pins. [See the pin out diagram](https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/). 

### Firmware setup 
1. Make a copy of `secrets_sample.h` and rename it to `secrets.h`
1. Edit `secrets.h`
    - Change the SSID of the network you want to monitor 
    - Add the network password 
1. Edit the `config.ino` file 
    - Change `device_ip` to the preferred IP address for the controller 
    - Change `router_ip` to the default gateway
    - (Optional) Change the `subnet` 
    - You can get these values from a computer on the network by running `ipconfig /all` on Windows in command prompt
    - Change `server_ip` to a device you want to monitor (not actually in use right now)
    - You can also edit the intervals at which each of these devices will be pinged and how many failures are allowed before a reset is triggered by editing `interval_xxxx` and `max_xxxx_fails` where `xxxx` represents either relay, server, or remote. 
1. Upload the sketch to the Arduino and enjoy! 

# Future development 
- [ ] Add wake on LAN magic packet capability for the cases where the server is not reachable 
- [x] Move the user configurable variables to a separate file 