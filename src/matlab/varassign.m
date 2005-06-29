function [col,fcts2val]=varassign();
% defines cell array with variable names and columns and creates structures
% col and fcts2val with fields named after the variables

% cell array, 
% 1st column contains variable names;
% 2nd column reflects column assignments in ReadAvgData.c,
% 3rd column contains counts-to-value conversion funcions,
% 4th column contains value units. 
% edit 2nd column if assignments in ReadAvgData change 
% edit 3rd column if sensor calibrations change
statusDataCols=...
{'PMTThresh',7,'5*(bitand(x,4095))/4096','V';
'LaserTrigThresh',8,'5*(bitand(x,4095))/4096','V';
'MCP1Thresh',18,'5*(bitand(x,4095))/4096','V';
'MCP2Thresh',19,'5*(bitand(x,4095))/4096','V';
'V5VLift',11,'3*2.5*(bitand(x,4095))/4096','V';
'V28VLift',12,'16*2.5*(bitand(x,4095))/4096','V';
'V3_3VLift',13,'5*(bitand(x,4095))/4096','V';
'V1_5VLift',14,'2.5*(bitand(x,4095))/4096','V';
'V5VarmAxis',20,'3*2.5*(bitand(x,4095))/4096','V';
'V28VarmAxis',21,'16*2.5*(bitand(x,4095))/4096','V';
'V3_3VarmAxis',22,'5*(bitand(x,4095))/4096','V';
'V1_5VarmAxis',23,'2.5*(bitand(x,4095))/4096','V';
'ccMasterDelayLift',15,'12+x*4','ns';
'ccMasterDelayarmAxis',24,'12+x*4','ns';
'ccShiftDelay0',25,'12+x*4','ns';
'ccGateDelay0',26,'x*NaN','';
'ccGateWidth0',27,'x*NaN','';
'ccData0',28,'x*5','cps';
'ccMask0',220,'x*NaN','';
'ccCounts0',232,'x*5','cps';
'ccPulses0',233,'x*5','cps';
'ccShiftDelay1',234,'12+x*4','ns';
'ccGateDelay1',235,'16+x*4','ns';
'ccGateWidth1',236,'12+x*4','ns';
'ccData1',237,'x*5','cps';
'ccMask1',429,'x*NaN','';
'ccCounts1',441,'x*5','cps';
'ccPulses1',442,'x*5','cps';
'ccShiftDelay2',443,'12+x*4','ns';
'ccGateDelay2',444,'16+x*4','ns';
'ccGateWidth2',445,'12+x*4','ns';
'ccData2',446,'x*5','cps';
'ccMask2',638,'x*NaN','';
'ccCounts2',650,'x*5','cps';
'ccPulses2',651,'x*5','cps';
'etaSetPosLow',652,'x*NaN','';
'etaSetPosHigh',653,'x*NaN','';
'etaCurPosLow',654,'x*NaN','';
'etaCurPosHigh',655,'x*NaN','';
'etaEncoderPosLow',656,'x*NaN','';
'etaEncoderPosHigh',657,'x*NaN','';
'etaIndexPosLow',658,'x*NaN','';
'etaIndexPosHigh',659,'x*NaN','';
'etaSetSpd',660,'x','';
'etaSetAcclSpd',661,'x','';
'etaCurSpd',662,'x','';
'etaStatus',663,'x*NaN','';
'NumSamplesLiftADC1',664,'x*5','s';
'PDyelaser',665,'x*NaN','mbar';
'PDyelaserSumDat',666,'x*NaN','mbar';
'PDyelaserSumSqr',667,'x*NaN','mbar';
'PDyelaserSupplyV',668,'x*NaN','V';
'PDyelaserSupplyVSumDat',669,'x*NaN','V';
'PDyelaserSupplyVSumSqr',670,'x*NaN','V';
'DiodeUV','1,0184E-06*(x-10000)^2 + 4,0284E-03*(x-10000) - 6,5349E-02','mW';
'DiodeUVSumDat',672,'1,0184E-06*(x-10000)^2 + 4,0284E-03*(x-10000) - 6,5349E-02','mW';
'DiodeUVSumSqr',673,'1,0184E-06*(x-10000)^2 + 4,0284E-03*(x-10000) - 6,5349E-02','mW';
'PRef',674,'(x-10306)/(12720-10306)*998.7','mbar';
'PRefSumDat',675,'x*NaN','mbar';
'PRefSumSqr',676,'x*NaN','mbar';
'DiodeEtalon',677,'x*NaN','W';
'DiodeEtalonSumDat',678,'x*NaN','W';
'DiodeEtalonSumSqr',679,'x*NaN','W';
'DiodeGr',680,'(x-10000)/(11690-10000)*2.7','W';
'DiodeGrSumDat',681,'(x-10000)/(11690-10000)*2.7','W';
'DiodeGrSumSqr',682,'(x-10000)/(11690-10000)*2.7','W';
'PVent',683,'(x-9050)/(10497-9050)*998.7','mbar';
'PVentSumDat',684,'(x-9050)/(10497-9050)*998.7','mbar';
'PVentSumSqr',685,'(x-9050)/(10497-9050)*998.7','mbar';
'PVentSupplyV',686,'x*NaN','V';
'PVentSupplyVSumDat',687,'x*NaN','V';
'PVentSupplyVSumSqr',688,'x*NaN','V';
'PDyelaserConfig',689,'x*NaN','';
'PDyelaserSupplyVConfig',690,'x*NaN','';
'DiodeUVConfig',691,'x*NaN','';
'PRefConfig',692,'x*NaN','';
'DiodeEtalonConfig',693,'x*NaN','';
'DiodeGrConfig',694,'x*NaN','';
'PVentConfig',695,'x*NaN','';
'PVentSupplyVConfig',696,'x*NaN','';
'NumSamplesarmAxisADC2',866,'x*5','s';
'P1000',867,'x*NaN','mbar';
'P1000SumDat',868,'x*NaN','mbar';
'P1000SumSqr',869,'x*NaN','mbar';
'P20',870,'x*0.00891-89.55','mbar';
'P20SumDat',871,'x*0.00891-89.55','mbar';
'P20SumSqr',872,'x*0.00891-89.55','mbar';
'PNO',873,'(x-10130)/(10600-10130)*998.7','mbar';
'PNOSumDat',874,'(x-10130)/(10600-10130)*998.7';
'PNOSumSqr',875,'(x-10130)/(10600-10130)*998.7','mbar';
'VHV',876,'x*NaN','V';
'VHVSumDat',877,'x*NaN','V';
'VHVSumSqr',878,'x*NaN','V';
'TDet',879,'x','C';
'TDetSumDat',880,'x*NaN','C';
'TDetSumSqr',881,'x*NaN','C';
'DiodeWZ1out',882,'(x-9790.0)/193.2836','W';
'DiodeWZ1outSumDat',883,'(x-9790.0)/193.2836','W';
'DiodeWZ1outSumSqr',884,'(x-9790.0)/193.2836','W';
'DiodeWZ2out',885,'(x-9790.0)/193.2836','W';
'DiodeWZ2outSumDat',886,'(x-9790.0)/193.2836','W';
'DiodeWZ2outSumSqr',887,'(x-9790.0)/193.2836','W';
'DiodeWZ1in',888,'(x-9790.0)/193.2836','W';
'DiodeWZ1inSumDat',889,'(x-9790.0)/193.2836','W';
'DiodeWZ1inSumSqr',890,'(x-9790.0)/193.2836','W';
'DiodeWZ2in',840,'(x-9790.0)/193.2836','W';
'DiodeWZ2inSumDat',841,'(x-9790.0)/193.2836','W';
'DiodeWZ2inSumSqr',842,'(x-9790.0)/193.2836','W';
'PitotAbs',843,'x*NaN','mbar';
'PitotAbsSumDat',844,'x*NaN','mbar';
'PitotAbsSumSqr',845,'x*NaN','mbar';
'PitotDiff',846,'x*NaN','mbar';
'PitotDiffSumDat',847,'x*NaN','mbar';
'PitotDiffSumSqr',848,'x*NaN','mbar';
'IFilament',716,'x*NaN','A';
'IFilamentSumDat',717,'x*NaN','A';
'IFilamentSumSqr',718,'x*NaN','A';
'Ophir',719,'x','W';
'OphirSumDat',720,'x','W';
'OphirSumSqr',721,'x','W';
'P1000Config',891,'x*NaN','';
'P20Config',892,'x*NaN','';
'PNOConfig',893,'x*NaN','';
'VHVConfig',894,'x*NaN','';
'TDetSupplyVConfig',895,'x*NaN','';
'DiodeWZ1outConfig',896,'x*NaN','';
'DiodeWZ2outConfig',897,'x*NaN','';
'DiodeWZ1inConfig',898,'x*NaN','';
'DiodeWZ2inConfig',860,'x*NaN','';
'PitotAbsConfig',861,'x*NaN','';
'PitotDiffConfig',862,'x*NaN','';
'IFilamentConfig',728,'x*NaN','';
'OphirConfig',729,'x*NaN','';
'MFCSetFlow',730,'20*x/255','sccm';
'MFCFlow',731,'20*(x-9996)/(11447-9996)','sccm';
'ValveVoltLift',732,'x/140','V';
'ValveLift',733,'x*NaN','';
'Valve1VoltarmAxis',916,'x/140','V';
'Valve1armAxis',917,'x*NaN','';
'Valve2VoltarmAxis',918,'x/140','V';
'Valve2armAxis',919,'x*NaN','';
'DCDC4ch0SwitchV',734,'x/140','V';
'DCDC4ch1SwitchV',735,'x/140','V';
'DCDC4ch2SwitchV',736,'x/140','V';
'DCDC4ch3SwitchV',737,'x/140','V';
'TempMissedLift',738,'x*NaN','';
'TempNumberLift',739,'x*NaN','';
'TempErrCRCLift',740,'x*NaN','';
'TempNoResponseLift',741,'x*NaN','';
'Temp0L',742,'x/100-273.15','C';
'Temp0LStatus',743,'x*NaN','';
'Temp0Lid',744,'x*NaN','';
'Temp1L',745,'x/100-273.15','C';
'Temp1LStatus',746,'x*NaN','';
'Temp1Lid',747,'x*NaN','';
'Temp2L',748,'x/100-273.15','C';
'Temp2LStatus',749,'x*NaN','';
'Temp2Lid',750,'x*NaN','';
'Temp3L',751,'x/100-273.15','C';
'Temp3LStatus',752,'x*NaN','';
'Temp3Lid',753,'x*NaN','';
'Temp4L',754,'x/100-273.15','C';
'Temp4LStatus',755,'x*NaN','';
'Temp4Lid',756,'x*NaN','';
'Temp5L',757,'x/100-273.15','C';
'Temp5LStatus',758,'x*NaN','';
'Temp5Lid',759,'x*NaN','';
'Temp6L',760,'x/100-273.15','C';
'Temp6LStatus',761,'x*NaN','';
'Temp6Lid',762,'x*NaN','';
'Temp7L',763,'x/100-273.15','C';
'Temp7LStatus',764,'x*NaN','';
'Temp7Lid',765,'x*NaN','';
'Temp8L',766,'x/100-273.15','C';
'Temp8LStatus',767,'x*NaN','';
'Temp8Lid',768,'x*NaN','';
'Temp9L',769,'x/100-273.15','C';
'Temp9LStatus',770,'x*NaN','';
'Temp9Lid',771,'x*NaN','';
'Temp10L',772,'x/100-273.15','C';
'Temp10LStatus',773,'x*NaN','';
'Temp10Lid',774,'x*NaN','';
'Temp11L',775,'x/100-273.15','C';
'Temp11LStatus',776,'x*NaN','';
'Temp11Lid',777,'x*NaN','';
'Temp12L',778,'x/100-273.15','C';
'Temp12LStatus',779,'x*NaN','';
'Temp12Lid',780,'x*NaN','';
'Temp13L',781,'x/100-273.15','C';
'Temp13LStatus',782,'x*NaN','';
'Temp13Lid',783,'x*NaN','';
'Temp14L',784,'x/100-273.15','C';
'Temp14LStatus',785,'x*NaN','';
'Temp14Lid',786,'x*NaN','';
'Temp15L',787,'x/100-273.15','C';
'Temp15LStatus',788,'x*NaN','';
'Temp15Lid',789,'x*NaN','';
'Temp16L',790,'x/100-273.15','C';
'Temp16LStatus',791,'x*NaN','';
'Temp16Lid',792,'x*NaN','';
'Temp17L',793,'x/100-273.15','C';
'Temp17LStatus',794,'x*NaN','';
'Temp17Lid',795,'x*NaN','';
'Temp18L',796,'x/100-273.15','C';
'Temp18LStatus',797,'x*NaN','';
'Temp18Lid',798,'x*NaN','';
'Temp19L',799,'x/100-273.15','C';
'Temp19LStatus',800,'x*NaN','';
'Temp19Lid',801,'x*NaN','';
'Temp20L',802,'x/100-273.15','C';
'Temp20LStatus',803,'x*NaN','';
'Temp20Lid',804,'x*NaN','';
'Temp21L',805,'x/100-273.15','C';
'Temp21LStatus',806,'x*NaN','';
'Temp21Lid',807,'x*NaN','';
'Temp22L',808,'x/100-273.15','C';
'Temp22LStatus',809,'x*NaN','';
'Temp22Lid',810,'x*NaN','';
'TempMissedarmAxis',920,'x*NaN','';
'TempNumberarmAxis',921,'x*NaN','';
'TempErrCRCarmAxis',922,'x*NaN','';
'TempNoResponsearmAxis',923,'x*NaN','';
'Temp0WP',924,'x/100-273.15','C';
'Temp0WPStatus',925,'x*NaN','';
'Temp0WPid',926,'x*NaN','';
'Temp1WP',927,'x/100-273.15','C';
'Temp1WPStatus',928,'x*NaN','';
'Temp1WPid',929,'x*NaN','';
'Temp2WP',930,'x/100-273.15','C';
'Temp2WPStatus',931,'x*NaN','';
'Temp2WPid',932,'x*NaN','';
'Temp3WP',933,'x/100-273.15','C';
'Temp3WPStatus',934,'x*NaN','';
'Temp3WPid',935,'x*NaN','';
'Temp4WP',936,'x/100-273.15','C';
'Temp4WPStatus',937,'x*NaN','';
'Temp4WPid',938,'x*NaN','';
'Temp5WP',939,'x/100-273.15','C';
'Temp5WPStatus',940,'x*NaN','';
'Temp5WPid',941,'x*NaN','';
'Temp6WP',942,'x/100-273.15','C';
'Temp6WPStatus',943,'x*NaN','';
'Temp6WPid',944,'x*NaN','';
'Temp7WP',945,'x/100-273.15','C';
'Temp7WPStatus',946,'x*NaN','';
'Temp7WPid',947,'x*NaN','';
'Temp8WP',948,'x/100-273.15','C';
'Temp8WPStatus',949,'x*NaN','';
'Temp8WPid',950,'x*NaN','';
'Temp9WP',951,'x/100-273.15','C';
'Temp9WPStatus',952,'x*NaN','';
'Temp9WPid',953,'x*NaN','';
'Temp10WP',954,'x/100-273.15','C';
'Temp10WPStatus',955,'x*NaN','';
'Temp10WPid',956,'x*NaN','';
'Temp11WP',957,'x/100-273.15','C';
'Temp11WPStatus',958,'x*NaN','';
'Temp11WPid',959,'x*NaN','';
'Temp12WP',960,'x/100-273.15','C';
'Temp12WPStatus',961,'x*NaN','';
'Temp12WPid',962,'x*NaN','';
'Temp13WP',963,'x/100-273.15','C';
'Temp13WPStatus',964,'x*NaN','';
'Temp13WPid',965,'x*NaN','';
'Temp14WP',966,'x/100-273.15','C';
'Temp14WPStatus',967,'x*NaN','';
'Temp14WPid',968,'x*NaN','';
'Temp15WP',969,'x/100-273.15','C';
'Temp15WPStatus',970,'x*NaN','';
'Temp15WPid',971,'x*NaN','';
'Temp16WP',972,'x/100-273.15','C';
'Temp16WPStatus',973,'x*NaN','';
'Temp16WPid',974,'x*NaN','';
'Temp17WP',975,'x/100-273.15','C';
'Temp17WPStatus',976,'x*NaN','';
'Temp17WPid',977,'x*NaN','';
'Temp18WP',978,'x/100-273.15','C';
'Temp18WPStatus',979,'x*NaN','';
'Temp18WPid',980,'x*NaN','';
'Temp19WP',981,'x/100-273.15','C';
'Temp19WPStatus',982,'x*NaN','';
'Temp19WPid',983,'x*NaN','';
'Temp20WP',984,'x/100-273.15','C';
'Temp20WPStatus',985,'x*NaN','';
'Temp20WPid',986,'x*NaN','';
'Temp21WP',987,'x/100-273.15','C';
'Temp21WPStatus',988,'x*NaN','';
'Temp21WPid',989,'x*NaN','';
'Temp22WP',990,'x/100-273.15','C';
'Temp22WPStatus',991,'x*NaN','';
'Temp22WPid',992,'x*NaN','';
'EtalonAction',811,'x*NaN','';
'etaOnlinePosLow',812,'x*NaN','';
'etaOnlinePosHigh',813,'x*NaN','';
'RAvgOnOffFlag',814,'x*NaN','';
'InstrumentAction',815,'x*NaN','';
'GPSsecondsUTC',816,'x','s';
'GPSLongitude',817,'x/60-180','deg';
'GPSLongitudeDecimals',818,'x/10000','min';
'GPSLatitude',819,'x/60-90','deg';
'GPSLatitudeDecimals',820,'x/10000','min';
'GPSAltitude',821,'x','m';
'GPSHDOP',822,'x','';
'GPSnumSat',823,'x','';
'GPSLastValidData',824,'x*5','s';
'GPSGroundSpeed',825,'x/100','m/s';
'GPSHeading',826,'x/10','deg'
'ValidSlaveDataFlag',1004,'x',''};

% create structure 'col' with field names taken from 1st column of
% statusDataCols and values from 2nd column
col=cell2struct(statusDataCols(:,2),statusDataCols(:,1),1);

% create structure 'fcts2val' with field names taken from 1st column of
% statusDataCols and values from 3rd column
fcts2val=cell2struct(statusDataCols(:,3),statusDataCols(:,1),1);

