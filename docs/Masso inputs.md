## Wiring guide

This is the list of connectors, functions and connections.

### List of Masso inputs of interest

   | MASSO In# | Function Type             | Type         | To#  |
   |-----------|---------------------------|--------------|------|
   | (1-1/6) 1 | X-limit                   | Limit switch | -    |
   | (1-2/6) 2 | Y-limit                   | Limit switch | -    |
   | (1-3/6) 3 | Z-limit                   | Limit switch | -    |
   | (1-4/6) 4 | Probe input               | Touch probe  |      |
   | (1-5/6) 5 | Tool setter               | Tool Setter  |      |
   | (1-6/6) 6 | Spindle Coolant Flw Alarm | Alarm        |      |
   | (2-1/6) 7 | Spindle Motor Alarm       | Alarm        |      |
   | (2-2/6) 8 | X Motor Alarm             | Alarm        |      |
   | (2-3/6) 9 | Y Motor Alarm             | Alarm        |      |
   | (2-4/6)10 | Z Motor Alarm             | Alarm        |      |
   | (2-5/6)11 | -                         | -            |      |
   | (2-6/6)12 | -                         | -            |      |
   | (3-1/6) 1 | Cycle Start Input         | Button       | J1-1 |
   | (3-2/6) 2 | Home Machine Input        | Button       | J1-2 |
   | (3-3/6) 3 | Stop Button Input         | Button       | J1-3 |
   | (3-4/6) 4 | Go to Home Machine        | Button       | J1-4 |
   | (3-5/6) 5 | Go to Parking Position    | Button       | J1-5 |
   | (3-6/6) 6 | Chuck Clamp/Unclamp       | Button       | J1-6 |
   | (4-1/6) 7 | Door Open/Close           | Button       | J2-1 |
   | (4-2/6) 8 | Autoload G-Code 1         | Button       | J2-2 |
   | (4-3/6) 9 | Autoload G-Code 2         | Button       | J2-3 |
   | (4-4/6)10 | Autoload G-Code 3         | Button       | J2-4 |
   | (4-5/6)11 | Door Sensor Input         | Sensor       | J2-5 |
   | (4-6/6)12 | Air Pressure Low Alarm    | Alarm        | J2-6 |


### List of Masso outputs of interrest

   | MASSO Out# | Function type            | Type         | To#  | OC#  |
   |------------|--------------------------|--------------|------|------|
   | (1-1/6)  1 | Tower Light Red          | Indicator    | J3-1 | J5-1 |
   | (1-2/6)  2 | Tower Light Yellow       | Indicator    | J3-2 | J5-2 |
   | (1-3/6)  3 | Tower Light Green        | Indicator    | J3-3 | J5-3 |
   | (1-4/6)  4 | Door Open/Close          | Command      | J3-4 | -    |
   | (1-5/6)  5 | Tool Air Blast Cleaning  | Pneumatic    | J3-5 | -    |
   | (1-6/6)  6 | Chuck Clamp              | Pneumatic    | J3-6 | -    |
   | (2-1/7)  7 | --                       | --           | J4-1 | -    |
   | (2-2/7)  8 | Touch Screen Beep Output | Sound        | J4-2 | -    |
   | (2-3/7)  9 | Sounder                  | Sound        | J4-3 | -    |
   | (2-4/7) 10 | [Light On / Off]         | Spare1       | J4-4 | J5-4 |
   | (2-5/7) 11 | [Laser Crosshair +]      | Spare2       | J4-5 | J5-5 |
   | (2-6/7) 12 | [Camera-Light On/Off]    | Spare3       | J4-6 | J5-6 |
   | (2-7/7) ES | Emergency Stop           | ES           | J4-7 | J5-7 |


### List of CNC Center OC outputs

   | J#-# | Function                 |
   |------|--------------------------|
   | J5-1 | [Tower Light Red]        |
   | J5-2 | [Tower Light Yellow]     |
   | J5-3 | [Tower Light Green]      |
   | J5-4 | [Light On / Off]         |
   | J5-5 | [Laser Crosshair +]      |
   | J5-6 | [Camera-Light On/Off]    |
   | J5-7 | Stepper motor release    |
   | J5-8 | Emergency Stop           |

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

   | Function Type            | J#-# | PCA# |
   |--------------------------|------|------|
   | Tower Light Red          | J3-1 | 0    |
   | Tower Light Yellow       | J3-2 | 1    |
   | Tower Light Green        | J3-3 | 2    |
   | Door Open/Close          | J3-4 | 3    |
   | Tool Air Blast Cleaning  | J3-5 | 4    |
   | Chuck Clamp              | J3-6 | 5    |
   | --                       | J4-1 | 8    |
   | Touch Screen Beep Output | J4-2 | 9    |
   | Sounder                  | J4-3 | 10   |
   | [Light On / Off]         | J4-4 | 11   |
   | [Laser Crosshair +]      | J4-5 | 12   |
   | [Camera-Light On/Off]    | J4-6 | 13   |
   | Emergency Stop           | J4-7 | 14   |

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
