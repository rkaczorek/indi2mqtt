# indi2mqtt
indi2mqtt is a MQTT bridge to INDI. It monitors devices connected to an INDI server
and publishes their properties to a MQTT server.

# Building
This assumes you already have INDI installed.

First, let's install some build tools, if you haven't already.
```
sudo apt install build-essential cmake
```

You must install INDI and Mosquitto development libraries.
```
sudo apt install libindi-dev libmosquitto-dev
```

Okay, we're ready to get the source and build it.
```
git clone https://github.com/rkaczorek/indi2mqtt.git
cd indi2mqtt
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
```

# Running
Start your INDI server
Start indi2mqtt
Enjoy MQTT topic of your choice

Now you can monitor your INDI devices with MQTT!
