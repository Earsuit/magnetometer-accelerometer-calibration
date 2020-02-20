# Acceleration and Magnetometer calibration
Here is an example for how to calibrate the acceleration and magnetometer in MPU9250. 

The main.cpp is code written for STM32f108, but it should work with Arduino as I use the Arduino framework. 

The collect_data.py is for collecting data from the Nano, to make it work, you need to change the ser.port(line 38) to the USB port you used to connect to the Nano. It write data to a .csv file. 

The EllipsoidFitting.m is a Matlab code for ellipsoid fitting.

If you have any questions, please contact me :)

