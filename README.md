# Sustainable green plants
![Basic kit](doc/img/basickitexample.jpg?raw=true "Basic kit version in the window")

[Images from the build](https://www.dropbox.com/sh/nx1g1m9xj6oz451/AABnalTH5EBVytOSFOYaLh_Da?dl=0 "Images in Dropbox"),
development is currently in alpha so use on your own risk. The parts cost around USD 57 (SEK 500) and running the basic kit in the window cost about USD 0.11 (SEK 1) per month. If you like me live in Sweden and need some help from LEDs during the winter, it costs about USD 1 (SEK 9) per month with the LEDs from IKEA.

------

# Introduction
The goal of this repository is to contain all the information needed to get,
build and run an aeroponic system. The idea is to design a cheap modular system
that anyone can use.

# How to get, build and run the Small and modular DIY kit
To get the parts, go to the [wiki](https://github.com/rhagman/sustainable-green-plants/wiki) and look under the [Small and modular DIY kit](https://github.com/rhagman/sustainable-green-plants/wiki/Small-and-modular-DIY-kit) section. There you have a list of what you need to get and also a [list of materials](https://github.com/rhagman/sustainable-green-plants/wiki/Small-and-modular-DIY-kit#list-of-materials) of where to buy the materials and the prizes I payed for it.

To build the kit is currently under construction, so for now you can look at the [Images from the build](https://www.dropbox.com/sh/nx1g1m9xj6oz451/AABnalTH5EBVytOSFOYaLh_Da?dl=0 "Images in Dropbox") and copy it as best as you can. But in short, once everything is connected you simply upload the arduino sketch to your arduino board. When you plug in the arduino uno or press the reset button, the arduino assumes that the time is 20:00 and runs the pump to fill up the PET bottles with nutrient solution. Then it will spray the roots at a set interval. At 10:00 the pump will once again run and fill up the PET bottles.

To run the kit is currently under construction, but basicly you mix 10g of [nutrients](https://github.com/rhagman/sustainable-green-plants/wiki/Small-and-modular-DIY-kit#nutrient-solution) into 4L of water and add it to the vase. Then you add about 2L of water 2 times a week to the vase and once a month you instead add 2L of the same nutrient mix as before.

# How to get, build and run the Large and modular DIY kit
Under construction, will be added.

# The projects
1. [Small and modular DIY kit](https://github.com/rhagman/sustainable-green-plants/projects/1)
2. [Large and modular DIY kit](https://github.com/rhagman/sustainable-green-plants/projects/2)

# Software instructions
* arduino_sketch - contains the arduino sketch file which you need to upload to
  your arduino uno.

# License
MIT
