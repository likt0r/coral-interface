# ESP32 Program for Servo Control with Adafruit PCA9685
This ESP32 program leverages the capabilities of the Adafruit PCA9685 board to control servos, moving them at varying velocities from 0° to 180°.

## State Machine

Upon each button press, a state machine is activated. This state machine controls the servo's movements, causing one of the servos to halt in its last state.
## JSON Data and HTTP API

After each operation, the final positions of all servos are collated into a JSON array. This JSON data is then sent to a specified HTTP API endpoint.

# dummy-server
This project includes a simple Node.js web server designed specifically for ESP32 testing purposes. The server features a single endpoint that receives payload from the ESP32 program.

When a payload is received, it's immediately printed to the console, providing a real-time view of the transmitted data. This setup is perfect for validating and debugging the data sent from your ESP32 program.

```
nodejs dummy-server/index.js
```

