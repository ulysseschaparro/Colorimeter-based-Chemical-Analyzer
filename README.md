# CSE3442 Embedded Systems I Final Project

# Overview
This project integrates features of the Tiva™ TM4C123GH6PM Microcontroller to create a 
colorimeter-based chemical analyzer. The purpose of the chemical analyzer is to handle chemical 
samples of varying pH in turret-mounted test tubes and use colorimetry to measure the pH of any 
sample. The color of each sample tube will differ based on its pH level, thus allowing a RGB LED to shine through the samples and find the amount of refraction experienced with a phototransistor. The refraction values were then evaluated to find a formula that estimates a sample’s pH.

# Commands
The project supports the functionality of UART commands and IR remote commands. The commands are as follows: 

##home
Moves turret to reference position and stores home position

## turret TUBENUM
Rotates the turret to position 1, 2, 3, 4, 5, or R (reference)

## calibrate
Rotates the turret to reference position and calibrates the light path

## measure TUBENUM raw
Rotates the turret to position 1, 2, 3, 4, 5, or R (reference) and displays the raw RGB data values from the A/D converter

## measure TUBE pH | Cl
Rotates the turret to position 1, 2, 3, 4, 5, or R (reference) and displays the pH or chlorine values
