function [varnames,col,fcts2val]=varassign();
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
{'PMTThresh',7,'5*x/4096','V';
'LaserTrigThresh',8,'5*x/4096','V';
'MCP1Thresh',9,'5*x/4096','V';
'MCP2Thresh',10,'5*x/4096','V';
'V5V',11,'3*2.5*x/4096','V';
'V28V',12,'16*2.5*x/4096','V';
'V3_3V',13,'5*x/4096','V';
'V1_5V',14,'2.5*x/4096','V';
'ccMasterDelay',15,'12+x*4','ns';
'ccShiftDelay0',16,'12+x*4','ns';
'ccGateDelay0',17,'x*NaN','';
'ccGateWidth0',18,'x*NaN','';
'ccData0',19,'x*5','cps';
'ccMask0',211,'x*NaN','';
'ccCounts0',223,'x*5','cps';
'ccPulses0',224,'x*5','cps';
'ccShiftDelay1',225,'12+x*4','ns';
'ccGateDelay1',226,'16+x*4','ns';
'ccGateWidth1',227,'12+x*4','ns';
'ccData1',228,'x*5','cps';
'ccMask1',420,'x*NaN','';
'ccCounts1',432,'x*5','cps';
'ccPulses1',433,'x*5','cps';
'ccShiftDelay2',434,'12+x*4','ns';
'ccGateDelay2',435,'16+x*4','ns';
'ccGateWidth2',436,'12+x*4','ns';
'ccData2',437,'x*5','cps';
'ccMask2',629,'x*NaN','';
'ccCounts2',641,'x*5','cps';
'ccPulses2',642,'x*5','cps';
'etaSetPosLow',643,'x*NaN','';
'etaSetPosHigh',644,'x*NaN','';
'etaCurPosLow',645,'x*NaN','';
'etaCurPosHigh',646,'x*NaN','';
'etaEncoderPosLow',647,'x*NaN','';
'etaEncoderPosHigh',648,'x*NaN','';
'etaIndexPosLow',649,'x*NaN','';
'etaIndexPosHigh',650,'x*NaN','';
'etaSetSpd',651,'x','';
'etaSetAcclSpd',652,'x','';
'etaCurSpd',653,'x','';
'etaStatus',654,'x*NaN','';
'NumSamplesADC1',655,'x*5','s';
'PDyelaser',656,'x*NaN','mbar';
'PDyelaserSumDat',657,'x*NaN','mbar';
'PDyelaserSumSqr',658,'x*NaN','mbar';
'PDyelaserSupplyV',659,'x*NaN','V';
'PDyelaserSupplyVSumDat',660,'x*NaN','V';
'PDyelaserSupplyVSumSqr',661,'x*NaN','V';
'DiodeUV',662,'4.6863E-6*x^2-8.5857E-2*x+390.41','W';
'DiodeUVSumDat',663,'4.6863E-6*x^2-8.5857E-2*x+390.41','W';
'DiodeUVSumSqr',664,'4.6863E-6*x^2-8.5857E-2*x+390.41','W';
'PRef',665,'x*NaN','mbar';
'PRefSumDat',666,'x*NaN','mbar';
'PRefSumSqr',667,'x*NaN','mbar';
'DiodeEtalon',668,'x*NaN','W';
'DiodeEtalonSumDat',669,'x*NaN','W';
'DiodeEtalonSumSqr',670,'x*NaN','W';
'DiodeGr',671,'x*NaN','W';
'DiodeGrSumDat',672,'x*NaN','W';
'DiodeGrSumSqr',673,'x*NaN','W';
'PVent',674,'x*NaN','mbar';
'PVentSumDat',675,'x*NaN','mbar';
'PVentSumSqr',676,'x*NaN','mbar';
'PVentSupplyV',677,'x*NaN','V';
'PVentSupplyVSumDat',678,'x*NaN','V';
'PVentSupplyVSumSqr',679,'x*NaN','V';
'PDyelaserConfig',680,'x*NaN','';
'PDyelaserSupplyVConfig',681,'x*NaN','';
'DiodeUVConfig',682,'x*NaN','';
'PRefConfig',683,'x*NaN','';
'DiodeEtalonConfig',684,'x*NaN','';
'DiodeGrConfig',685,'x*NaN','';
'PVentConfig',686,'x*NaN','';
'PVentSupplyVConfig',687,'x*NaN','';
'NumSamplesADC2',688,'x*5','s';
'P1000',689,'x*NaN','mbar';
'P1000SumDat',690,'x*NaN','mbar';
'P1000SumSqr',691,'x*NaN','mbar';
'P20',692,'x*0.00891-89.55','mbar';
'P20SumDat',693,'x*0.00891-89.55','mbar';
'P20SumSqr',694,'x*0.00891-89.55','mbar';
'PNO',695,'x*NaN','mbar';
'PNOSumDat',696,'x*NaN','mbar';
'PNOSumSqr',697,'x*NaN','mbar';
'VHV',698,'x*NaN','V';
'VHVSumDat',699,'x*NaN','V';
'VHVSumSqr',700,'x*NaN','V';
'TDet',701,'x*NaN','C';
'TDetSumDat',702,'x*NaN','C';
'TDetSumSqr',703,'x*NaN','C';
'DiodeWZout',704,'(x-9790.0)/193.2836','W';
'DiodeWZoutSumDat',705,'(x-9790.0)/193.2836','W';
'DiodeWZoutSumSqr',706,'(x-9790.0)/193.2836','W';
'IFilament',707,'x*NaN','A';
'IFilamentSumDat',708,'x*NaN','A';
'IFilamentSumSqr',709,'x*NaN','A';
'Ophir',710,'x','W';
'OphirSumDat',711,'x','W';
'OphirSumSqr',712,'x','W';
'P1000Config',713,'x*NaN','';
'P20Config',714,'x*NaN','';
'PNOConfig',715,'x*NaN','';
'VHVConfig',716,'x*NaN','';
'TDetSupplyVConfig',717,'x*NaN','';
'DiodeWZoutConfig',718,'x*NaN','';
'IFilamentConfig',719,'x*NaN','';
'OphirConfig',720,'x*NaN','';
'MFCSetFlow',721,'x*NaN','';
'MFCFlow',722,'x*NaN','sccm';
'ValveVolt',723,'x/140','V';
'Valve',724,'x*NaN','';
'HVSwitchV',725,'x/140','V';
'ButterflySwitchV',726,'x/140','V';
'BlowerSwitchV',727,'x/140','V';
'DCDC4ch3SwitchV',728,'x/140','V';
'TempMissed',729,'x*NaN','';
'TempNumber',730,'x*NaN','';
'TempErrCRC',731,'x*NaN','';
'TempNoResponse',732,'x*NaN','';
'Temp0',733,'x/100-273.15','C';
'Temp0Status',734,'x*NaN','';
'Temp0ID',735,'x*NaN','';
'TDetaxis',736,'x/100-273.15','C';
'TDetaxisStatus',737,'x*NaN','';
'TDetaxisID',738,'x*NaN','';
'Temp2',739,'x/100-273.15','C';
'Temp2Status',740,'x*NaN','';
'Temp2ID',741,'x*NaN','';
'Temp3',742,'x/100-273.15','C';
'Temp3Status',743,'x*NaN','';
'Temp3ID',744,'x*NaN','';
'Temp4',745,'x/100-273.15','C';
'Temp4Status',746,'x*NaN','';
'Temp4ID',747,'x*NaN','';
'Temp5',748,'x/100-273.15','C';
'Temp5Status',749,'x*NaN','';
'Temp5ID',750,'x*NaN','';
'Temp6',751,'x/100-273.15','C';
'Temp6Status',752,'x*NaN','';
'Temp6ID',753,'x*NaN','';
'Temp7',754,'x/100-273.15','C';
'Temp7Status',755,'x*NaN','';
'Temp7ID',756,'x*NaN','';
'Temp8',757,'x/100-273.15','C';
'Temp8Status',758,'x*NaN','';
'Temp8ID',759,'x*NaN','';
'Temp9',760,'x/100-273.15','C';
'Temp9Status',761,'x*NaN','';
'Temp9ID',762,'x*NaN','';
'Temp10',763,'x/100-273.15','C';
'Temp10Status',764,'x*NaN','';
'Temp10ID',765,'x*NaN','';
'Temp11',766,'x/100-273.15','C';
'Temp11Status',767,'x*NaN','';
'Temp11ID',768,'x*NaN','';
'Temp12',769,'x/100-273.15','C';
'Temp12Status',770,'x*NaN','';
'Temp12ID',771,'x*NaN','';
'Temp13',772,'x/100-273.15','C';
'Temp13Status',773,'x*NaN','';
'Temp13ID',774,'x*NaN','';
'Temp14',775,'x/100-273.15','C';
'Temp14Status',776,'x*NaN','';
'Temp14ID',777,'x*NaN','';
'Temp15',778,'x/100-273.15','C';
'Temp15Status',779,'x*NaN','';
'Temp15ID',780,'x*NaN','';
'Temp16',781,'x/100-273.15','C';
'Temp16Status',782,'x*NaN','';
'Temp16ID',783,'x*NaN','';
'Temp17',784,'x/100-273.15','C';
'Temp17Status',785,'x*NaN','';
'Temp17ID',786,'x*NaN','';
'Temp18',787,'x/100-273.15','C';
'Temp18Status',788,'x*NaN','';
'Temp18ID',789,'x*NaN','';
'Temp19',790,'x/100-273.15','C';
'Temp19Status',791,'x*NaN','';
'Temp19ID',792,'x*NaN','';
'Temp20',793,'x/100-273.15','C';
'Temp20Status',794,'x*NaN','';
'Temp20ID',795,'x*NaN','';
'Temp21',796,'x/100-273.15','C';
'Temp21Status',797,'x*NaN','';
'Temp21ID',798,'x*NaN','';
'Temp22',799,'x/100-273.15','C';
'Temp22Status',800,'x*NaN','';
'Temp22ID',801,'x*NaN','';
'EtalonAction',802,'x*NaN','';
'etaOnlinePosLow',803,'x*NaN','';
'etaOnlinePosHigh',804,'x*NaN','';
'RAvgOnOffFlag',805,'x*NaN','';
'InstrumentAction',806,'x*NaN','';
'GPSsecondsUTC',807,'x','s';
'GPSLongitude',808,'x/60-180','deg';
'GPSLongitudeDecimals',809,'x/10000','min';
'GPSLatitude',810,'x/60-90','deg';
'GPSLatitudeDecimals',811,'x/10000','min';
'GPSAltitude',812,'x','m';
'GPSHDOP',813,'x','';
'GPSnumSat',814,'x','';
'GPSLastValidData',815,'x*5','s';
'GPSGroundSpeed',816,'x/100','m/s';
'GPSHeading',817,'x/10','deg'};

% create structure 'col' with field names taken from 1st column of
% statusDataCols and values from 2nd column
col=cell2struct(statusDataCols(:,2),statusDataCols(:,1),1);

% create structure 'fcts2val' with field names taken from 1st column of
% statusDataCols and values from 3rd column
fcts2val=cell2struct(statusDataCols(:,3),statusDataCols(:,1),1);

