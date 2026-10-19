base library for native rjoy and touch telemetry to allow support for modded games with a virtual
USB interface.

in theory the limitation that caused this mod to never happen before is data transfer bottleneck

BUT my thoughts are the USB attachment communication is ONLY the method of setting memory storage for addressing not the mechanism in which to change those addressed values.

without thinking or testing, my thoughts is a hardware fpga or microcontroller joystick value smush to compress two floats into the address space of one.

so the microcontroller is powered by and dictates required on-psp address space through the USB bus while handling communication of two joys through a single joystick line.

there is some calculation to be done on a prl in userland tho. this is where you would set the second joystick in the periphral address space.

this also might require some attenuation on compression algorithm implemention vs. lowering of float precision.
