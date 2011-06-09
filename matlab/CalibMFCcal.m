% script to calculate the calibrator coefficience
% not working yet (haven't even started) HH

MFCCalOffset = 2.107 ;
MFCCalSlope  = 4.1e-3;

MFCSetFlow   = 500;
MFCSetCounts = 25;
DryCalRead   =   10;  % flow measured with Drycal

MFCCalRead = 4000; % counts from MFC

MFCSetCounts = (MFCSetFlow - MFCCalOffset) * MFCCalSlope;

