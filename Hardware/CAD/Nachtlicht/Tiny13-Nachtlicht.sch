EESchema Schematic File Version 4
LIBS:Tiny13-Nachtlicht-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Nachtlicht"
Date "2019-11-22"
Rev "1.0"
Comp "DARC OV \"Pentacon Dresden\""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L MCU_Microchip_ATtiny:ATtiny13A-SSU U3
U 1 1 5DBCBFF2
P 4850 3600
F 0 "U3" H 4320 3646 50  0000 R CNN
F 1 "ATtiny13A-SSU" H 4320 3555 50  0000 R CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 4850 3600 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/doc8126.pdf" H 4850 3600 50  0001 C CNN
	1    4850 3600
	1    0    0    -1  
$EndComp
$Comp
L Device:C C1
U 1 1 5DBCC0AA
P 4850 2450
F 0 "C1" H 4735 2404 50  0000 R CNN
F 1 "100 nF" H 4735 2495 50  0000 R CNN
F 2 "Capacitors_SMD:C_0805_HandSoldering" H 4888 2300 50  0001 C CNN
F 3 "~" H 4850 2450 50  0001 C CNN
	1    4850 2450
	-1   0    0    1   
$EndComp
$Comp
L Device:C C2
U 1 1 5DBCC1DF
P 6200 3600
F 0 "C2" H 6315 3646 50  0000 L CNN
F 1 "100 nF" V 6350 3250 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805_HandSoldering" H 6238 3450 50  0001 C CNN
F 3 "~" H 6200 3600 50  0001 C CNN
	1    6200 3600
	1    0    0    -1  
$EndComp
$Comp
L Device:D_Photo D1
U 1 1 5DBCC2B5
P 6600 3650
F 0 "D1" V 6350 3700 50  0000 L CNN
F 1 "PD333-3C/H0/L2" H 6400 3900 50  0000 L CNN
F 2 "LEDs:LED-5MM" H 6550 3650 50  0001 C CNN
F 3 "~" H 6550 3650 50  0001 C CNN
	1    6600 3650
	0    1    1    0   
$EndComp
$Comp
L Device:LED D2
U 1 1 5DBCC355
P 7350 3600
F 0 "D2" V 7388 3483 50  0000 R CNN
F 1 "LED" V 7297 3483 50  0000 R CNN
F 2 "LEDs:LED-5MM" H 7350 3600 50  0001 C CNN
F 3 "~" H 7350 3600 50  0001 C CNN
	1    7350 3600
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR02
U 1 1 5DBCC481
P 4850 4300
F 0 "#PWR02" H 4850 4050 50  0001 C CNN
F 1 "GND" H 4855 4127 50  0000 C CNN
F 2 "" H 4850 4300 50  0001 C CNN
F 3 "" H 4850 4300 50  0001 C CNN
	1    4850 4300
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR03
U 1 1 5DBCC4E9
P 6200 3900
F 0 "#PWR03" H 6200 3650 50  0001 C CNN
F 1 "GND" H 6205 3727 50  0000 C CNN
F 2 "" H 6200 3900 50  0001 C CNN
F 3 "" H 6200 3900 50  0001 C CNN
	1    6200 3900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR04
U 1 1 5DBCC505
P 6600 3900
F 0 "#PWR04" H 6600 3650 50  0001 C CNN
F 1 "GND" H 6605 3727 50  0000 C CNN
F 2 "" H 6600 3900 50  0001 C CNN
F 3 "" H 6600 3900 50  0001 C CNN
	1    6600 3900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR05
U 1 1 5DBCC51A
P 7350 3900
F 0 "#PWR05" H 7350 3650 50  0001 C CNN
F 1 "GND" H 7355 3727 50  0000 C CNN
F 2 "" H 7350 3900 50  0001 C CNN
F 3 "" H 7350 3900 50  0001 C CNN
	1    7350 3900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR01
U 1 1 5DBCC52F
P 4850 2050
F 0 "#PWR01" H 4850 1800 50  0001 C CNN
F 1 "GND" H 4855 1877 50  0000 C CNN
F 2 "" H 4850 2050 50  0001 C CNN
F 3 "" H 4850 2050 50  0001 C CNN
	1    4850 2050
	-1   0    0    1   
$EndComp
Wire Wire Line
	4850 2050 4850 2300
Wire Wire Line
	6200 3750 6200 3900
Wire Wire Line
	6600 3750 6600 3900
Wire Wire Line
	7350 3750 7350 3900
Wire Wire Line
	5450 3400 5900 3400
Wire Wire Line
	6600 3400 6600 3450
Wire Wire Line
	6200 3450 6200 3400
Connection ~ 6200 3400
Wire Wire Line
	6200 3400 6600 3400
Wire Wire Line
	7350 3300 7350 3450
$Comp
L Device:R R1
U 1 1 5DBD8D80
P 7000 3300
F 0 "R1" V 6793 3300 50  0000 C CNN
F 1 "27 Ω" V 6884 3300 50  0000 C CNN
F 2 "Resistors_SMD:R_0805_HandSoldering" V 6930 3300 50  0001 C CNN
F 3 "~" H 7000 3300 50  0001 C CNN
	1    7000 3300
	0    1    1    0   
$EndComp
Wire Wire Line
	5450 3300 5800 3300
Wire Wire Line
	7150 3300 7350 3300
Text Label 4850 2800 0    50   ~ 0
Vcc
Wire Wire Line
	4850 4200 4850 4250
Wire Wire Line
	3700 4250 4850 4250
Connection ~ 4850 4250
Wire Wire Line
	4850 4250 4850 4300
Wire Wire Line
	3700 2800 4850 2800
Wire Wire Line
	4850 2600 4850 2800
Connection ~ 4850 2800
Wire Wire Line
	4850 2800 4850 3000
$Comp
L Battery:2xAA B1
U 1 1 5DBEA2E0
P 3500 3500
F 0 "B1" H 3639 3925 50  0000 C CNN
F 1 "2xAA" H 3639 3834 50  0000 C CNN
F 2 "Battery:2xAA-standing" H 3500 3500 50  0001 C CNN
F 3 "" H 3500 3500 50  0001 C CNN
	1    3500 3500
	-1   0    0    -1  
$EndComp
Wire Wire Line
	3700 2800 3700 3300
Wire Wire Line
	3700 3300 3500 3300
Wire Wire Line
	3500 3700 3700 3700
Wire Wire Line
	3700 3700 3700 4250
$Comp
L Connector:AVR-ISP-6 J1
U 1 1 5DBF5223
P 5300 5300
F 0 "J1" H 5020 5396 50  0000 R CNN
F 1 "AVR-ISP-6" H 5020 5305 50  0000 R CNN
F 2 "Pin_Headers:Pin_Header_Straight_2x03" V 5050 5350 50  0001 C CNN
F 3 " ~" H 4025 4750 50  0001 C CNN
	1    5300 5300
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x03_Male J2
U 1 1 5DBF52E2
P 6400 2700
F 0 "J2" H 6372 2630 50  0000 R CNN
F 1 "Pin 1x3" H 6372 2721 50  0000 R CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x03" H 6400 2700 50  0001 C CNN
F 3 "~" H 6400 2700 50  0001 C CNN
	1    6400 2700
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR06
U 1 1 5DBF5427
P 5200 5800
F 0 "#PWR06" H 5200 5550 50  0001 C CNN
F 1 "GND" H 5205 5627 50  0000 C CNN
F 2 "" H 5200 5800 50  0001 C CNN
F 3 "" H 5200 5800 50  0001 C CNN
	1    5200 5800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR07
U 1 1 5DBF5444
P 6100 2700
F 0 "#PWR07" H 6100 2450 50  0001 C CNN
F 1 "GND" V 6105 2572 50  0000 R CNN
F 2 "" H 6100 2700 50  0001 C CNN
F 3 "" H 6100 2700 50  0001 C CNN
	1    6100 2700
	0    1    1    0   
$EndComp
Wire Wire Line
	4850 2800 5550 2800
Wire Wire Line
	5550 2800 5550 4500
Wire Wire Line
	5550 4500 5200 4500
Wire Wire Line
	5200 4500 5200 4800
Wire Wire Line
	5200 5700 5200 5800
Wire Wire Line
	5700 5200 5800 5200
Wire Wire Line
	5800 5200 5800 3300
Connection ~ 5800 3300
Wire Wire Line
	5800 3300 6850 3300
Wire Wire Line
	5700 5100 5900 5100
Wire Wire Line
	5900 5100 5900 3400
Connection ~ 5900 3400
Wire Wire Line
	5900 3400 6200 3400
Wire Wire Line
	5700 5300 6000 5300
Wire Wire Line
	6000 5300 6000 3500
Wire Wire Line
	6000 3500 5450 3500
Wire Wire Line
	5450 3800 6100 3800
Wire Wire Line
	6100 3800 6100 5400
Wire Wire Line
	6100 5400 5700 5400
Wire Wire Line
	5450 3600 5600 3600
Wire Wire Line
	5600 3600 5600 2600
Wire Wire Line
	5600 2600 6200 2600
Wire Wire Line
	6200 2800 5700 2800
Wire Wire Line
	5700 2800 5700 3700
Wire Wire Line
	5700 3700 5450 3700
Wire Wire Line
	6100 2700 6200 2700
Text Notes 6500 2900 0    50   ~ 0
Jumper
Text Notes 5400 5700 0    50   ~ 0
(Steckverbinder nicht bestückt.)
$EndSCHEMATC
