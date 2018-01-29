# AVR RF Transmitter

Here lies the code powering my 433mhz RF transmitter.  This transmitter supports 11 buttons and an analog stick, providing lots of potential inputs for any use case I might have for the receiving end circuit.  I utilized a cheap superheterodyne transmitter for this circuit which communicates via USART.  After some experimentation balancing between transmitter range and data able to be transferred per second, I've landed at a 2400 baud rate.  A complete data packet is transmitted roughly every 35 milliseconds.  I've also added in an easily configurable sleep mode when there are periods of inactivity in the name of battery preservation.

### What's in a data packet?

A packet is comprised of seven different bytes:

1. `U` - This is our training byte.  The binary value of `U` is `01010101`, which gives any receivers' data slicers a nice square wave to sync up with.  This makes them more receptive to the rest of our packet, which is important because that's where our actual data is.
2. `ª` - This is the start byte, which signals to any receiver that the bytes that follow are data bytes.  The binary value of 'ª' is 10101010, which should also hopefully help train the data slicers of our receivers to be more receptive to this sender.
3. `button_byte` - Each bit in this byte corresponds to the status of our 8 non-special buttons that are on the top face of the transmitter.  If a bit in this byte is `1`, it means the associated button is currently pressed.
4. `misc_byte` - A byte containing a conglomerate of bits that didn't fit anywhere else.  Here we have three bits corresponding to the pressed status of our left shoulder button, right shoulder button, and the button on the analog stick.  We also have the two most significant bits of both the x-axis and y-axis analog stick values.  The analog values of each axis are of 10-bit resolution, so rather than allocating two whole bytes for each one we instead put these MSBs here.
5. `lsb_analog_stick_x_byte` - A byte containing the 8 least significant bits of the x-analog stick value.
6. `lsb_analog_stick_y_byte` - A byte containing the 8 least significant bits of the y-analog stick values.
7. `checksum` - Finally, we have our checksum byte.  The checksum is calculated by adding up all our data bytes (so no training characters).  Any overflow is ignored.  This gives the receiver a simple (and admittedly not perfect) way to ensure that the data they've received is valid.