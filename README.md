SparkFun Qwiic MP3 Trigger
========================================

![SparkFun Qwiic MP3 Trigger](https://cdn.sparkfun.com//assets/parts/1/3/5/5/5/15165-SparkFun_Qwiic_MP3_Trigger-01.jpg)

[*SparkFun Qwiic MP3 Trigger (DEV-15165)*](https://www.sparkfun.com/products/15165)

This repo is the hardware repo for the Qwiic MP3 Trigger.

Sometimes you just need an MP3 to play. Whether it's a theme song as you enter the room or a pirate cackling when a dollar [gets donated](https://github.com/nseidle/Money_Vacuum) to the kid's museum. The Qwiic MP3 Trigger takes care of all the necessary bits, all you need to do is send a simple I2C command and listen.

The Qwiic MP3 Trigger comes loaded with everything a user may need. Connecting a USB C cable enumerates the Qwiic MP3 Trigger as a jump drive. Simply plug in the Qwiic MP3 Trigger and you'll be transferring MP3s. No need for drivers and no need for WAV or Vorbis conversion! Sound output is provided via a headphone jack or poke-home connector allowing an external speaker to be connected without soldering. The speaker is boosted by a Class-D mono amplifier capable of outputting up to 1.4W. What does 1.4W mean? It's incredibly loud; great for making sure your mech effects are heard on the *con floor and wonderful for annoying your officemates. Volume is software selectable between 32 levels. Equalization can be tuned to be sure your classical hits sound different from your jazz dance routines (in all seriousness EQ selection is available but it's pretty paltry).

And if you don't want to deal with *any* programming, there are four trigger pins. When pin 3 is pulled low the T003.mp3 file will immediately be played. This allows you to start playing sound effects with the touch of a button! By pulling multiple pins down simultaneously the four triggers can play up to ten tracks: T001 to T010.

All settings including volume, EQ, and I2C address are stored in NVM and loaded at each power up. The I2C address of the Qwiic MP3 Trigger can be modified via a solder jumper or be assigned using a software command. Multiple Qwiic MP3 Triggers can be chained together on a single bus allowing for simultaneous track mixing and triggering.

We've written an extensive Arduino library to make MP3 playing over I2C a breeze. Play track X, change volume, play next/previous, check if track is playing, stop play, change EQ, and change I2C address are all supported.

The I2C address of Qwiic MP3 Trigger is software configurable which means you can hookup over 100 on a single I2C bus!

This board is one of our many [Qwiic](https://www.sparkfun.com/qwiic) compatible boards! Simply plug and go. No soldering, no figuring out which is SDA or SCL, and no voltage regulation or translation required!

SparkFun labored with love to create this code. Feel like supporting open source hardware? 
Buy a [breakout board](https://www.sparkfun.com/products/15165) from SparkFun!

Repository Contents
-------------------

* **/Documents** - Datasheets
* **/Hardware** - Eagle design files (.brd, .sch)

Documentation
--------------
* **[Qwiic MP3 Trigger Hookup Guide](https://learn.sparkfun.com/tutorials/qwiic-mp3-trigger-hookup-guide)** - Hookup guide for the Qwiic MP3 Trigger
* **[Installing an Arduino Library Guide](https://learn.sparkfun.com/tutorials/installing-an-arduino-library)** - Basic information on how to install an Arduino library.
* **[Arduino Library](https://github.com/sparkfun/SparkFun_Qwiic_MP3_Trigger_Arduino_Library)** - Library if you decide to use an Arduino via serial I2C.

License Information
-------------------

This product is _**open source**_! 

If you have any questions or concerns on licensing, please contact techsupport@sparkfun.com.

Please use, reuse, and modify these files as you see fit. Please maintain attribution to SparkFun Electronics and release any derivative under the same license.

Distributed as-is; no warranty is given.

- Your friends at SparkFun.
