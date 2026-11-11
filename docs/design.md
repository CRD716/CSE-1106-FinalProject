# System Overview

There are two physical inputs, a virtual input, and both physical and virtual outputs in the system. There is the entry and exit buttons, as well as the maximum occupancy argument on the command line. For outputs, there is the maximum occupancy light, as well as the csv log and the terminal outputs.

# Physical Overview

A single ground line is run from the raspberry pi to the breadboard ground rail, GPIO pins are connected to one side of each button, and the other side of each button is bridged to the ground rail. Another GPIO is used to connect to a resistor that connects to an LED light,which is bridged back to ground

# Code Overview

The main code loop checks for the moment when either the entry or exit button is released after being pressed. It then calls the personEnters and PersonExits functions, which check for capacity and and logs the entry or exit. showStatus is called each time, and both updates the LED and prints the current occupancy and status.