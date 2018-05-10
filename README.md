SparkFun Qwiic MP3
========================================

![SparkFun Qwiic MP3](https://cdn.sparkfun.com//assets/parts/1/2/9/2/9/14714-Qwiic_MP3-01.jpg)

[*SparkX Qwiic MP3 (SPX-14714)*](https://www.sparkfun.com/products/14714)

Sometimes you just need an MP3 to play. Whether it's a sound track as you enter the room or a pirate cackling when a dollar [gets donated](https://github.com/nseidle/Money_Vacuum) to the kid's museum. The Qwiic MP3 takes care of all the necessary bits, all you need to do is send a simple I2C command and listen.

We've tried to think of everything a user may need. The Qwiic MP3 comes with a microB connector and the WT2003S MP3 decoder IC that gives you access to the files on the microSD card. Simply plug in the Qwiic MP3 and you'll be transferring MP3s. No need for drivers and no need for WAV or Vorbis conversion! Sound output is provided via a headphone jack or poke-home connector allowing an external speaker to be connected without soldering. The speaker is boosted by a Class-D mono amplifier capable of outputting up to 1.4W. What does 1.4W mean? It's incredibly loud; great for making sure your mech effects are heard on the *con floor and wonderful for annoying your officemates. Volume is software selectable between 32 levels. Equalization can be tuned to be sure your classical hits sound different from your jazz dance routines (in all seriousness EQ selection is available but it's pretty paltry).

And if you don't want to deal with *any* programming, there are four trigger pins. When pin 3 is pulled low the T003.mp3 file will immediately be played. This allows you to start playing sound effects with the touch of a button! By pulling multiple pins down simultaneously the four triggers can play up to ten tracks: T001 to T010.

All settings including volume, EQ, and I2C address are stored in NVM and loaded at each power up. The I2C address of the Qwiic MP3 can be modified via a solder jumper or be assigned using a software command. Multiple Qwiic MP3s can be chained together on a single bus allowing for simultaneous track mixing and triggering.

We've provided a full suite of example sketches to get you started including: play track X, change volume, play next/previous, check if track is playing, stop play, change EQ, and change I2C address.

For a limited time we are including a 512MB microSD card with every Qwiic MP3 purchased. These are perfect to store over 300 minutes of MP3s.

This board is one of our many [Qwiic](https://www.sparkfun.com/qwiic) compatible boards! Simply plug and go. No soldering, no figuring out which is SDA or SCL, and no voltage regulation or translation required!


SparkFun labored with love to create this code. Feel like supporting open source hardware? 
Buy a [breakout board](https://www.sparkfun.com/products/14714) from SparkFun!

Repository Contents
-------------------

* **/Documents** - Datasheets
* **/Hardware** - Eagle design files (.brd, .sch)

License Information
-------------------

This product is _**open source**_! 

If you have any questions or concerns on licensing, please contact techsupport@sparkfun.com.

Please use, reuse, and modify these files as you see fit. Please maintain attribution to SparkFun Electronics and release any derivative under the same license.

Distributed as-is; no warranty is given.

- Your friends at SparkFun.