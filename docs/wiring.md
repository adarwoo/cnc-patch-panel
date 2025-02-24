### Wiring: Patch board to the Console (SubD-9)

From the standard: Modbus over Serial Line V1.02

- **A(-)** = Pin 9 - Brown
- **B(+)** = Pin 5 - Yellow
- **C(0V)** = Pin 1 - Grey

#### Pin Configuration:

| Pin# | Function     | Wire Color |
|------|--------------|------------|
| 1    | Common/GND   | Grey       |
| 2    | VDD (+24V)   | Red        |
| 3    | NC           | -          |
| 4    | VDD          | White      |
| 5    | B(+)         | Yellow     |
| 6    | GND          | Black      |
| 7    | Vcc (5V)     | Blue       |
| 8    | Vcc          | Green      |
| 9    | A(-)         | Brown      |

#### Cable Matting:

- **From the patch board (energy source):** Female
- **At the console:** Male (receiving)
- **Cable:** SubD-9 Male-Female 20AWG, 55cm

### Consideration for the LEDs

General error AVR's I/O

#### LED assignment table and functions

| Led#     | Color        | Function                  | Detail                                                                 |
|----------|--------------|---------------------------|------------------------------------------------------------------------|
| 1,2,3,4  | Green        | Open collectors inputs    |                                                                        |
| 5,6,7,8  | Yellow       | Open collectors outputs   |                                                                        |
| 9        | Green        | Relay comm status         | on=OK<br/>blink=error<br/> off=down                                           |
| 10       | Green        | OC Hub comm status        |                                                                        |
| 11       | Red          | Pneumatic hub comm status |                                                                        |
| 12       | Red          | Pneumatic Low pressure    | on=Low pressure                                                        |
| 13       | Orange       | Air blast                 |                                                                        |
| 14       | Orange       | Spindle blast             |                                                                        |
| 15/16    | Green/Orange | Door status               | green blink=Opening<br/>orange blink=Closing<br/>green on=Closed<br/>orange on=opened<br/>both on=error |
---
