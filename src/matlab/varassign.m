function [col,fcts2val]=varassign(Data);
% defines cell array with variable names and columns and creates structures
% col and fcts2val with fields named after the variables

% cell array, 
% 1st column contains variable names;
% 2nd column reflects column assignments in ReadAvgData.c,
% 3rd column contains counts-to-value conversion funcions,
% 4th column contains value units. 
% edit 2nd column if assignments in ReadAvgData change 
% edit 3rd column if sensor calibrations change

colTemp0L=742;
for i=0:39
    x=median(uint16(Data(:,colTemp0L+3*i+2))); 
    eval(['Tempid(',num2str(i+1),',:)=','dec2hex(x);']);
    if strcmp('6B92',Tempid(i+1,:)) colTempLaserPlate=colTemp0L+3*i;
    elseif strcmp('3F36',Tempid(i+1,:)) colTempDyelaser=colTemp0L+3*i;
    elseif strcmp('506D',Tempid(i+1,:)) colTempRefCell=colTemp0L+3*i;
    elseif strcmp('3C93',Tempid(i+1,:)) colTempDiodeEt=colTemp0L+3*i;
    elseif strcmp('656B',Tempid(i+1,:)) colTemp4L=colTemp0L+3*i;
    elseif strcmp('4227',Tempid(i+1,:)) colTempDiodeUV=colTemp0L+3*i;
    elseif strcmp('3D47',Tempid(i+1,:)) colTempSensCardLift=colTemp0L+3*i;
    elseif strcmp('403F',Tempid(i+1,:)) colTempCPULift=colTemp0L+3*i;
    elseif strcmp('614B',Tempid(i+1,:)) colTempDiodeGr=colTemp0L+3*i;
    else colTempLUnknown=colTemp0L+3*i;
    end
end
clear Tempid;

colTemp0WP=991;
for i=0:39
    x=median(uint16(Data(:,colTemp0WP+3*i+2))); 
    eval(['Tempid(',num2str(i+1),',:)=','dec2hex(x);']);
    if strcmp('3E04',Tempid(i+1,:)) colTempKuvette=colTemp0WP+3*i;
    elseif strcmp('4DD7',Tempid(i+1,:)) colTempPenray=colTemp0WP+3*i;
    elseif strcmp('57DA',Tempid(i+1,:)) colTempAxisPlate=colTemp0WP+3*i;
    elseif strcmp('649F',Tempid(i+1,:)) colTempHV=colTemp0WP+3*i;
    elseif strcmp('4803',Tempid(i+1,:)) colTempInverter=colTemp0WP+3*i;
    elseif strcmp('5E92',Tempid(i+1,:)) colTempPumpOil=colTemp0WP+3*i;
    elseif strcmp('B2C4',Tempid(i+1,:)) colTempMCP1=colTemp0WP+3*i;
    elseif strcmp('3C81',Tempid(i+1,:)) colTempMCP2=colTemp0WP+3*i;
    elseif strcmp('60F8',Tempid(i+1,:)) colTempPreamp1=colTemp0WP+3*i;
    elseif strcmp('499C',Tempid(i+1,:)) colTempPrallpl=colTemp0WP+3*i;
    elseif strcmp('4AEB',Tempid(i+1,:)) colTempBlower=colTemp0WP+3*i;
    elseif strcmp('60A3',Tempid(i+1,:)) colTempArmBackWall=colTemp0WP+3*i;
    elseif strcmp('3E76',Tempid(i+1,:)) colTempAxis=colTemp0WP+3*i;
    elseif strcmp('473D',Tempid(i+1,:)) colTempLasersync=colTemp0WP+3*i;
    elseif strcmp('56CE',Tempid(i+1,:)) colTempArmPS=colTemp0WP+3*i;
    elseif strcmp('5970',Tempid(i+1,:)) colTempPump=colTemp0WP+3*i;
    elseif strcmp('602A',Tempid(i+1,:)) colTempWZ1out=colTemp0WP+3*i;
    elseif strcmp('5BA7',Tempid(i+1,:)) colTempCalPlate=colTemp0WP+3*i;
    elseif strcmp('87F9',Tempid(i+1,:)) colTempWZ1in=colTemp0WP+3*i;
    elseif strcmp('5AC5',Tempid(i+1,:)) colTempWZ2in=colTemp0WP+3*i;
    elseif strcmp('A2EF',Tempid(i+1,:)) colTempSensCard=colTemp0WP+3*i;
    elseif strcmp('63EB',Tempid(i+1,:)) colTempWZ2out=colTemp0WP+3*i;
    elseif strcmp('157B',Tempid(i+1,:)) colTempScrollMotor=colTemp0WP+3*i;
    elseif strcmp('28CD',Tempid(i+1,:)) colTempScrollContr=colTemp0WP+3*i;
    else colTempWPUnknown=colTemp0WP+3*i;
    end
end

if exist('colTempLaserPlate')==0 colTempLaserPlate=NaN; end
if exist('colTempDyelaser')==0 colTempDyelaser=NaN; end
if exist('colTempRefCell')==0 colTempRefCell=NaN; end
if exist('colTempDiodeEt')==0 colTempDiodeEt=NaN; end
if exist('colTemp4L')==0 colTemp4L=NaN; end
if exist('colTempDiodeUV')==0 colTempDiodeUV=NaN; end
if exist('colTempSensCardLift')==0 colTempSensCardLift=NaN; end
if exist('colTempCPULift')==0 colTempCPULift=NaN; end
if exist('colTempDiodeGr')==0 colTempDiodeGr=NaN; end
if exist('colTempLUnknown')==0 colTempLUnknown=NaN; end

if exist('colTempKuvette')==0 colTempKuvette=NaN; end
if exist('colTempPenray')==0 colTempPenray=NaN; end
if exist('colTempAxisPlate')==0 colTempAxisPlate=NaN; end
if exist('colTempHV')==0 colTempHV=NaN; end
if exist('colTempInverter')==0 colTempInverter=NaN; end
if exist('colTempPumpOil')==0 colTempPumpOil=NaN; end
if exist('colTempMCP1')==0 colTempMCP1=NaN; end
if exist('colTempMCP2')==0 colTempMCP2=NaN; end
if exist('colTempPreamp1')==0 colTempPreamp1=NaN; end
if exist('colTempPrallpl')==0 colTempPrallpl=NaN; end
if exist('colTempBlower')==0 colTempBlower=NaN; end
if exist('colTempArmBackWall')==0 colTempArmBackWall=NaN; end
if exist('colTempAxis')==0 colTempAxis=NaN; end
if exist('colTempLasersync')==0 colTempLasersync=NaN; end
if exist('colTempArmPS')==0 colTempArmPS=NaN; end
if exist('colTempPump')==0 colTempPump=NaN; end
if exist('colTempWZ1out')==0 colTempWZ1out=NaN; end
if exist('colTempCalPlate')==0 colTempCalPlate=NaN; end
if exist('colTempWZ1in')==0 colTempWZ1in=NaN; end
if exist('colTempWZ2in')==0 colTempWZ2in=NaN; end
if exist('colTempSensCard')==0 colTempSensCard=NaN; end
if exist('colTempWZ2out')==0 colTempWZ2out=NaN; end
if exist('colTempScrollMotor')==0 colTempScrollMotor=NaN; end
if exist('colTempScrollContr')==0 colTempScrollContr=NaN; end
if exist('colTempWPUnknown')==0 colTempLUnknown=NaN; end

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
'PDyelaser',665,'(x-9070)/(11463-9070)*991.3','mbar';
'PDyelaserSumDat',666,'(x-9070)/(11463-9070)*991.3','mbar';
'PDyelaserSumSqr',667,'(x-9070)/(11463-9070)*991.3','mbar';
'PDyelaserSupplyV',668,'x*NaN','V';
'PDyelaserSupplyVSumDat',669,'x*NaN','V';
'PDyelaserSupplyVSumSqr',670,'x*NaN','V';
'DiodeUV',671,'16.4/(11390-10000)*(x-10000)','mW';
'DiodeUVSumDat',672,'16.4/(11390-10000)*(x-10000)','mW';
'DiodeUVSumSqr',673,'16.4/(11390-10000)*(x-10000)','mW';
'PRef',674,'(x-10320)/(12720-10320)*998.7','mbar';
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
'NumSamplesarmAxisADC2',917,'x*5','s';
'P1000',918,'0.44588*(x-9996.6)','mbar';
'P1000SumDat',919,'0.44588*(x-9996.6)','mbar';
'P1000SumSqr',920,'0.44588*(x-9996.6)','mbar';
'P20',921,'0.013345*(x-10012)','mbar';
'P20SumDat',922,'0.013345*(x-10012)','mbar';
'P20SumSqr',923,'0.013345*(x-10012)','mbar';
'PNO',924,'(x-10130)/(10600-10130)*998.7','mbar';
'PNOSumDat',925,'(x-10130)/(10600-10130)*998.7','mbar';
'PNOSumSqr',926,'(x-10130)/(10600-10130)*998.7','mbar';
'VHV',927,'(x-10000)*13.28205/3030','V';
'VHVSumDat',928,'x*NaN','V';
'VHVSumSqr',929,'x*NaN','V';
%Gabriel, HOOVER1'TDet',930,'1./(3.3539646E-3+2.565409E-4.*log(680./3000.*(5.*1750./(x-10000)-1))+1.9243889E-6.*(log(680./3000.*(5.*1750./(x-10000)-1))).^2+1.0969244E-7.*(log(680./3000.*(5.*1750./(x-10000)-1))).^3)-273.15','C';
'TDet',930,'1./(3.3540154E-3+2.5627725E-4.*log(220000./3000./(4.094.*1550./(x-9999)-1))+2.082921E-6.*(log(220000./3000./(4.094.*1550./(x-9999)-1))).^2+7.30003206E-8.*(log(220000./3000./(4.094.*1550./(x-9999)-1))).^3)-273.15','C';
'TDetSumDat',931,'x*NaN','C';
'TDetSumSqr',932,'x*NaN','C';
'DiodeWZ1out',933,'(x-10007.0)./(10650-10007).*6.1','mW';
'DiodeWZ1outSumDat',934,'x*NaN','mW';
'DiodeWZ1outSumSqr',935,'x*NaN','mW';
'DiodeWZ2out',936,'(x-10003.0)./(10164-10003).*1.05','mW';
'DiodeWZ2outSumDat',937,'x*NaN','mW';
'DiodeWZ2outSumSqr',938,'x*NaN','mW';
'DiodeWZ1in',939,'(x-10004.0)./(10925-10004).*7.2','mW';
'DiodeWZ1inSumDat',940,'x*NaN','mW';
'DiodeWZ1inSumSqr',941,'x*NaN','mW';
'DiodeWZ2in',891,'(x-10000.0)./(10145-10000).*1.3','mW';
'DiodeWZ2inSumDat',892,'x*NaN','mW';
'DiodeWZ2inSumSqr',893,'x*NaN','mW';
'PitotAbs',897,'(x-10000-3030/5).*1500./3030-15.8','mbar';
'PitotAbsSumDat',898,'(x-10000-3030/5).*1500./3030','mbar';
'PitotAbsSumSqr',899,'(x-10000-3030/5).*1500./3030','mbar';
'PitotDiff',900,'(x-10000)*5/3030','mbar';
'PitotDiffSumDat',901,'(x-10000)*5/3030','mbar';
'PitotDiffSumSqr',902,'(x-10000)*5/3030','mbar';
'IFilament',716,'x*NaN','A';
'IFilamentSumDat',717,'x*NaN','A';
'IFilamentSumSqr',718,'x*NaN','A';
'Ophir',719,'x','W';
'OphirSumDat',720,'x','W';
'OphirSumSqr',721,'x','W';
'P1000Config',942,'x*NaN','';
'P20Config',943,'x*NaN','';
'PNOConfig',944,'x*NaN','';
'VHVConfig',945,'x*NaN','';
'TDetSupplyVConfig',946,'x*NaN','';
'DiodeWZ1outConfig',947,'x*NaN','';
'DiodeWZ2outConfig',948,'x*NaN','';
'DiodeWZ1inConfig',949,'x*NaN','';
'DiodeWZ2inConfig',911,'x*NaN','';
'PitotAbsConfig',913,'x*NaN','';
'PitotDiffConfig',914,'x*NaN','';
'IFilamentConfig',728,'x*NaN','';
'OphirConfig',729,'x*NaN','';
'MFCSetFlow',730,'20*x/255','sccm';
'MFCFlow',731,'20*(x-10010)/(11461-10010)','sccm';
'ValveVoltLift',732,'x/140','V';
'ValveLift',733,'x*NaN','';
'Valve1VoltarmAxis',983,'x/140','V';
'Valve1armAxis',984,'x*NaN','';
'Valve2VoltarmAxis',985,'x/140','V';
'Valve2armAxis',986,'x*NaN','';
'DCDC4ch0SwitchV',734,'x/140','V';
'DCDC4ch1SwitchV',735,'x/140','V';
'DCDC4ch2SwitchV',736,'x/140','V';
'DCDC4ch3SwitchV',737,'x/140','V';
'TempMissedLift',738,'x','';
'TempNumberLift',739,'x','';
'TempErrCRCLift',740,'x','';
'TempNoResponseLift',741,'x','';
'TempLaserPlate',colTempLaserPlate,'x/100-273.15','C';
'TempLaserPlateStatus',colTempLaserPlate+1,'x*NaN','';
'TempLaserPlateid',colTempLaserPlate+2,'dec2hex(x)','';
'TempDyelaser',colTempDyelaser,'x/100-273.15','C';
'TempDyelaserStatus',colTempDyelaser+1,'x*NaN','';
'TempDyelaserid',colTempDyelaser+2,'dec2hex(x)','';
'TempRefCell',colTempRefCell,'x/100-273.15','C';
'TempRefCellStatus',colTempRefCell+1,'x*NaN','';
'TempRefCellid',colTempRefCell+2,'dec2hex(x)','';
'TempDiodeEt',colTempDiodeEt,'x/100-273.15','C';
'TempDiodeEtStatus',colTempDiodeEt+1,'x*NaN','';
'TempDiodeEtid',colTempDiodeEt+2,'dec2hex(x)','';
'Temp4L',colTemp4L,'x/100-273.15','C';
'Temp4LStatus',colTemp4L+1,'x*NaN','';
'Temp4Lid',colTemp4L+2,'dec2hex(x)','';
'TempDiodeUV',colTempDiodeUV,'x/100-273.15','C';
'TempDiodeUVStatus',colTempDiodeUV+1,'x*NaN','';
'TempDiodeUVid',colTempDiodeUV+2,'dec2hex(x)','';
'TempSensCardLift',colTempSensCardLift,'x/100-273.15','C';
'TempSensCardLiftStatus',colTempSensCardLift+1,'x*NaN','';
'TempSensCardLiftid',colTempSensCardLift+2,'dec2hex(x)','';
'TempCPULift',colTempCPULift,'x/100-273.15','C';
'TempCPULiftStatus',colTempCPULift+1,'x*NaN','';
'TempCPULiftid',colTempCPULift+2,'dec2hex(x)','';
'TempDiodeGr',colTempDiodeGr,'x/100-273.15','C';
'TempDiodeGrStatus',colTempDiodeGr+1,'x*NaN','';
'TempDiodeGrid',colTempDiodeGr+2,'dec2hex(x)','';
'TempLUnknown',colTempLUnknown,'x/100-273.15','C';
'TempLUnknownStatus',colTempLUnknown+1,'x*NaN','';
'TempLUnknownid',colTempLUnknown+2,'dec2hex(x)','';
'TempMissedarmAxis',987,'x*NaN','';
'TempNumberarmAxis',988,'dec2hex(x)','';
'TempErrCRCarmAxis',989,'x*NaN','';
'TempNoResponsearmAxis',990,'dec2hex(x)','';
'TempPump',colTempPump,'x/100-273.15','C';
'TempPumpStatus',colTempPump+1,'x*NaN','';
'TempPumpid',colTempPump+2,'dec2hex(x)','';
'TempPumpOil',colTempPumpOil,'x/100-273.15','C';
'TempPumpOilStatus',colTempPumpOil+1,'x*NaN','';
'TempPumpOilid',colTempPumpOil+2,'dec2hex(x)','';
'TempDiodeWZ1out',colTempWZ1out,'x/100-273.15','C';
'TempDiodeWZ1outStatus',colTempWZ1out+1,'x*NaN','';
'TempDiodeWZ1outid',colTempWZ1out+2,'dec2hex(x)','';
'TempAxisPlate',colTempAxisPlate,'x/100-273.15','C';
'TempAxisPlateStatus',colTempAxisPlate+1,'x*NaN','';
'TempAxisPlateid',colTempAxisPlate+2,'dec2hex(x)','';
'TempArmPS',colTempArmPS,'x/100-273.15','C';
'TempArmPSStatus',colTempArmPS+1,'x*NaN','';
'TempArmPSid',colTempArmPS+2,'dec2hex(x)','';
'TempDiodeWZ1in',colTempWZ1in,'x/100-273.15','C';
'TempDiodeWZ1inStatus',colTempWZ1in+1,'x*NaN','';
'TempDiodeWZ1inid',colTempWZ1in+2,'dec2hex(x)','';
'TempDiodeWZ2in',colTempWZ2in,'x/100-273.15','C';
'TempDiodeWZ2inStatus',colTempWZ2in+1,'x*NaN','';
'TempDiodeWZ2inid',colTempWZ2in+2,'dec2hex(x)','';
'TempTMPSensCard',colTempSensCard,'x/100-273.15','C';
'TempTMPSensCardStatus',colTempSensCard+1,'x*NaN','';
'TempTMPSensCardid',colTempSensCard+2,'dec2hex(x)','';
'TempLaserSync',colTempLasersync,'x/100-273.15','C';
'TempLaserSyncStatus',colTempLasersync+1,'x*NaN','';
'TempLaserSyncid',colTempLasersync+2,'dec2hex(x)','';
'TempInverter',colTempInverter,'x/100-273.15','C';
'TempInverterStatus',colTempInverter+1,'x*NaN','';
'TempInverterid',colTempInverter+2,'dec2hex(x)','';
'TempArmBackWall',colTempArmBackWall,'x/100-273.15','C';
'TempArmBackWallStatus',colTempArmBackWall+1,'x*NaN','';
'TempArmBackWallid',colTempArmBackWall+2,'dec2hex(x)','';
'TempBlower',colTempBlower,'x/100-273.15','C';
'TempBlowerStatus',colTempBlower+1,'x*NaN','';
'TempBlowerid',colTempBlower+2,'dec2hex(x)','';
'TempDiodeWZ2out',colTempWZ2out,'x/100-273.15','C';
'TempDiodeWZ2outStatus',colTempWZ2out+1,'x*NaN','';
'TempDiodeWZ2outid',colTempWZ2out+2,'dec2hex(x)','';
'TempCalPlate',colTempCalPlate,'x/100-273.15','C';
'TempCalPlateStatus',colTempCalPlate+1,'x*NaN','';
'TempCalPlateid',colTempCalPlate+2,'dec2hex(x)','';
'TempHV',colTempHV,'x/100-273.15','C';
'TempHVStatus',colTempHV+1,'x*NaN','';
'TempHVid',colTempHV+2,'dec2hex(x)','';
'TempKuvette',colTempKuvette,'x/100-273.15','C';
'TempKuvetteStatus',colTempKuvette+1,'x*NaN','';
'TempKuvetteid',colTempKuvette+2,'dec2hex(x)','';
'TempPenray',colTempPenray,'x/100-273.15','C';
'TempPenrayStatus',colTempPenray+1,'x*NaN','';
'TempPenrayid',colTempPenray+2,'dec2hex(x)','';
'TempMCP1',colTempMCP1,'x/100-273.15','C';
'TempMCP1Status',colTempMCP1+1,'x*NaN','';
'TempMCP1id',colTempMCP1+2,'dec2hex(x)','';
'TempMCP2',colTempMCP2,'x/100-273.15','C';
'TempMCP2Status',colTempMCP2+1,'x*NaN','';
'TempMCP2id',colTempMCP2+2,'dec2hex(x)','';
'TempPreamp1',colTempPreamp1,'x/100-273.15','C';
'TempPreamp1Status',colTempPreamp1+1,'x*NaN','';
'TempPreamp1id',colTempPreamp1+2,'dec2hex(x)','';
'TempPrallpl',colTempPrallpl,'x/100-273.15','C';
'TempPrallplStatus',colTempPrallpl+1,'x*NaN','';
'TempPrallplid',colTempPrallpl+2,'dec2hex(x)','';
'TempAxis',colTempAxis,'x/100-273.15','C';
'TempAxisStatus',colTempAxis+1,'x*NaN','';
'TempAxisid',colTempAxis+2,'x*NaN','';
'TempScrollMotor',colTempScrollMotor,'x/100-273.15','C';
'TempScrollMotorStatus',colTempScrollMotor+1,'x*NaN','';
'TempScrollMotorid',colTempScrollMotor+2,'x*NaN','';
'TempScrollContr',colTempScrollContr,'x/100-273.15','C';
'TempScrollContrStatus',colTempScrollContr+1,'x*NaN','';
'TempScrollContrid',colTempScrollContr+2,'x*NaN','';
'TempWPUnknown',colTempWPUnknown,'x/100-273.15','C';
'TempWPUnknownStatus',colTempWPUnknown+1,'x*NaN','';
'TempWPUnknownid',colTempWPUnknown+2,'dec2hex(x)','';
'EtalonAction',862,'x*NaN','';
'etaOnlinePosLow',863,'x*NaN','';
'etaOnlinePosHigh',864,'x*NaN','';
'RAvgOnOffFlag',865,'x*NaN','';
'InstrumentAction',866,'x*NaN','';
'GPSsecondsUTC',867,'x','s';
'GPSLongitude',868,'x/60-180','deg';
'GPSLongitudeDecimals',869,'x/10000','min';
'GPSLatitude',870,'x/60-90','deg';
'GPSLatitudeDecimals',871,'x/10000','min';
'GPSAltitude',872,'x*3.28084','ft';
'GPSHDOP',873,'x','';
'GPSnumSat',874,'x','';
'GPSLastValidData',875,'x*5','s';
'GPSGroundSpeed',876,'x/100','m/s';
'GPSHeading',877,'x/10','deg';
'ValidSlaveDataFlag',1122,'x','';
'PhototubeLamp1',951,'x*NaN','';
'PhototubeLamp2',954,'x*NaN','';
'PCuvette',957,'(x-10485).*5.12264e-4+1','bar';
'PCuvetteSupply',960,'x*NaN','';
'ButterflyPositionValid',1129,'x','';
'ButterflyCurrentPosition',1130,'x','';
'MirrorGr1XAxisLo',1135,'x','';
'MirrorGr1XAxisHi',1136,'x','';
'MirrorGr1YAxisLo',1137,'x','';
'MirrorGr1YAxisHi',1138,'x','';
'MirrorGr2XAxisLo',1139,'x','';
'MirrorGr2XAxisHi',1140,'x','';
'MirrorGr2YAxisLo',1141,'x','';
'MirrorGr2YAxisHi',1142,'x','';
'MirrorUV1XAxisLo',1143,'x','';
'MirrorUV1XAxisHi',1144,'x','';
'MirrorUV1YAxisLo',1145,'x','';
'MirrorUV1YAxisHi',1146,'x','';
'MirrorUV2XAxisLo',1147,'x','';
'MirrorUV2XAxisHi',1148,'x','';
'MirrorUV2YAxisLo',1149,'x','';
'MirrorUV2YAxisHi',1150,'x','';
'MirrorRealigning',1151,'x','';
'MirrorMovingFlags',1152,'x','';
'MirrorMinUVDiffCts',1153,'x','';
'MirrorRealignMinutes',1154,'x',''
'I28V',972,'(x-10000)./3030.*80','A';
};

% create structure 'col' with field names taken from 1st column of
% statusDataCols and values from 2nd column
col=cell2struct(statusDataCols(:,2),statusDataCols(:,1),1);

% create structure 'fcts2val' with field names taken from 1st column of
% statusDataCols and values from 3rd column
fcts2val=cell2struct(statusDataCols(:,3),statusDataCols(:,1),1);

