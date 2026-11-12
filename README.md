# JPSine
a scientific calculator running on a raspberry pi pico, with a 2.42in oled screen, 40 buttons, using a 1000mAh 3.7V lipo battery.

# Design
the buttons are arranged in a matrix with 8 rows and 5 columns, with a diode for each button, so even if 2 buttons are pressed together theres no issue as it only lets current pass in one direction. one leg of all the leds in one row is passed through a diode and then connected together to a GPIO pin on the pico, and one leg of all leds in one column is tied together to another GPIO. this way, only 13 pins are needed to read 40 buttons - one for each row and column.
im using TP4056 module for charging the battery, and it also provides protection from over charging, over current, etc. the OLED Screen uses I2C, so it only needs 2 pins on the pico - SDA and SCL. it also has a slide switch for power, and a push button for reset.

Schematic
<img width="461" height="382" alt="image" src="https://github.com/user-attachments/assets/8eb844c7-d065-465f-bdb6-f95df98d68a4" />

PCB
<img width="245" height="354" alt="image" src="https://github.com/user-attachments/assets/95e3091a-6256-41cb-a254-109ee3f076e1" />

<img width="237" height="350" alt="image" src="https://github.com/user-attachments/assets/33577e3a-a908-42a9-9eab-4ff37fad1c2c" />

# BOM


| Item | Quantity | Total Cost | Source |
| :--- | :---: | :---: | :--- |
| Raspberry Pi Pico | 1 | $4.50 | Robu |
| 3.7V 1000mAh battery | 1 | $1.60 | Hubtronics |
| TP4056 module | 1 | $0.15 | Robu |
| 2.42in OLED screen | 1 | $15.00 | Zbotic |
| 10uF 0603 capacitor | 2 | $0.10 | Robu |
| 0.1uF 0603 capacitor | 2 | $0.10 | Robu |
| 100k 0603 resistor | 2 | $0.10 | Robu |
| 4.7k 0603 resistor | 2 | $0.10 | Robu |
| PTS 647 push buttons | 40 | $5.50 | Robu |
| SS14 Schottky diode | 1 | $0.10 | Robu |
| SOD-523 diode | 40 | $0.20 | Robu |
| PCM12SMTR slide switch | 1 | $0.60 | Robu |
| PCB | 1 | $4.00 | JLCPCB |


