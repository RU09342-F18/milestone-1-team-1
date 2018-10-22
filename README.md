# Milestone 1: Communicating with Will Byers
This milestone utilizes PWM and UART to custimize the brightness and color of an RGB LED. CCR registers corresponding with
the 3 
colors are used to initialize PWM. The duty cycle of each color can be manipulated through this by using UART to 
send packets of bytes
with hex values to dictate the duty cycle of each color of the LED. These values range from 0x00 to 0xFF
in scale. If a packet is sent
that holds information about more than just 1 set of colors for an LED, the micorcontroller 
will pass the rest of the bytes on to
a node connected to the Tx pin that is assigned to the board.

