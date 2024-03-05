# LPD8806-with-FS1000A
LPD8806 strip led receiving commands via a radio module called FS1000A in Arduino
# The transmitter

The transmitter side only needs a radio transmitter module and the virtualWire library to work.

The Radio.h class is only needed on the transmitter side because it takes care of converting the RGB values, measuring the length of a JSON and sending messages.

# The receiver

The receiver gets 4 variables from the transmitter :
- (int) the animation you want
- (uint32_t) the color of the leds
- (int) the speed at wich the animation will be played
- (int) the part in wich the animation will be played (in this case there is three parts)

*The receiver will not receive the message well sometimes so you might want to send the message multiples times on the transmitter side*

The receiver has 5 different animations :
- Knight_rider - Two segments start from each side until they reach the other side and return
- OneTwo - Alternate between the odd and even leds
- Rainbow - It's a rainbow
- Flash - Lights up one led at a time at a random place
- Splash - Random segments light up until the strip is completly lit and the flashes the whole strip for a defined amount of times and starting again


