# TM 1637 by hand

Arduino sketch written for interfacing four digit display with chipset TM1637,
without any third party libraries.

## About TM1637

The TM1637 display is chipset driving a four digit seven segment display, 
which uses a signaling similar to I2C, except by the fact that it 
doesn't use a peripheral address and the bytes are transmitted from LSB to MSB.

It has four pins: CLK, DIO, VCC, GND
- CLK: Serial clock. Similar to I2C SCL pin
- DIO: Data IO pin. Similar to I2C SDA pin
- VCC: Logical positive voltage supply (up to 7V, but 5V is typical).
- GND: Ground pin

### Displaying numbers

To put numbers in the display send four commands in the following order:

|start|command 1|stop|start|command 2|data|stop|start|command 3|stop|

- Command 1: Display mode = We will use value 0x40, indicating we will write in normal mode, fixed address setting.
- Command 2: Address setting = We will use value 0XCn, starting with address 0xC0 for the leftmost digit.
- Data: here comes the data sent to the address informed into command 2. Note that it follows the command 2, without restarting the communication. After sent the byte with data to be written, send a stop signal.
- Command 3: Bright Setting: Here we will send bright setting. TM1637 has eight levels of bright. The command is 0x8n, where _n_ is one of 8 levels of bright. 0x8F is the brightest setting.


For example, let's send packages to put the number one in the rightmost digit with display into lowest bright setting:
```
| start | 0x40 | stop | start | 0xC3 | 0x06 | stop | start | 0x88 | stop |
``` 
The 0x06 is the encoded value to light the right segments on the display.

#### Number encoding

TM1637 uses the following format for converting data bytes to seven segment display: 0bHGFEDCBA.
```
   __A__
F |     | B
   __G__
E |     | C
   __D__   _H

```
The eighth bit is used for the decimal dot. We won't use it here because we don't have decimal dots on this display.

So, for displaying the number one, we need to light segments B and C. Using the format presented before, our value to be sent is: 0x000000110 => 0x06. Apply the same method to display the other digits. All the formats are described into method `encode_digit`.
