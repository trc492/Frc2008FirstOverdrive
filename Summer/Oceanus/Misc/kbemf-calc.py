#Kbemf-calc.py  Written by Paul Malmsten, Titan Robotics Club, 2008
verified = False

while verified != True:
    a = input("Max Motor RPM:")
    b = input("Max Motor Voltage(1 direction, PWM value):")
    c = input("Sampling Delay(ms):")
    d = input("Encoder CPR:")

    print "\nExpected max ticks/unit time:", ((a/60)*(c/1000))*d

    if str(raw_input("Is this correct? (y/n):")) == "y":
             verified = True

#Nasty bit of math here. Okay; so the first thing we do
#is convert the max RPM into rotations per unit time. To do this,
#we take the max RPM/60 which gets us rotations/second,
#multiplied by the percentage delay in the sampling rate
#for that single second. This gives us how many rotations
#occur during a single sampling period at full speed. Next
#we multiply that by the encoder Counts Per Rotation to get
#the max velocity in encoder ticks. Finally, to make sense of
#it all, we take the maximum voltage PWM value for the motor
#and divide it by the corresponding encoder ticks/per unit
#time value at full speed to print a Kemf coefficient, in units
#of PWM increments per encoder click. Kemf can then be used
#by:
#
#Drive += (velocity * Kemf)
#
#with Drive as the PWM output and velocity in encoder ticks per
#specified unit time to compensate for Back EMF at a given velocity.

print "Kbemf:", b/(((a/60)*(c/1000))*d), "\n"
raw_input("Press enter to exit...");
