# Hiome RFM69 Gateway

When Hiome sensors detect something, they broadcast it via RFM69 radios to the gateway. The gateway
is a Raspberry Pi that processes that data and acts accordingly. In order to receive messages, it
needs its own RFM69 radio. This code receives and pipes messages over serial to your Raspberry Pi.

## Dependencies

```bash
git clone git@github.com:Hiome/SPIFlash.git
git clone -b skipcansend git@github.com:hiome/RFM69.git
```

Your gateway `NODEID` should always be 1, and it must have the same `NETWORKID` and `ENCRYPTKEY` as
every other node on your network. The `SERIAL_BAUD` needs to be consistent with whatever your
Raspberry Pi is expecting.
