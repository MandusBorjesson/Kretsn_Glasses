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

- The images follow a naming pattern, which determines how they are displayed on the glasses, IMG[index]_[type]_[period]_[lEye]_[rEye].png.
	- index: Image index, values between 0 anf 7 are permitted and higher indexes are ignored. The program will load AT MOST one image with each index, ignoring duplicates.
	- type: Image display type, FRAME or SWEEP. FRAME displays the image one "frame" (eight rows) at the time. SWEEP sweeps over the image smoothly
	- period: time period between frame updates, in milliseconds. Values between 0 and 25000 are permitted. 
	- lEye: Left eye color setting, if "R", "G" or "B" are present, the respective color will be on. Use "O" for all OFF.
	- rEye: Same as Leye, but for right eye. 

# Example file folder
File folders may be stored anywhere on the computer, and can look something like this:

Test
 |_ IMG0_FRAME_200_R_R.png	(Frame image with both eyes red, updating every 200 ms)
 |_ IMG0_SWEEP_200.png		(Duplicate index, will be ignored)
 |_ IMG1_SWEEP_100_G.png	(Sweeping image, both eyes green)
 |_ IMG2_FRAME_1000_RGB_B.png	(White/Blue eyes)
 |_ IMG3.png			(Frame, 100 ms updates, eyes off)
 |_ IMG4_FRAME_100_O_RG.png	(Left eye off, right eye yellow)
 |_ IMG6_FRAME_1500.png		(both eyes off)
 |_ IMG22.png			(Index larger than 7, will be ignored)
