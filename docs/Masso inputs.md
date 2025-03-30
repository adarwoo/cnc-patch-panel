## Wiring guide

This is the list of connectors, functions and connections.

### List of Masso inputs of interest

   | MASSO In# | Function Type             | Type         | To#  |
   |-----------|---------------------------|--------------|------|
   | (1-1/6) 1 | X-limit                   | Limit switch | -    |
   | (1-2/6) 2 | Y-limit                   | Limit switch | -    |
   | (1-3/6) 3 | Z-limit                   | Limit switch | -    |
   | (1-4/6) 4 | Probe input               | Touch probe  | -    |
   | (1-5/6) 5 | Tool setter               | Tool Setter  | -    |
   | (1-6/6) 6 | Spindle Coolant Flw Alarm | Alarm        | -    |
   | (2-1/6) 7 | Spindle Motor Alarm       | Alarm        | -    |
   | (2-2/6) 8 | X Motor Alarm             | Alarm        | -    |
   | (2-3/6) 9 | Y Motor Alarm             | Alarm        | -    |
   | (2-4/6)10 | Z Motor Alarm             | Alarm        | -    |
   | (2-5/6)11 | -                         | -            | -    |
   | (2-6/6)12 | -                         | -            | -    |
   | (3-1/6)13 | Cycle Start Input         | Button       | J1-1 |
   | (3-2/6)14 | Home Machine Input        | Button       | J1-2 |
   | (3-3/6)15 | Stop Button Input         | Button       | J1-3 |
   | (3-4/6)16 | Go to Home Machine        | Button       | J1-4 |
   | (3-5/6)17 | Go to Parking Position    | Button       | J1-5 |
   | (3-6/6)18 | Chuck Clamp/Unclamp       | Button       | J1-6 |
   | (4-1/6)19 | Door Open/Close           | Button       | J2-1 |
   | (4-2/6)20 | Autoload G-Code 1         | Button       | J2-2 |
   | (4-3/6)21 | Autoload G-Code 2         | Button       | J2-3 |
   | (4-4/6)22 | Autoload G-Code 3         | Button       | J2-4 |
   | (4-5/6)23 | Door Sensor Input         | Sensor       | J2-5 |
   | (4-6/6)24 | Air Pressure Low Alarm    | Alarm        | J2-6 |


### List of Masso outputs of interrest

   | MASSO Out# | Function type            | Type         | To#  | OC#  | TTL Input |
   |------------|--------------------------|--------------|------|------|----|
   | (1-1/6)  1 | Tower Light Red          | Indicator    | J3-1 | J5-1 | 1  |
   | (1-2/6)  2 | Tower Light Yellow       | Indicator    | J3-2 | J5-2 | 2  |
   | (1-3/6)  3 | Tower Light Green        | Indicator    | J3-3 | J5-3 | 3  |
   | (1-4/6)  4 | Door Open/Close          | Command      | J3-4 | -    | 4  |
   | (1-5/6)  5 | Tool Air Blast Cleaning  | Pneumatic    | J3-5 | -    | 5  |
   | (1-6/6)  6 | Chuck Clamp              | Pneumatic    | J3-6 | -    | 6  |
   | (2-1/7)  7 | Touch Screen Beep Output | Sound        | J4-7 | -    | 7  |
   | (2-2/7)  8 | Sounder                  | Sound        | J4-6 | -    | 8  |
   | (2-3/7)  9 | [Laser Crosshair +]      | Command      | J4-5 | -    | 9  |
   | (2-4/7) 10 | [Camera-Light On/Off]    | Command      | J4-4 | J5-4 | 10 |
   | (2-5/7) 11 | Spare Input 1            | Spare        | J4-3 | J5-5 | 11 |
   | (2-6/7) 12 | Spare Input 2            | Spare        | J4-2 | J5-6 | 12 |
   | (2-7/7) ES | Emergency Stop           | ES           | J4-1 | J5-7 | 13 |


### List of CNC Center OC outputs

   | J5-# | Function                 |
   |------|--------------------------|
   | J5-1 | [Tower Light Red]        |
   | J5-2 | [Tower Light Yellow]     |
   | J5-3 | [Tower Light Green]      |
   | J5-4 | [Laser Crosshair +]      |
   | J5-5 | [Camera-Light On/Off]    |
   | J5-6 | Stepper motor release    |
   | J5-7 | Emergency Stop           |

### List of CNC Center OC inputs

#### Door closed sensor

   | J#-# | Door closed              |
   |------|--------------------------|
   | J6-1 | +24V                     |
   | J6-2 | NPN Collector in         |
   | J6-3 | GND                      |

#### Door opened sensor

   | J#-# | Door opened              |
   |------|--------------------------|
   | J7-1 | +24V                     |
   | J7-2 | NPN Collector in         |
   | J7-3 | GND                      |

#### Spindle on from VFD

   | J#-# | Spindle on (from VFD)    |
   |------|--------------------------|
   | J8-1 | NPN Collector in         |
   | J8-2 | GND                      |

### List of CNC Center TTL inputs

   | Function Type            | J#-# |PCA# |TTL# |
   |--------------------------|------|-----|-----|
   | Tower Light Red          | J3-6 | 0-5 |  1  |
   | Tower Light Yellow       | J3-5 | 0-4 |  2  |
   | Tower Light Green        | J3-4 | 0-3 |  3  |
   | Door Open/Close          | J3-3 | 0-2 |  4  |
   | Tool Air Blast Cleaning  | J3-2 | 0-1 |  5  |
   | Chuck Clamp              | J3-1 | 0-0 |  6  |
   | Sounder                  | J4-7 | 1-4 |  7  |
   | Touch Screen Beep Output | J4-6 | 1-3 |  8  |
   | [Laser Crosshair +]      | J4-5 | 1-2 |  9  |
   | [Camera-Light On/Off]    | J4-4 | 1-1 | 10  |
   | [Spare Input 1]          | J4-3 | 1-0 | 11  |
   | [Spare Input 2]          | J4-2 | 0-7 | 12  |
   | Emergency Stop           | J4-1 | 0-6 | 13  |

### List of CNC Center TTL outputs

   | Function Type            | J#-# | PCA# |
   |--------------------------|------|------|
   | Cycle Start Button Input | J1-1 | 0    |
   | Home Machine Input       | J1-2 | 1    |
   | Stop Button Input        | J1-3 | 2    |
   | Go to Home Machine       | J1-4 | 3    |
   | Go to Parking Position   | J1-5 | 4    |
   | Chuck Clamp/Unclamp      | J1-6 | 5    |
   | Door Open/Close          | J2-1 | 6    |
   | Autoload G-Code 1        | J2-2 | 8    |
   | Autoload G-Code 2        | J2-3 | 9    |
   | Autoload G-Code 3        | J2-4 | 10   |
   | Door Sensor Input        | J2-5 | 14   |
   | Air Pressure Low Alarm   | J2-6 | 15   |

### LEDs

   | D#  | Color  | Function Type           | Comment | PCA# |
   |-----|--------|-------------------------|---------|------|
   | D5  |<span style="color: green;">green</span>  |OC in door opened       |         | 0    |
   | D6  |<span style="color: green;">green</span>  |OC in door closed       |         | 1    |
   | D7  |<span style="color: green;">green</span>  |OC in spindle is on	  |         | 2    |
   | D8  |<span style="color: green;">green</span>  |OC in spare	           |         | 3    |
   | D9  |<span style="color: yellow;">yellow</span>|OC out release steppers |         | 4    |
   | D10 |<span style="color: yellow;">yellow</span>|OC ES                   |         | 5    |
   | D11 |<span style="color: green;">green</span>  |Comm stat relay         | on=ok<br/>blink=error<br/>off=down | 6 |
   | D12 |<span style="color: green;">green</span>  |Comm stat Hub           |         | 7    |
   | D13 |<span style="color: green;">green</span>  |Comm stat Console       |         | 8    |
   | D14 |<span style="color: red;">red</span>      |Low pressure            |         | 9    |
   | D15 |<span style="color: orange;">orange</span>|Air Blast	              |         | 10   |
   | D16 |<span style="color: orange;">orange</span>|Spindle clean           |         | 11   |
   | D17 |<span style="color: green;">green</span>  |Door opening status     | blink=opening<br/>on=error | 12 |
   | D18 |<span style="color: orange;">orange</span>|Door closing status     |         | 13   |
