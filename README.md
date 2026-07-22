# Stratum 
Stratum is a 9-key personal macropad creation with a rotary encoder, an OLED display and RGB backlighting under each key that uses QMK firmware. It supporteds five independent layers, allowing each key to perform a unique fucntion depending on the active layer. Stratum was made as a submission for Hackpad v5 through HackClub's Stardance Challenge.

## Features:
- 0.91 inch OLED display
- EC11 Rotary encoders (push button diabled)
- 9 MX-Style switches with DSA keycaps
- 9 accompnaying SK6812 MINI-E LEDs for backlighting

## PCB
This is the PCB I made from Stratum; made in KiCAd 10.0. Most of the footprints were provided by HackClub. Anything else was from KICad's default libraries or was sourced through peers on the HackClub Slack. The sikscreen uses HackClub's logo and a character from the Stardance Challenge.

**Schematic:**

**PCB:**

## CAD Model
This CAD model utilizes an integrated plate keyboard mount style. The rotary encoder cap was provided by HackClub.

Right now, the case utilizes self-threading, as I knew I would not be unscrewing the macropad much after assembly. Realistically, I will need to add heatset inserts to ensure its long-term survival.

## Firmware Overview
The name Stratum reflects this macropad's ability to swap between 5, distict layers through QMK firmware. This is done through a Settings mode in which you must hold down three center keys until the OLED displays a selection menu where you can use the rotary encoder to scroll through the 5 different layers. You select with the lower-right key. An overview of even more cool firmware features are:

- 5 Different Layers: Main, Productivity, Design, Scientific and Media. The Keys and rotary encoder change functions on each mode
- The OLED that displays current layer, settings menu and a transition animation
- RGB Backlights change color on each layer
- A sleep and brightness control in settings

The 5 layers will likely be updated as I use the macropad and see what does and does not work. A future idea I definely have in mind is adding VIA support for quick macro fucntionality edits.

##BOM:
All the components needed for this macropad design:
- 9x Cherry MX Switches
- 9x DSA Keycaps
- 4x M3x16mm screws
- 9x through-hole 1N4148 Diodes
- 9x SK6812 MINI-E LEDs (reverse mount)
- 1x 0.91 inch OLED display (128x32)
- 1x EC11 Rotary encoder
- 1x XIAO RP2040
- 1x Case (2 printed parts + optional Rotary Encoder Cap)
  





![Screenshot of overall macropad] (assets/placeholder.png)
