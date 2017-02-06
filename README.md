# Sustainable Green Plants
![Basic kit](doc/img/basickitexample.jpg?raw=true "Basic kit version in the window")

The development is currently in alpha, but you can of course try it out. [These images](https://www.dropbox.com/sh/nx1g1m9xj6oz451/AABnalTH5EBVytOSFOYaLh_Da?dl=0 "Images in Dropbox"), along with the details I provide below, can help you to get started. 

# Introduction
The goal of this repository is to contain all the information needed to get, build and run an aeroponics system.
The idea is to design a cheap modular system that anyone can use.

# Small and modular DIY kit
Using a small and modular DIY kit can be very useful for growing herbs or other small plants you prefer.
The parts cost around USD 57 (SEK 500) and to run the basic kit in the window costs about USD 0.11 (SEK 1) per month. If you, like me, live in Sweden and need some extra light during winter, you can use LEDs which can cost you USD 1 (SEK 9) per month - a price example of using LEDs from IKEA.

## How to get
To get the parts, go to the [wiki](https://github.com/rhagman/sustainable-green-plants/wiki) and look under the [small and modular DIY kit](https://github.com/rhagman/sustainable-green-plants/wiki/Small-and-modular-DIY-kit) section. There you have a list of what you need to get and also a [list of materials](https://github.com/rhagman/sustainable-green-plants/wiki/Small-and-modular-DIY-kit#list-of-materials) for which I suggested some links to buy from and also the how much I paid for them.

## How to build 
As I mentioned, building the kit is currently under construction, so for now you can look at these [images](https://www.dropbox.com/sh/nx1g1m9xj6oz451/AABnalTH5EBVytOSFOYaLh_Da?dl=0 "Images in Dropbox") and copy it as best as you can. Once everything is connected you simply upload the arduino sketch to your arduino board. When you plug in the arduino uno or press the reset button, the arduino assumes that the time is 20:00 and runs the pump to fill up the PET bottles with nutrient solution. Then it will spray the roots at a set interval. At 10:00 the pump will once again run and fill up the PET bottles.

## How to run
To run the kit is also currently under construction, but basically you mix 10g of [nutrients](https://github.com/rhagman/sustainable-green-plants/wiki/Small-and-modular-DIY-kit#nutrient-solution) into 4L of water and add it to the vase. Then, you add about 2L of water 2 times a week to the vase and once a month you instead add 2L of the same nutrient mix as before.

# Large and modular DIY kit
Still under construction. It will be added as soon as it is done.

# Software instructions
* arduino_sketch - contains the arduino sketch file which you need to upload to your arduino uno.

# License
MIT
