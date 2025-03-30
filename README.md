# cnc-patch-panel
The Mobbus master and interface to the Masso

This device servers several purpose
1. It is the Modbus center for the CNC.
   As such, it takes Masso output signal, and drives them on Modbus (relays, pneumatic etc.)
   It interfaces with the console panel (buttons, leds, switches and sounder)
2. It drive the console
   Feed the Masso inputs
3. It converts Masso TTLs outputs in isolated Open Collector outputs or Modbus driven register
4. It implements the door control logic (TDB)

