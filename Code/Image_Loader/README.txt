# Using the Image Loader
Recommended program for creating images: <https://www.piskelapp.com/>

# Actions 

p: changes serial port used for communication.

l: Load files. Opens a file browser where the user is prompted to select a folder with images. More about the folder later. 
Note: This is when the images are imported and converted, any changes made to the images after loading them in the program will not be sent to the glasses. The user can reload the folder to include any changes. 

space: Send data over serial port. Sends the converted data to the glasses as well as the image descriptors used by the glasses. 

# Image folders and images 
The program loads .png files from a folder, the images follow a set of rules:

- The program will look at the eight leftmost and rigthmost pixels of each low in the image, any other pixels are ignored and can be used freely.

- Any pixel that is not completely transparent is interpreted as a turned on LED on the glasses.

- The images follow a naming pattern, which determines how they are displayed on the glasses: "IMGx_arg1_arg2..._argN.png", where x is image index.

Arguments switches, use as many as you like:
	- _M : Mode switch, "1" displays the image one "frame" (eight rows) at the time. "2" sweeps over the image
	- _P: time period between frame updates, in milliseconds. Values between 0 and 25000 are permitted. 
	- _L/R: Left/Right eye color setting, see table below.
	- _S[1-9]/R: Number of loops. Animations loop [1-9] times (or a random amount when using _SR) and then halt at last frame
	- _C[N+]: Chain command. Use alongside _S switch. When animation finishes looping it jumps to animation N. Can use a single multiple N, i.e _C62, in this case the animation will jump to one of the two options (2 or 6, random) when finished. 

Eye color settings:
 0: Eye off
 1: Red
 2: Green
 3: Yellow
 4: Blue
 5: Purple
 6: Cyan
 7: White

# Example file folder
File folders may be stored anywhere on the computer, and can look something like this:

Test
 |_ IMG0_M1_P200_L1_R1.png	(Frame image with both eyes red, updating every 200 ms)
 |_ IMG0_M2_P200.png		(Duplicate index, will be ignored)
 |_ IMG1_M2_P100.png		(Sweeping image)
 |_ IMG2_M1_P1000_L7_R4.png	(White/Blue eyes)
 |_ IMG3.png			(Frame, 100 ms updates, eyes off)
 |_ IMG4_M1_P100_L0_R3.png	(Left eye off, right eye yellow)
 |_ IMG6_M1_P1500.png		(both eyes off)
 |_ IMG22.png			(Index larger than 7, will be ignored)
