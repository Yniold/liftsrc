function varargout = Sensors(varargin)
% SENSORS M-file for Sensors.fig
%      SENSORS, by itself, creates a new SENSORS or raises the existing
%      singleton*.
%
%      H = SENSORS returns the handle to a new SENSORS or the handle to
%      the existing singleton*.
%
%      SENSORS('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in SENSORS.M with the given input arguments.
%
%      SENSORS('Property','Value',...) creates a new SENSORS or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before Sensors_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to Sensors_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Copyright 2002-2003 The MathWorks, Inc.

% Edit the above text to modify the response to help Sensors

% Last Modified by GUIDE v2.5 03-Jun-2005 10:13:30

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @Sensors_OpeningFcn, ...
                   'gui_OutputFcn',  @Sensors_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before Sensors is made visible.
function Sensors_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to Sensors (see VARARGIN)

% Choose default command line output for Sensors
handles.output = hObject;

% get horus handle
if length(varargin)==2 & varargin{1}=='handle'
    handles.parenthandle=str2double(varargin{2});
end

%setup Timer function
handles.Timer = timer('ExecutionMode','fixedDelay',...
      'Period',0.7,...    
      'BusyMode','drop',...
      'TimerFcn', {@SensRefresh,handles});   

data.Timer=handles.Timer;

% Update handles structure
guidata(hObject, handles);
setappdata(handles.output, 'Sensdata', data);

start(handles.Timer);


function SensRefresh(arg1,arg2,handles)

horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
AvgData=horusdata.AvgData;
col=horusdata.col;
fcts2val=horusdata.fcts2val;

% Calculate time as sum of day, hour, min, etc.
statustime=double(statusData(:,2))./1.0+ ...
           double(statusData(:,3))./24.0+...
           double(statusData(:,4))./1440.0+...
           double(statusData(:,5))./86400.0+...
           double(statusData(:,6))./86400000.0;

[SortZeit,indexZeit]=sort(statustime);
maxLen=size(statustime,1);
lastrow=indexZeit(maxLen);
       
% get parameter selected in listbox X
xvars=get(handles.listboxX,'String');
xvalue=get(handles.listboxX,'Value');
xpar=xvars(xvalue);

% get parameter selected in listbox Y
yvars=get(handles.listboxY,'String');
yvalue=get(handles.listboxY,'Value');
ypar=yvars(yvalue);

% assign data to x-axis
switch char(xpar)
    case 'Time'
        xdata=statustime;
    case 'DiodeGr'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.DiodeGr);
        else
            x=double(statusData(:,col.DiodeGr)); eval(['xdata=',fcts2val.DiodeGr,';']);
        end
    case 'DiodeUV'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.DiodeUV);
        else
            x=double(statusData(:,col.DiodeUV)); eval(['xdata=',fcts2val.DiodeUV,';']);
        end
    case 'DiodeEtalon'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.DiodeEtalon);
        else
            x=double(statusData(:,col.DiodeEtalon)); eval(['xdata=',fcts2val.DiodeEtalon,';']);
        end
    case 'DiodeWZ1in'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.DiodeWZ1in);
        else
            x=double(statusData(:,col.DiodeWZ1in)); eval(['xdata=',fcts2val.DiodeWZ1in,';']);
        end
    case 'DiodeWZ1out'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.DiodeWZ1out);
        else
            x=double(statusData(:,col.DiodeWZ1out)); eval(['xdata=',fcts2val.DiodeWZ1out,';']);
        end
    case 'DiodeWZ2in'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.DiodeWZ2in);
        else
            x=double(statusData(:,col.DiodeWZ2in)); eval(['xdata=',fcts2val.DiodeWZ2in,';']);
        end
    case 'DiodeWZ2out'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.DiodeWZ2out);
        else
            x=double(statusData(:,col.DiodeWZ2out)); eval(['xdata=',fcts2val.DiodeWZ2out,';']);
        end
    case 'Ophir'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Ophir);
        else
            x=double(statusData(:,col.Ophir)); eval(['xdata=',fcts2val.Ophir,';']);
        end
    case 'P20'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.P20);
        else
            x=double(statusData(:,col.P20)); eval(['xdata=',fcts2val.P20,';']);
        end
    case 'P1000'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.P1000);
        else
            x=double(statusData(:,col.P1000)); eval(['xdata=',fcts2val.P1000,';']);
        end
    case 'PRef'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.PRef);
        else
            x=double(statusData(:,col.PRef)); eval(['xdata=',fcts2val.PRef,';']);
        end
    case 'PDyelaser'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.PDyelaser);
        else
            x=double(statusData(:,col.PDyelaser)); eval(['xdata=',fcts2val.PDyelaser,';']);
        end
    case 'PVent'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.PVent);
        else
            x=double(statusData(:,col.PVent)); eval(['xdata=',fcts2val.PVent,';']);
        end
    case 'PNO'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.PNO);
        else
            x=double(statusData(:,col.PNO)); eval(['xdata=',fcts2val.PNO,';']);
        end
    case 'PCuvette'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.PCuvette);
        else
            x=double(statusData(:,col.PCuvette)); eval(['xdata=',fcts2val.PCuvette,';']);
        end
    case 'IFilament'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.IFilament);
        else
            x=double(statusData(:,col.IFilament)); eval(['xdata=',fcts2val.IFilament,';']);
        end
    case 'VHV'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.VHV);
        else
            x=double(statusData(:,col.VHV)); eval(['xdata=',fcts2val.VHV,';']);
        end
    case 'LaserTrigThresh'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.LaserTrigThresh);
        else
            x=double(statusData(:,col.LaserTrigThresh)); eval(['xdata=',fcts2val.LaserTrigThresh,';']);
        end
    case 'PMTThresh'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.PMTThresh);
        else
            x=double(statusData(:,col.PMTThresh)); eval(['xdata=',fcts2val.PMTThresh,';']);
        end
    case 'MCP1Thresh'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.MCP1Thresh);
        else
            x=double(statusData(:,col.MCP1Thresh)); eval(['xdata=',fcts2val.MCP1Thresh,';']);
        end
    case 'MCP2Thresh'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.MCP2Thresh);
        else
            x=double(statusData(:,col.MCP2Thresh)); eval(['xdata=',fcts2val.MCP2Thresh,';']);
        end
    case '+5VLift'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.V5VLift);
        else
            x=double(statusData(:,col.V5VLift)); eval(['xdata=',fcts2val.V5VLift,';']);
        end
    case '+28VLift'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.V28VLift);
        else
            x=double(statusData(:,col.V28VLift)); eval(['xdata=',fcts2val.V28VLift,';']);
        end
    case '+3.3VLift'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.V3_3VLift);
        else
            x=double(statusData(:,col.V3_3VLift)); eval(['xdata=',fcts2val.V3_3VLift,';']);
        end
    case '+1.5VLift'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.V1_5VLift);
        else
            x=double(statusData(:,col.V1_5VLift)); eval(['xdata=',fcts2val.V1_5VLift,';']);
        end
    case '+5VarmAxis'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.V5VarmAxis);
        else
            x=double(statusData(:,col.V5VarmAxis)); eval(['xdata=',fcts2val.V5VarmAxis,';']);
        end
    case '+28VarmAxis'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.V28VarmAxis);
        else
            x=double(statusData(:,col.V28VarmAxis)); eval(['xdata=',fcts2val.V28VarmAxis,';']);
        end
    case '+3.3VarmAxis'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.V3_3VarmAxis);
        else
            x=double(statusData(:,col.V3_3VarmAxis)); eval(['xdata=',fcts2val.V3_3VarmAxis,';']);
        end
    case '+1.5VarmAxis'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.V1_5VarmAxis);
        else
            x=double(statusData(:,col.V1_5VarmAxis)); eval(['xdata=',fcts2val.V1_5VarmAxis,';']);
        end
    case 'I28V'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.I28V);
        else
            x=double(statusData(:,col.I28V)); eval(['xdata=',fcts2val.I28V,';']);
        end
    case 'EtaSpd'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.etaCurSpd);
        else
            x=double(statusData(:,col.etaCurSpd)); eval(['xdata=',fcts2val.etaCurSpd,';']);
        end
    case 'EtaSetPos'
        Etalonhelp=bitget(uint16(statusData(:,col.etaSetPosHigh)),16);
        EtalonSetPos=double(statusData(:,col.etaSetPosHigh)).*65536+double(statusData(:,col.etaSetPosLow));
        EtalonSetPos(Etalonhelp==1)=bitset(floor(EtalonSetPos(Etalonhelp==1)),32,0)-2^32/2;
        xdata=EtalonSetPos;
    case 'EtaCurPos'
        Etalonhelp=bitget(uint16(statusData(:,col.etaCurPosHigh)),16);
        EtalonCurPos=double(statusData(:,col.etaCurPosHigh)).*65536+double(statusData(:,col.etaCurPosLow));
        EtalonCurPos(Etalonhelp==1)=bitset(floor(EtalonCurPos(Etalonhelp==1)),32,0)-2^32/2;
        xdata=EtalonCurPos;
    case 'EtaEncPos'
        Etalonhelp=bitget(uint16(statusData(:,col.etaEncoderPosHigh)),16);
        EtalonEncPos=double(statusData(:,col.etaEncoderPosHigh)).*65536+double(statusData(:,col.etaEncoderPosLow));
        EtalonEncPos(Etalonhelp==1)=bitset(floor(EtalonEncPos(Etalonhelp==1)),32,0)-2^32/2;
        xdata=EtalonEncPos;
    case 'EtaIndPos'
        Etalonhelp=bitget(uint16(statusData(:,col.etaIndexPosHigh)),16);
        EtalonIndPos=double(statusData(:,col.etaIndexPosHigh)).*65536+double(statusData(:,col.etaIndexPosLow));
        EtalonIndPos(Etalonhelp==1)=bitset(floor(EtalonIndPos(Etalonhelp==1)),32,0)-2^32/2;
        xdata=EtalonIndPos;
    case 'EtaOnlinePos'
        Etalonhelp=bitget(uint16(statusData(:,col.etaOnlinePosHigh)),16);
        OnlinePos=double(statusData(:,col.etaOnlinePosHigh)).*65536+double(statusData(:,col.etaOnlinePosLow));
        OnlinePos(Etalonhelp==1)=bitset(floor(OnlinePos(Etalonhelp==1)),32,0)-2^32/2;
        xdata=OnlinePos;
    case 'ValveVoltLift'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ValveVoltLift);
        else
            x=double(statusData(:,col.ValveVoltLift)); eval(['xdata=',fcts2val.ValveVoltLift,';']);
        end
    case 'Valve1VoltarmAxis'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Valve1VoltarmAxis);
        else
            x=double(statusData(:,col.Valve1VoltarmAxis)); eval(['xdata=',fcts2val.Valve1VoltarmAxis,';']);
        end
    case 'Valve2VoltarmAxis'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Valve2VoltarmAxis);
        else
            x=double(statusData(:,col.Valve2VoltarmAxis)); eval(['xdata=',fcts2val.Valve2VoltarmAxis,';']);
        end
    case 'MFCFlow'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.MFCFlow);
        else
            x=double(statusData(:,col.MFCFlow)); eval(['xdata=',fcts2val.MFCFlow,';']);
        end
    case 'MFCSetFlow'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.MFCSetFlow);
        else
            x=double(statusData(:,col.MFCSetFlow)); eval(['xdata=',fcts2val.MFCSetFlow,';']);
        end
    case 'PitotAbs'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.PitotAbs);
        else
            x=double(statusData(:,col.PitotAbs)); eval(['xdata=',fcts2val.PitotAbs,';']);
        end
    case 'PitotDiff'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.PitotDiff);
        else
            x=double(statusData(:,col.PitotDiff)); eval(['xdata=',fcts2val.PitotDiff,';']);
        end
    case 'PhototubeLamp1'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.PhototubeLamp1);
        else
            x=double(statusData(:,col.PhototubeLamp1)); eval(['xdata=',fcts2val.PhototubeLamp1,';']);
        end
    case 'PhototubeLamp2'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.PhototubeLamp2);
        else
            x=double(statusData(:,col.PhototubeLamp2)); eval(['xdata=',fcts2val.PhototubeLamp2,';']);
        end
    case 'ccMasterDelayLift'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ccMasterDelayLift);
        else
            x=double(statusData(:,col.ccMasterDelayLift)); eval(['xdata=',fcts2val.ccMasterDelayLift,';']);
        end
    case 'ccMasterDelayarmAxis'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ccMasterDelayarmAxis);
        else
            x=double(statusData(:,col.ccMasterDelayarmAxis)); eval(['xdata=',fcts2val.ccMasterDelayarmAxis,';']);
        end
    case 'ccShiftDelay0'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ccShiftDelay0);
        else
            x=double(statusData(:,col.ccShiftDelay0)); eval(['xdata=',fcts2val.ccShiftDelay0,';']);
        end
    case 'ccCounts0'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ccCounts0);
        else
            x=double(statusData(:,col.ccCounts0)); eval(['xdata=',fcts2val.ccCounts0,';']);
        end
    case 'ccPulses0'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ccPulses0);
        else
            x=double(statusData(:,col.ccPulses0)); eval(['xdata=',fcts2val.ccPulses0,';']);
        end
    case 'ccShiftDelay1'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ccShiftDelay1);
        else
            x=double(statusData(:,col.ccShiftDelay1)); eval(['xdata=',fcts2val.ccShiftDelay1,';']);
        end
    case 'ccGateDelay1'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ccGateDelay1);
        else
            x=double(statusData(:,col.ccGateDelay1)); eval(['xdata=',fcts2val.ccGateDelay1,';']);
        end
    case 'ccGateWidth1'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ccGateWidth1);
        else
            x=double(statusData(:,col.ccGateWidth1)); eval(['xdata=',fcts2val.ccGateWidth1,';']);
        end
    case 'ccCounts1'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ccCounts1);
        else
            x=double(statusData(:,col.ccCounts1)); eval(['xdata=',fcts2val.ccCounts1,';']);
        end
    case 'ccPulses1'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ccPulses1);
        else
            x=double(statusData(:,col.ccPulses1)); eval(['xdata=',fcts2val.ccPulses1,';']);
        end
    case 'ccShiftDelay2'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ccShiftDelay2);
        else
            x=double(statusData(:,col.ccShiftDelay2)); eval(['xdata=',fcts2val.ccShiftDelay2,';']);
        end
    case 'ccGateDelay2'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ccGateDelay2);
        else
            x=double(statusData(:,col.ccGateDelay2)); eval(['xdata=',fcts2val.ccGateDelay2,';']);
        end
    case 'ccGateWidth2'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ccGateWidth2);
        else
            x=double(statusData(:,col.ccGateWidth2)); eval(['xdata=',fcts2val.ccGateWidth2,';']);
        end
    case 'ccCounts2'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ccCounts2);
        else
            x=double(statusData(:,col.ccCounts2)); eval(['xdata=',fcts2val.ccCounts2,';']);
        end
    case 'ccPulses2'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ccPulses2);
        else
            x=double(statusData(:,col.ccPulses2)); eval(['xdata=',fcts2val.ccPulses2,';']);
        end
    case 'GPSMsecondsUTC'
        xdata=statusData(:,col.GPSMsecondsUTC);
    case 'GPSMLongitude'
        xdata=double(statusData(:,col.GPSMLongitude))/60-180 ...
            +double(statusData(:,col.GPSMLongitudeDecimals))/10000;
    case 'GPSMLatitude'
        xdata=double(statusData(:,col.GPSMLatitude))/60-90 ...
            +double(statusData(:,col.GPSMLatitudeDecimals))/10000;
    case 'GPSMAltitude'
        xdata=statusData(:,col.GPSMAltitude);
    case 'GPSMHDOP'
        xdata=statusData(:,col.GPSMHDOP);
    case 'GPSMnumSat'
        xdata=statusData(:,col.GPSMnumSat);
    case 'GPSMLastValidData'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.GPSMLastValidData);
        else
            x=double(statusData(:,col.GPSMLastValidData)); eval(['xdata=',fcts2val.GPSMLastValidData,';']);
        end
    case 'GPSMGroundSpeed'
        xdata=double(statusData(:,col.GPSMGroundSpeed))/100;
    case 'GPSMHeading'
        xdata=double(statusData(:,col.GPSMHeading))/10;
    case 'GPSSsecondsUTC'
        xdata=statusData(:,col.GPSSsecondsUTC);
    case 'GPSSLongitude'
        xdata=double(statusData(:,col.GPSSLongitude))/60-180 ...
            +double(statusData(:,col.GPSSLongitudeDecimals))/10000;
    case 'GPSSLatitude'
        xdata=double(statusData(:,col.GPSSLatitude))/60-90 ...
            +double(statusData(:,col.GPSSLatitudeDecimals))/10000;
    case 'GPSSAltitude'
        xdata=statusData(:,col.GPSSAltitude);
    case 'GPSSHDOP'
        xdata=statusData(:,col.GPSSHDOP);
    case 'GPSSnumSat'
        xdata=statusData(:,col.GPSSnumSat);
    case 'GPSSLastValidData'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.GPSSLastValidData);
        else
            x=double(statusData(:,col.GPSSLastValidData)); eval(['xdata=',fcts2val.GPSSLastValidData,';']);
        end
    case 'GPSSGroundSpeed'
        xdata=double(statusData(:,col.GPSSGroundSpeed))/100;
    case 'GPSSHeading'
        xdata=double(statusData(:,col.GPSSHeading))/10;
    case 'TDet'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TDet);
        else
            x=double(statusData(:,col.TDet)); eval(['xdata=',fcts2val.TDet,';']);
        end
    case 'TempMissedLift'
        xdata=statusData(:,col.TempMissedLift);
    case 'TempNumberLift'
        xdata=statusData(:,col.TempNumberLift);
    case 'TempErrCRCLift'
        xdata=statusData(:,col.TempErrCRCLift);
    case 'TempNoResponseLift'
        xdata=statusData(:,col.TempNoResponseLift);
    case 'TempLaserPlate'
        if ~isnan(col.TempLaserPlate)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempLaserPlate);
            else
                x=double(statusData(:,col.TempLaserPlate)); eval(['xdata=',fcts2val.TempLaserPlate,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDyelaser'
        if ~isnan(col.TempDyelaser)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDyelaser);
            else
                x=double(statusData(:,col.TempDyelaser)); eval(['xdata=',fcts2val.TempDyelaser,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempRefCell'
        if ~isnan(col.TempRefCell)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempRefCell);
            else
                x=double(statusData(:,col.TempRefCell)); eval(['xdata=',fcts2val.TempRefCell,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDiodeEt'
        if ~isnan(col.TempDiodeEt)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDiodeEt);
            else
                x=double(statusData(:,col.TempDiodeEt)); eval(['xdata=',fcts2val.TempDiodeEt,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'Temp4L'
        if ~isnan(col.Temp4L)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.Temp4L);
            else
                x=double(statusData(:,col.Temp4L)); eval(['xdata=',fcts2val.Temp4L,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDiodeUV'
        if ~isnan(col.TempDiodeUV)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDiodeUV);
            else
                x=double(statusData(:,col.TempDiodeUV)); eval(['xdata=',fcts2val.TempDiodeUV,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempSensCardLift'
        if ~isnan(col.TempSensCardLift)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempSensCardLift);
            else
                x=double(statusData(:,col.TempSensCardLift)); eval(['xdata=',fcts2val.TempSensCardLift,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempCPULift'
        if ~isnan(col.TempCPULift)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempCPULift);
            else
                x=double(statusData(:,col.TempCPULift)); eval(['xdata=',fcts2val.TempCPULift,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDiodeGr'
        if ~isnan(col.TempDiodeGr)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDiodeGr);
            else
                x=double(statusData(:,col.TempDiodeGr)); eval(['xdata=',fcts2val.TempDiodeGr,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempMissedarmAxis'
        xdata=statusData(:,col.TempMissedarmAxis);
    case 'TempNumberarmAxis'
        xdata=statusData(:,col.TempNumberarmAxis);
    case 'TempErrCRCarmAxis'
        xdata=statusData(:,col.TempErrCRCarmAxis);
    case 'TempNoResponsearmAxis'
        xdata=statusData(:,col.TempNoResponsearmAxis);
    case 'TempPump'
        if ~isnan(col.TempPump)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempPump);
            else
                x=double(statusData(:,col.TempPump)); eval(['xdata=',fcts2val.TempPump,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempPumpOil'
        if ~isnan(col.TempPumpOil)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempPumpOil);
            else
                x=double(statusData(:,col.TempPumpOil)); eval(['xdata=',fcts2val.TempPumpOil,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDiodeWZ1out'
        if ~isnan(col.TempDiodeWZ1out)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDiodeWZ1out);
            else
                x=double(statusData(:,col.TempDiodeWZ1out)); eval(['xdata=',fcts2val.TempDiodeWZ1out,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempAxisPlate'
        if ~isnan(col.TempAxisPlate)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempAxisPlate);
            else
                x=double(statusData(:,col.TempAxisPlate)); eval(['xdata=',fcts2val.TempAxisPlate,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempArmPS'
        if ~isnan(col.TempArmPS)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempArmPS);
            else
                x=double(statusData(:,col.TempArmPS)); eval(['xdata=',fcts2val.TempArmPS,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDiodeWZ1in'
        if ~isnan(col.TempDiodeWZ1in)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDiodeWZ1in);
            else
                x=double(statusData(:,col.TempDiodeWZ1in)); eval(['xdata=',fcts2val.TempDiodeWZ1in,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDiodeWZ2in'
        if ~isnan(col.TempDiodeWZ2in)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDiodeWZ2in);
            else
                x=double(statusData(:,col.TempDiodeWZ2in)); eval(['xdata=',fcts2val.TempDiodeWZ2in,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempTMPSensCard'
        if ~isnan(col.TempTMPSensCard)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempTMPSensCard);
            else
                x=double(statusData(:,col.TempTMPSensCard)); eval(['xdata=',fcts2val.TempTMPSensCard,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempLaserSync'
        if ~isnan(col.TempLaserSync)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempLaserSync);
            else
                x=double(statusData(:,col.TempLaserSync)); eval(['xdata=',fcts2val.TempLaserSync,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempInverter'
        if ~isnan(col.TempInverter)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempInverter);
            else
                x=double(statusData(:,col.TempInverter)); eval(['xdata=',fcts2val.TempInverter,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempArmBackWall'
        if ~isnan(col.TempArmBackWall)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempArmBackWall);
            else
                x=double(statusData(:,col.TempArmBackWall)); eval(['xdata=',fcts2val.TempArmBackWall,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempBlower'
        if ~isnan(col.TempBlower)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempBlower);
            else
                x=double(statusData(:,col.TempBlower)); eval(['xdata=',fcts2val.TempBlower,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDiodeWZ2out'
        if ~isnan(col.TempDiodeWZ2out)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDiodeWZ2out);
            else
                x=double(statusData(:,col.TempDiodeWZ2out)); eval(['xdata=',fcts2val.TempDiodeWZ2out,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempCalPlate'
        if ~isnan(col.TempCalPlate)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempCalPlate);
            else
                x=double(statusData(:,col.TempCalPlate)); eval(['xdata=',fcts2val.TempCalPlate,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempHV'
        if ~isnan(col.TempHV)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempHV);
            else
                x=double(statusData(:,col.TempHV)); eval(['xdata=',fcts2val.TempHV,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempKuvette'
        if ~isnan(col.TempKuvette)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempKuvette);
            else
                x=double(statusData(:,col.TempKuvette)); eval(['xdata=',fcts2val.TempKuvette,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempPenray'
        if ~isnan(col.TempPenray)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempPenray);
            else
                x=double(statusData(:,col.TempPenray)); eval(['xdata=',fcts2val.TempPenray,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempMCP1'
        if ~isnan(col.TempMCP1)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempMCP1);
            else
                x=double(statusData(:,col.TempMCP1)); eval(['xdata=',fcts2val.TempMCP1,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempMCP2'
        if ~isnan(col.TempMCP2)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempMCP2);
            else
                x=double(statusData(:,col.TempMCP2)); eval(['xdata=',fcts2val.TempMCP2,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempPreamp1'
        if ~isnan(col.TempPreamp1)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempPreamp1);
            else
                x=double(statusData(:,col.TempPreamp1)); eval(['xdata=',fcts2val.TempPreamp1,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempPrallpl'
        if ~isnan(col.TempPrallpl)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempPrallpl);
            else
                x=double(statusData(:,col.TempPrallpl)); eval(['xdata=',fcts2val.TempPrallpl,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempAxis'
        if ~isnan(col.TempAxis)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempAxis);
            else
                x=double(statusData(:,col.TempAxis)); eval(['xdata=',fcts2val.TempAxis,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempScrollMotor'
        if ~isnan(col.TempScrollMotor)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempScrollMotor);
            else
                x=double(statusData(:,col.TempScrollMotor)); eval(['xdata=',fcts2val.TempScrollMotor,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempScrollContr'
        if ~isnan(col.TempScrollContr)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempScrollContr);
            else
                x=double(statusData(:,col.TempScrollContr)); eval(['xdata=',fcts2val.TempScrollContr,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempArmTempCard'
        if ~isnan(col.TempArmTempCard)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempArmTempCard);
            else
                x=double(statusData(:,col.TempArmTempCard)); eval(['xdata=',fcts2val.TempArmTempCard,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempPhotoOpp'
        if ~isnan(col.TempPhotoOpp)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempPhotoOpp);
            else
                x=double(statusData(:,col.TempPhotoOpp)); eval(['xdata=',fcts2val.TempPhotoOpp,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempPrallhz'
        if ~isnan(col.TempPrallhz)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempPrallhz);
            else
                x=double(statusData(:,col.TempPrallhz)); eval(['xdata=',fcts2val.TempPrallhz,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempFiberTube'
        if ~isnan(col.TempFiberTube)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempFiberTube);
            else
                x=double(statusData(:,col.TempFiberTube)); eval(['xdata=',fcts2val.TempFiberTube,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempLaserPlateid'
        if ~isnan(col.TempLaserPlateid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempLaserPlateid);
            else
                x=uint16(statusData(:,col.TempLaserPlateid)); eval(['xdata=',fcts2val.TempLaserPlateid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDyelaserid'
        if ~isnan(col.TempDyelaserid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDyelaserid);
            else
                x=uint16(statusData(:,col.TempDyelaserid)); eval(['xdata=',fcts2val.TempDyelaserid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempRefCellid'
        if ~isnan(col.TempRefCellid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempRefCellid);
            else
                x=uint16(statusData(:,col.TempRefCellid)); eval(['xdata=',fcts2val.TempRefCellid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDiodeEtid'
        if ~isnan(col.TempDiodeEtid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDiodeEtid);
            else
                x=uint16(statusData(:,col.TempDiodeEtid)); eval(['xdata=',fcts2val.TempDiodeEtid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'Temp4Lid'
        if ~isnan(col.Temp4Lid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.Temp4Lid);
            else
                x=uint16(statusData(:,col.Temp4Lid)); eval(['xdata=',fcts2val.Temp4Lid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDiodeUVid'
        if ~isnan(col.TempDiodeUVid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDiodeUVid);
            else
                x=uint16(statusData(:,col.TempDiodeUVid)); eval(['xdata=',fcts2val.TempDiodeUVid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempSensCardLiftid'
        if ~isnan(col.TempSensCardLiftid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempSensCardLiftid);
            else
                x=uint16(statusData(:,col.TempSensCardLiftid)); eval(['xdata=',fcts2val.TempSensCardLiftid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempCPULiftid'
        if ~isnan(col.TempCPULiftid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempCPULiftid);
            else
                x=uint16(statusData(:,col.TempCPULiftid)); eval(['xdata=',fcts2val.TempCPULiftid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDiodeGrid'
        if ~isnan(col.TempDiodeGrid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDiodeGrid);
            else
                x=uint16(statusData(:,col.TempDiodeGrid)); eval(['xdata=',fcts2val.TempDiodeGrid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempPumpid'
        if ~isnan(col.TempPumpid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempPumpid);
            else
                x=uint16(statusData(:,col.TempPumpid)); eval(['xdata=',fcts2val.TempPumpid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempPumpOilid'
        if ~isnan(col.TempPumpOilid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempPumpOilid);
            else
                x=uint16(statusData(:,col.TempPumpOilid)); eval(['xdata=',fcts2val.TempPumpOilid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDiodeWZ1outid'
        if ~isnan(col.TempDiodeWZ1outid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDiodeWZ1outid);
            else
                x=uint16(statusData(:,col.TempDiodeWZ1outid)); eval(['xdata=',fcts2val.TempDiodeWZ1outid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempAxisPlateid'
        if ~isnan(col.TempAxisPlateid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempAxisPlateid);
            else
                x=uint16(statusData(:,col.TempAxisPlateid)); eval(['xdata=',fcts2val.TempAxisPlateid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempArmPSid'
        if ~isnan(col.TempArmPSid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempArmPSid);
            else
                x=uint16(statusData(:,col.TempArmPSid)); eval(['xdata=',fcts2val.TempArmPSid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDiodeWZ1inid'
        if ~isnan(col.TempDiodeWZ1inid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDiodeWZ1inid);
            else
                x=uint16(statusData(:,col.TempDiodeWZ1inid)); eval(['xdata=',fcts2val.TempDiodeWZ1inid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDiodeWZ2inid'
        if ~isnan(col.TempDiodeWZ2inid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDiodeWZ2inid);
            else
                x=uint16(statusData(:,col.TempDiodeWZ2inid)); eval(['xdata=',fcts2val.TempDiodeWZ2inid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempTMPSensCardid'
        if ~isnan(col.TempTMPSensCardid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempTMPSensCardid);
            else
                x=uint16(statusData(:,col.TempTMPSensCardid)); eval(['xdata=',fcts2val.TempTMPSensCardid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempLaserSyncid'
        if ~isnan(col.TempLaserSyncid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempLaserSyncid);
            else
                x=uint16(statusData(:,col.TempLaserSyncid)); eval(['xdata=',fcts2val.TempLaserSyncid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempInverterid'
        if ~isnan(col.TempInverterid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempInverterid);
            else
                x=uint16(statusData(:,col.TempInverterid)); eval(['xdata=',fcts2val.TempInverterid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempArmBackWallid'
        if ~isnan(col.TempArmBackWallid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempArmBackWallid);
            else
                x=uint16(statusData(:,col.TempArmBackWallid)); eval(['xdata=',fcts2val.TempArmBackWallid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempBlowerid'
        if ~isnan(col.TempBlowerid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempBlowerid);
            else
                x=uint16(statusData(:,col.TempBlowerid)); eval(['xdata=',fcts2val.TempBlowerid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempDiodeWZ2outid'
        if ~isnan(col.TempDiodeWZ2outid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempDiodeWZ2outid);
            else
                x=uint16(statusData(:,col.TempDiodeWZ2outid)); eval(['xdata=',fcts2val.TempDiodeWZ2outid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempCalPlateid'
        if ~isnan(col.TempCalPlateid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempCalPlateid);
            else
                x=uint16(statusData(:,col.TempCalPlateid)); eval(['xdata=',fcts2val.TempCalPlateid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempHVid'
        if ~isnan(col.TempHVid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempHVid);
            else
                x=uint16(statusData(:,col.TempHVid)); eval(['xdata=',fcts2val.TempHVid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempKuvetteid'
        if ~isnan(col.TempKuvetteid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempKuvetteid);
            else
                x=uint16(statusData(:,col.TempKuvetteid)); eval(['xdata=',fcts2val.TempKuvetteid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempPenrayid'
        if ~isnan(col.TempPenrayid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempPenrayid);
            else
                x=uint16(statusData(:,col.TempPenrayid)); eval(['xdata=',fcts2val.TempPenrayid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempMCP1id'
        if ~isnan(col.TempMCP1id)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempMCP1id);
            else
                x=uint16(statusData(:,col.TempMCP1id)); eval(['xdata=',fcts2val.TempMCP1id,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempMCP2id'
        if ~isnan(col.TempMCP2id)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempMCP2id);
            else
                x=uint16(statusData(:,col.TempMCP2id)); eval(['xdata=',fcts2val.TempMCP2id,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempPreamp1id'
        if ~isnan(col.TempPreamp1id)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempPreamp1id);
            else
                x=uint16(statusData(:,col.TempPreamp1id)); eval(['xdata=',fcts2val.TempPreamp1id,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempPrallplid'
        if ~isnan(col.TempPrallplid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempPrallplid);
            else
                x=uint16(statusData(:,col.TempPrallplid)); eval(['xdata=',fcts2val.TempPrallplid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempAxisid'
        if ~isnan(col.TempAxisid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempAxisid);
            else
                x=uint16(statusData(:,col.TempAxisid)); eval(['xdata=',fcts2val.TempAxisid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempScrollMotorid'
        if ~isnan(col.TempScrollMotorid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempScrollMotorid);
            else
                x=uint16(statusData(:,col.TempScrollMotorid)); eval(['xdata=',fcts2val.TempScrollMotorid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempScrollContrid'
        if ~isnan(col.TempScrollContrid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempScrollContrid);
            else
                x=uint16(statusData(:,col.TempScrollContrid)); eval(['xdata=',fcts2val.TempScrollContrid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempArmTempCardid'
        if ~isnan(col.TempArmTempCardid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempArmTempCardid);
            else
                x=uint16(statusData(:,col.TempArmTempCardid)); eval(['xdata=',fcts2val.TempArmTempCardid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempPhotoOppid'
        if ~isnan(col.TempPhotoOppid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempPhotoOppid);
            else
                x=uint16(statusData(:,col.TempPhotoOppid)); eval(['xdata=',fcts2val.TempPhotoOppid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempPrallhzid'
        if ~isnan(col.TempPrallhzid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempPrallhzid);
            else
                x=uint16(statusData(:,col.TempPrallhzid)); eval(['xdata=',fcts2val.TempPrallhzid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'TempFiberTubeid'
        if ~isnan(col.TempFiberTubeid)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.TempFiberTubeid);
            else
                x=uint16(statusData(:,col.TempFiberTubeid)); eval(['xdata=',fcts2val.TempFiberTubeid,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'EtalonAction'
        if ~isnan(col.EtalonAction)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.EtalonAction);
            else
                x=uint16(statusData(:,col.EtalonAction)); eval(['xdata=',fcts2val.EtalonAction,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'InstrumentAction'
        if ~isnan(col.InstrumentAction)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.InstrumentAction);
            else
                x=uint16(statusData(:,col.InstrumentAction)); eval(['xdata=',fcts2val.InstrumentAction,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'ButterflyCurrentPosition'
        if ~isnan(col.ButterflyCurrentPosition)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.ButterflyCurrentPosition);
            else
                x=uint16(statusData(:,col.ButterflyCurrentPosition)); eval(['xdata=',fcts2val.ButterflyCurrentPosition,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'MirrorGr1XAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorGr1XAxisHi)),16);
        MirrorGr1XAxis=double(statusData(:,col.MirrorGr1XAxisHi)).*65536+double(statusData(:,col.MirrorGr1XAxisLo));
        MirrorGr1XAxis(Mirrorhelp==1)=bitset(floor(MirrorGr1XAxis(Mirrorhelp==1)),32,0)-2^32/2;
        xdata=MirrorGr1XAxis;
    case 'MirrorGr1YAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorGr1YAxisHi)),16);
        MirrorGr1YAxis=double(statusData(:,col.MirrorGr1YAxisHi)).*65536+double(statusData(:,col.MirrorGr1YAxisLo));
        MirrorGr1YAxis(Mirrorhelp==1)=bitset(floor(MirrorGr1YAxis(Mirrorhelp==1)),32,0)-2^32/2;
        xdata=MirrorGr1YAxis;
    case 'MirrorGr2XAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorGr2XAxisHi)),16);
        MirrorGr2XAxis=double(statusData(:,col.MirrorGr2XAxisHi)).*65536+double(statusData(:,col.MirrorGr2XAxisLo));
        MirrorGr2XAxis(Mirrorhelp==1)=bitset(floor(MirrorGr2XAxis(Mirrorhelp==1)),32,0)-2^32/2;
        xdata=MirrorGr2XAxis;
    case 'MirrorGr2YAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorGr2YAxisHi)),16);
        MirrorGr2YAxis=double(statusData(:,col.MirrorGr2YAxisHi)).*65536+double(statusData(:,col.MirrorGr2YAxisLo));
        MirrorGr2YAxis(Mirrorhelp==1)=bitset(floor(MirrorGr2YAxis(Mirrorhelp==1)),32,0)-2^32/2;
        xdata=MirrorGr2YAxis;
    case 'MirrorUV1XAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorUV1XAxisHi)),16);
        MirrorUV1XAxis=double(statusData(:,col.MirrorUV1XAxisHi)).*65536+double(statusData(:,col.MirrorUV1XAxisLo));
        MirrorUV1XAxis(Mirrorhelp==1)=bitset(floor(MirrorUV1XAxis(Mirrorhelp==1)),32,0)-2^32/2;
        xdata=MirrorUV1XAxis;
    case 'MirrorUV1YAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorUV1YAxisHi)),16);
        MirrorUV1YAxis=double(statusData(:,col.MirrorUV1YAxisHi)).*65536+double(statusData(:,col.MirrorUV1YAxisLo));
        MirrorUV1YAxis(Mirrorhelp==1)=bitset(floor(MirrorUV1YAxis(Mirrorhelp==1)),32,0)-2^32/2;
        xdata=MirrorUV1YAxis;
    case 'MirrorUV2XAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorUV2XAxisHi)),16);
        MirrorUV2XAxis=double(statusData(:,col.MirrorUV2XAxisHi)).*65536+double(statusData(:,col.MirrorUV2XAxisLo));
        MirrorUV2XAxis(Mirrorhelp==1)=bitset(floor(MirrorUV2XAxis(Mirrorhelp==1)),32,0)-2^32/2;
        xdata=MirrorUV2XAxis;
    case 'MirrorUV2YAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorUV2YAxisHi)),16);
        MirrorUV2YAxis=double(statusData(:,col.MirrorUV2YAxisHi)).*65536+double(statusData(:,col.MirrorUV2YAxisLo));
        MirrorUV2YAxis(Mirrorhelp==1)=bitset(floor(MirrorUV2YAxis(Mirrorhelp==1)),32,0)-2^32/2;
        xdata=MirrorUV2YAxis;
    case 'MirrorRealigning'
        if ~isnan(col.MirrorRealigning)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.MirrorRealigning);
            else
                x=uint16(statusData(:,col.MirrorRealigning)); eval(['xdata=',fcts2val.MirrorRealigning,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'MirrorMovingFlags'
        if ~isnan(col.MirrorMovingFlags)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.MirrorMovingFlags);
            else
                x=uint16(statusData(:,col.MirrorMovingFlags)); eval(['xdata=',fcts2val.MirrorMovingFlags,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'MirrorMinUVDiffCts'
        if ~isnan(col.MirrorMinUVDiffCts)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.MirrorMinUVDiffCts);
            else
                x=uint16(statusData(:,col.MirrorMinUVDiffCts)); eval(['xdata=',fcts2val.MirrorMinUVDiffCts,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end
    case 'MirrorRealignMinutes'
        if ~isnan(col.MirrorRealignMinutes)
            if get(handles.toggleX,'Value')==0
                xdata=statusData(:,col.MirrorRealignMinutes);
            else
                x=uint16(statusData(:,col.MirrorRealignMinutes)); eval(['xdata=',fcts2val.MirrorRealignMinutes,';']);
            end
        else
            xdata=statustime;
            xdata(:)=NaN;
        end

end
        
% assign data to y-axis
switch char(ypar)
    case 'Time'
        ydata=statustime;
    case 'DiodeGr'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.DiodeGr);
        else
            x=double(statusData(:,col.DiodeGr)); eval(['ydata=',fcts2val.DiodeGr,';']);
        end
    case 'DiodeUV'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.DiodeUV);
        else
            x=double(statusData(:,col.DiodeUV)); eval(['ydata=',fcts2val.DiodeUV,';']);
        end
    case 'DiodeEtalon'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.DiodeEtalon);
        else
            x=double(statusData(:,col.DiodeEtalon)); eval(['ydata=',fcts2val.DiodeEtalon,';']);
        end
    case 'DiodeWZ1in'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.DiodeWZ1in);
        else
            x=double(statusData(:,col.DiodeWZ1in)); eval(['ydata=',fcts2val.DiodeWZ1in,';']);
        end
    case 'DiodeWZ1out'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.DiodeWZ1out);
        else
            x=double(statusData(:,col.DiodeWZ1out)); eval(['ydata=',fcts2val.DiodeWZ1out,';']);
        end
    case 'DiodeWZ2in'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.DiodeWZ2in);
        else
            x=double(statusData(:,col.DiodeWZ2in)); eval(['ydata=',fcts2val.DiodeWZ2in,';']);
        end
    case 'DiodeWZ2out'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.DiodeWZ2out);
        else
            x=double(statusData(:,col.DiodeWZ2out)); eval(['ydata=',fcts2val.DiodeWZ2out,';']);
        end
    case 'Ophir'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Ophir);
        else
            x=double(statusData(:,col.Ophir)); eval(['ydata=',fcts2val.Ophir,';']);
        end
    case 'P20'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.P20);
        else
            x=double(statusData(:,col.P20)); eval(['ydata=',fcts2val.P20,';']);
        end
    case 'P1000'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.P1000);
        else
            x=double(statusData(:,col.P1000)); eval(['ydata=',fcts2val.P1000,';']);
        end
    case 'PRef'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.PRef);
        else
            x=double(statusData(:,col.PRef)); eval(['ydata=',fcts2val.PRef,';']);
        end
    case 'PDyelaser'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.PDyelaser);
        else
            x=double(statusData(:,col.PDyelaser)); eval(['ydata=',fcts2val.PDyelaser,';']);
        end
    case 'PVent'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.PVent);
        else
            x=double(statusData(:,col.PVent)); eval(['ydata=',fcts2val.PVent,';']);
        end
    case 'PNO'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.PNO);
        else
            x=double(statusData(:,col.PNO)); eval(['ydata=',fcts2val.PNO,';']);
        end
    case 'PCuvette'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.PCuvette);
        else
            x=double(statusData(:,col.PCuvette)); eval(['ydata=',fcts2val.PCuvette,';']);
        end
    case 'IFilament'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.IFilament);
        else
            x=double(statusData(:,col.IFilament)); eval(['ydata=',fcts2val.IFilament,';']);
        end
    case 'VHV'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.VHV);
        else
            x=double(statusData(:,col.VHV)); eval(['ydata=',fcts2val.VHV,';']);
        end
    case 'LaserTrigThresh'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.LaserTrigThresh);
        else
            x=double(statusData(:,col.LaserTrigThresh)); eval(['ydata=',fcts2val.LaserTrigThresh,';']);
        end
    case 'PMTThresh'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.PMTThresh);
        else
            x=double(statusData(:,col.PMTThresh)); eval(['ydata=',fcts2val.PMTThresh,';']);
        end
    case 'MCP1Thresh'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.MCP1Thresh);
        else
            x=double(statusData(:,col.MCP1Thresh)); eval(['ydata=',fcts2val.MCP1Thresh,';']);
        end
    case 'MCP2Thresh'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.MCP2Thresh);
        else
            x=double(statusData(:,col.MCP2Thresh)); eval(['ydata=',fcts2val.MCP2Thresh,';']);
        end
    case '+5VLift'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.V5VLift);
        else
            x=double(statusData(:,col.V5VLift)); eval(['ydata=',fcts2val.V5VLift,';']);
        end
    case '+28VLift'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.V28VLift);
        else
            x=double(statusData(:,col.V28VLift)); eval(['ydata=',fcts2val.V28VLift,';']);
        end
    case '+3.3VLift'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.V3_3VLift);
        else
            x=double(statusData(:,col.V3_3VLift)); eval(['ydata=',fcts2val.V3_3VLift,';']);
        end
    case '+1.5VLift'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.V1_5VLift);
        else
            x=double(statusData(:,col.V1_5VLift)); eval(['ydata=',fcts2val.V1_5VLift,';']);
        end
    case '+5VarmAxis'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.V5VarmAxis);
        else
            x=double(statusData(:,col.V5VarmAxis)); eval(['ydata=',fcts2val.V5VarmAxis,';']);
        end
    case '+28VarmAxis'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.V28VarmAxis);
        else
            x=double(statusData(:,col.V28VarmAxis)); eval(['ydata=',fcts2val.V28VarmAxis,';']);
        end
    case '+3.3VarmAxis'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.V3_3VarmAxis);
        else
            x=double(statusData(:,col.V3_3VarmAxis)); eval(['ydata=',fcts2val.V3_3VarmAxis,';']);
        end
    case '+1.5VarmAxis'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.V1_5VarmAxis);
        else
            x=double(statusData(:,col.V1_5VarmAxis)); eval(['ydata=',fcts2val.V1_5VarmAxis,';']);
        end
    case 'I28V'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.I28V);
        else
            x=double(statusData(:,col.I28V)); eval(['ydata=',fcts2val.I28V,';']);
        end
    case 'EtaSpd'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.etaCurSpd);
        else
            x=double(statusData(:,col.etaCurSpd)); eval(['ydata=',fcts2val.etaCurSpd,';']);
        end
    case 'EtaSetPos'
        Etalonhelp=bitget(uint16(statusData(:,col.etaSetPosHigh)),16);
        EtalonSetPos=double(statusData(:,col.etaSetPosHigh)).*65536+double(statusData(:,col.etaSetPosLow));
        EtalonSetPos(Etalonhelp==1)=bitset(floor(EtalonSetPos(Etalonhelp==1)),32,0)-2^32/2;
        ydata=EtalonSetPos;
    case 'EtaCurPos'
        Etalonhelp=bitget(uint16(statusData(:,col.etaCurPosHigh)),16);
        EtalonCurPos=double(statusData(:,col.etaCurPosHigh)).*65536+double(statusData(:,col.etaCurPosLow));
        EtalonCurPos(Etalonhelp==1)=bitset(floor(EtalonCurPos(Etalonhelp==1)),32,0)-2^32/2;
        ydata=EtalonCurPos;
    case 'EtaEncPos'
        Etalonhelp=bitget(uint16(statusData(:,col.etaEncoderPosHigh)),16);
        EtalonEncPos=double(statusData(:,col.etaEncoderPosHigh)).*65536+double(statusData(:,col.etaEncoderPosLow));
        EtalonEncPos(Etalonhelp==1)=bitset(floor(EtalonEncPos(Etalonhelp==1)),32,0)-2^32/2;
        ydata=EtalonEncPos;
    case 'EtaIndPos'
        Etalonhelp=bitget(uint16(statusData(:,col.etaIndexPosHigh)),16);
        EtalonIndPos=double(statusData(:,col.etaIndexPosHigh)).*65536+double(statusData(:,col.etaIndexPosLow));
        EtalonIndPos(Etalonhelp==1)=bitset(floor(EtalonIndPos(Etalonhelp==1)),32,0)-2^32/2;
        ydata=EtalonIndPos;
    case 'EtaOnlinePos'
        Etalonhelp=bitget(uint16(statusData(:,col.etaOnlinePosHigh)),16);
        OnlinePos=double(statusData(:,col.etaOnlinePosHigh)).*65536+double(statusData(:,col.etaOnlinePosLow));
        OnlinePos(Etalonhelp==1)=bitset(floor(OnlinePos(Etalonhelp==1)),32,0)-2^32/2;
        ydata=OnlinePos;
    case 'ValveVoltLift'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ValveVoltLift);
        else
            x=double(statusData(:,col.ValveVoltLift)); eval(['ydata=',fcts2val.ValveVoltLift,';']);
        end
    case 'Valve1VoltarmAxis'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Valve1VoltarmAxis);
        else
            x=double(statusData(:,col.Valve1VoltarmAxis)); eval(['ydata=',fcts2val.Valve1VoltarmAxis,';']);
        end
    case 'Valve2VoltarmAxis'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Valve2VoltarmAxis);
        else
            x=double(statusData(:,col.Valve2VoltarmAxis)); eval(['ydata=',fcts2val.Valve2VoltarmAxis,';']);
        end
    case 'MFCFlow'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.MFCFlow);
        else
            x=double(statusData(:,col.MFCFlow)); eval(['ydata=',fcts2val.MFCFlow,';']);
        end
    case 'MFCSetFlow'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.MFCSetFlow);
        else
            x=double(statusData(:,col.MFCSetFlow)); eval(['ydata=',fcts2val.MFCSetFlow,';']);
        end
    case 'PitotAbs'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.PitotAbs);
        else
            x=double(statusData(:,col.PitotAbs)); eval(['ydata=',fcts2val.PitotAbs,';']);
        end
    case 'PitotDiff'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.PitotDiff);
        else
            x=double(statusData(:,col.PitotDiff)); eval(['ydata=',fcts2val.PitotDiff,';']);
        end
    case 'PhototubeLamp1'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.PhototubeLamp1);
        else
            x=double(statusData(:,col.PhototubeLamp1)); eval(['ydata=',fcts2val.PhototubeLamp1,';']);
        end
    case 'PhototubeLamp2'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.PhototubeLamp2);
        else
            x=double(statusData(:,col.PhototubeLamp2)); eval(['ydata=',fcts2val.PhototubeLamp2,';']);
        end
    case 'ccMasterDelayLift'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ccMasterDelayLift);
        else
            x=double(statusData(:,col.ccMasterDelayLift)); eval(['ydata=',fcts2val.ccMasterDelayLift,';']);
        end
    case 'ccMasterDelayarmAxis'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ccMasterDelayarmAxis);
        else
            x=double(statusData(:,col.ccMasterDelayarmAxis)); eval(['ydata=',fcts2val.ccMasterDelayarmAxis,';']);
        end
    case 'ccShiftDelay0'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ccShiftDelay0);
        else
            x=double(statusData(:,col.ccShiftDelay0)); eval(['ydata=',fcts2val.ccShiftDelay0,';']);
        end
    case 'ccCounts0'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ccCounts0);
        else
            x=double(statusData(:,col.ccCounts0)); eval(['ydata=',fcts2val.ccCounts0,';']);
        end
    case 'ccPulses0'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ccPulses0);
        else
            x=double(statusData(:,col.ccPulses0)); eval(['ydata=',fcts2val.ccPulses0,';']);
        end
    case 'ccShiftDelay1'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ccShiftDelay1);
        else
            x=double(statusData(:,col.ccShiftDelay1)); eval(['ydata=',fcts2val.ccShiftDelay1,';']);
        end
    case 'ccGateDelay1'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ccGateDelay1);
        else
            x=double(statusData(:,col.ccGateDelay1)); eval(['ydata=',fcts2val.ccGateDelay1,';']);
        end
    case 'ccGateWidth1'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ccGateWidth1);
        else
            x=double(statusData(:,col.ccGateWidth1)); eval(['ydata=',fcts2val.ccGateWidth1,';']);
        end
    case 'ccCounts1'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ccCounts1);
        else
            x=double(statusData(:,col.ccCounts1)); eval(['ydata=',fcts2val.ccCounts1,';']);
        end
    case 'ccPulses1'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ccPulses1);
        else
            x=double(statusData(:,col.ccPulses1)); eval(['ydata=',fcts2val.ccPulses1,';']);
        end
    case 'ccShiftDelay2'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ccShiftDelay2);
        else
            x=double(statusData(:,col.ccShiftDelay2)); eval(['ydata=',fcts2val.ccShiftDelay2,';']);
        end
    case 'ccGateDelay2'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ccGateDelay2);
        else
            x=double(statusData(:,col.ccGateDelay2)); eval(['ydata=',fcts2val.ccGateDelay2,';']);
        end
    case 'ccGateWidth2'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ccGateWidth2);
        else
            x=double(statusData(:,col.ccGateWidth2)); eval(['ydata=',fcts2val.ccGateWidth2,';']);
        end
    case 'ccCounts2'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ccCounts2);
        else
            x=double(statusData(:,col.ccCounts2)); eval(['ydata=',fcts2val.ccCounts2,';']);
        end
    case 'ccPulses2'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ccPulses2);
        else
            x=double(statusData(:,col.ccPulses2)); eval(['ydata=',fcts2val.ccPulses2,';']);
        end
    case 'GPSMsecondsUTC'
        ydata=statusData(:,col.GPSMsecondsUTC);
    case 'GPSMLongitude'
        ydata=double(statusData(:,col.GPSMLongitude))/60-180 ...
            +double(statusData(:,col.GPSMLongitudeDecimals))/10000;
    case 'GPSMLatitude'
        ydata=double(statusData(:,col.GPSMLatitude))/60-90 ...
            +double(statusData(:,col.GPSMLatitudeDecimals))/10000;
    case 'GPSMAltitude'
        ydata=statusData(:,col.GPSMAltitude);
    case 'GPSMHDOP'
        ydata=statusData(:,col.GPSMHDOP);
    case 'GPSMnumSat'
        ydata=statusData(:,col.GPSMnumSat);
    case 'GPSMLastValidData'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.GPSMLastValidData);
        else
            x=double(statusData(:,col.GPSMLastValidData)); eval(['ydata=',fcts2val.GPSMLastValidData,';']);
        end
    case 'GPSMGroundSpeed'
        ydata=double(statusData(:,col.GPSMGroundSpeed))/100;
    case 'GPSMHeading'
        ydata=double(statusData(:,col.GPSMHeading))/10;
    case 'GPSSsecondsUTC'
        ydata=statusData(:,col.GPSSsecondsUTC);
    case 'GPSSLongitude'
        ydata=double(statusData(:,col.GPSSLongitude))/60-180 ...
            +double(statusData(:,col.GPSSLongitudeDecimals))/10000;
    case 'GPSSLatitude'
        ydata=double(statusData(:,col.GPSSLatitude))/60-90 ...
            +double(statusData(:,col.GPSSLatitudeDecimals))/10000;
    case 'GPSSAltitude'
        ydata=statusData(:,col.GPSSAltitude);
    case 'GPSSHDOP'
        ydata=statusData(:,col.GPSSHDOP);
    case 'GPSSnumSat'
        ydata=statusData(:,col.GPSSnumSat);
    case 'GPSSLastValidData'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.GPSSLastValidData);
        else
            x=double(statusData(:,col.GPSSLastValidData)); eval(['ydata=',fcts2val.GPSSLastValidData,';']);
        end
    case 'GPSSGroundSpeed'
        ydata=double(statusData(:,col.GPSSGroundSpeed))/100;
    case 'GPSSHeading'
        ydata=double(statusData(:,col.GPSSHeading))/10;
    case 'TDet'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TDet);
        else
            x=double(statusData(:,col.TDet)); eval(['ydata=',fcts2val.TDet,';']);
        end
    case 'TempMissedLift'
        ydata=statusData(:,col.TempMissedLift);
    case 'TempNumberLift'
        ydata=statusData(:,col.TempNumberLift);
    case 'TempErrCRCLift'
        ydata=statusData(:,col.TempErrCRCLift);
    case 'TempNoResponseLift'
        ydata=statusData(:,col.TempNoResponseLift);
    case 'TempLaserPlate'
        if ~isnan(col.TempLaserPlate)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempLaserPlate);
            else
                x=double(statusData(:,col.TempLaserPlate)); eval(['ydata=',fcts2val.TempLaserPlate,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDyelaser'
        if ~isnan(col.TempDyelaser)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDyelaser);
            else
                x=double(statusData(:,col.TempDyelaser)); eval(['ydata=',fcts2val.TempDyelaser,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempRefCell'
        if ~isnan(col.TempRefCell)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempRefCell);
            else
                x=double(statusData(:,col.TempRefCell)); eval(['ydata=',fcts2val.TempRefCell,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDiodeEt'
        if ~isnan(col.TempDiodeEt)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDiodeEt);
            else
                x=double(statusData(:,col.TempDiodeEt)); eval(['ydata=',fcts2val.TempDiodeEt,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'Temp4L'
        if ~isnan(col.Temp4L)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.Temp4L);
            else
                x=double(statusData(:,col.Temp4L)); eval(['ydata=',fcts2val.Temp4L,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDiodeUV'
        if ~isnan(col.TempDiodeUV)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDiodeUV);
            else
                x=double(statusData(:,col.TempDiodeUV)); eval(['ydata=',fcts2val.TempDiodeUV,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempSensCardLift'
        if ~isnan(col.TempSensCardLift)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempSensCardLift);
            else
                x=double(statusData(:,col.TempSensCardLift)); eval(['ydata=',fcts2val.TempSensCardLift,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempCPULift'
        if ~isnan(col.TempCPULift)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempCPULift);
            else
                x=double(statusData(:,col.TempCPULift)); eval(['ydata=',fcts2val.TempCPULift,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDiodeGr'
        if ~isnan(col.TempDiodeGr)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDiodeGr);
            else
                x=double(statusData(:,col.TempDiodeGr)); eval(['ydata=',fcts2val.TempDiodeGr,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempMissedarmAxis'
        ydata=statusData(:,col.TempMissedarmAxis);
    case 'TempNumberarmAxis'
        ydata=statusData(:,col.TempNumberarmAxis);
    case 'TempErrCRCarmAxis'
        ydata=statusData(:,col.TempErrCRCarmAxis);
    case 'TempNoResponsearmAxis'
        ydata=statusData(:,col.TempNoResponsearmAxis);
    case 'TempPump'
        if ~isnan(col.TempPump)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempPump);
            else
                x=double(statusData(:,col.TempPump)); eval(['ydata=',fcts2val.TempPump,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempPumpOil'
        if ~isnan(col.TempPumpOil)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempPumpOil);
            else
                x=double(statusData(:,col.TempPumpOil)); eval(['ydata=',fcts2val.TempPumpOil,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDiodeWZ1out'
        if ~isnan(col.TempDiodeWZ1out)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDiodeWZ1out);
            else
                x=double(statusData(:,col.TempDiodeWZ1out)); eval(['ydata=',fcts2val.TempDiodeWZ1out,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempAxisPlate'
        if ~isnan(col.TempAxisPlate)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempAxisPlate);
            else
                x=double(statusData(:,col.TempAxisPlate)); eval(['ydata=',fcts2val.TempAxisPlate,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempArmPS'
        if ~isnan(col.TempArmPS)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempArmPS);
            else
                x=double(statusData(:,col.TempArmPS)); eval(['ydata=',fcts2val.TempArmPS,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDiodeWZ1in'
        if ~isnan(col.TempDiodeWZ1in)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDiodeWZ1in);
            else
                x=double(statusData(:,col.TempDiodeWZ1in)); eval(['ydata=',fcts2val.TempDiodeWZ1in,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDiodeWZ2in'
        if ~isnan(col.TempDiodeWZ2in)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDiodeWZ2in);
            else
                x=double(statusData(:,col.TempDiodeWZ2in)); eval(['ydata=',fcts2val.TempDiodeWZ2in,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempTMPSensCard'
        if ~isnan(col.TempTMPSensCard)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempTMPSensCard);
            else
                x=double(statusData(:,col.TempTMPSensCard)); eval(['ydata=',fcts2val.TempTMPSensCard,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempLaserSync'
        if ~isnan(col.TempLaserSync)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempLaserSync);
            else
                x=double(statusData(:,col.TempLaserSync)); eval(['ydata=',fcts2val.TempLaserSync,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempInverter'
        if ~isnan(col.TempInverter)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempInverter);
            else
                x=double(statusData(:,col.TempInverter)); eval(['ydata=',fcts2val.TempInverter,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempArmBackWall'
        if ~isnan(col.TempArmBackWall)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempArmBackWall);
            else
                x=double(statusData(:,col.TempArmBackWall)); eval(['ydata=',fcts2val.TempArmBackWall,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempBlower'
        if ~isnan(col.TempBlower)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempBlower);
            else
                x=double(statusData(:,col.TempBlower)); eval(['ydata=',fcts2val.TempBlower,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDiodeWZ2out'
        if ~isnan(col.TempDiodeWZ2out)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDiodeWZ2out);
            else
                x=double(statusData(:,col.TempDiodeWZ2out)); eval(['ydata=',fcts2val.TempDiodeWZ2out,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempCalPlate'
        if ~isnan(col.TempCalPlate)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempCalPlate);
            else
                x=double(statusData(:,col.TempCalPlate)); eval(['ydata=',fcts2val.TempCalPlate,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempHV'
        if ~isnan(col.TempHV)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempHV);
            else
                x=double(statusData(:,col.TempHV)); eval(['ydata=',fcts2val.TempHV,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempKuvette'
        if ~isnan(col.TempKuvette)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempKuvette);
            else
                x=double(statusData(:,col.TempKuvette)); eval(['ydata=',fcts2val.TempKuvette,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempPenray'
        if ~isnan(col.TempPenray)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempPenray);
            else
                x=double(statusData(:,col.TempPenray)); eval(['ydata=',fcts2val.TempPenray,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempMCP1'
        if ~isnan(col.TempMCP1)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempMCP1);
            else
                x=double(statusData(:,col.TempMCP1)); eval(['ydata=',fcts2val.TempMCP1,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempMCP2'
        if ~isnan(col.TempMCP2)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempMCP2);
            else
                x=double(statusData(:,col.TempMCP2)); eval(['ydata=',fcts2val.TempMCP2,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempPreamp1'
        if ~isnan(col.TempPreamp1)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempPreamp1);
            else
                x=double(statusData(:,col.TempPreamp1)); eval(['ydata=',fcts2val.TempPreamp1,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempPrallpl'
        if ~isnan(col.TempPrallpl)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempPrallpl);
            else
                x=double(statusData(:,col.TempPrallpl)); eval(['ydata=',fcts2val.TempPrallpl,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempAxis'
        if ~isnan(col.TempAxis)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempAxis);
            else
                x=double(statusData(:,col.TempAxis)); eval(['ydata=',fcts2val.TempAxis,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempScrollMotor'
        if ~isnan(col.TempScrollMotor)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempScrollMotor);
            else
                x=double(statusData(:,col.TempScrollMotor)); eval(['ydata=',fcts2val.TempScrollMotor,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempScrollContr'
        if ~isnan(col.TempScrollContr)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempScrollContr);
            else
                x=double(statusData(:,col.TempScrollContr)); eval(['ydata=',fcts2val.TempScrollContr,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempArmTempCard'
        if ~isnan(col.TempArmTempCard)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempArmTempCard);
            else
                x=double(statusData(:,col.TempArmTempCard)); eval(['ydata=',fcts2val.TempArmTempCard,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempPhotoOpp'
        if ~isnan(col.TempPhotoOpp)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempPhotoOpp);
            else
                x=double(statusData(:,col.TempPhotoOpp)); eval(['ydata=',fcts2val.TempPhotoOpp,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempPrallhz'
        if ~isnan(col.TempPrallhz)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempPrallhz);
            else
                x=double(statusData(:,col.TempPrallhz)); eval(['ydata=',fcts2val.TempPrallhz,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempFiberTube'
        if ~isnan(col.TempFiberTube)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempFiberTube);
            else
                x=double(statusData(:,col.TempFiberTube)); eval(['ydata=',fcts2val.TempFiberTube,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempLaserPlateid'
        if ~isnan(col.TempLaserPlateid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempLaserPlateid);
            else
                x=uint16(statusData(:,col.TempLaserPlateid)); eval(['ydata=',fcts2val.TempLaserPlateid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDyelaserid'
        if ~isnan(col.TempDyelaserid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDyelaserid);
            else
                x=uint16(statusData(:,col.TempDyelaserid)); eval(['ydata=',fcts2val.TempDyelaserid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempRefCellid'
        if ~isnan(col.TempRefCellid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempRefCellid);
            else
                x=uint16(statusData(:,col.TempRefCellid)); eval(['ydata=',fcts2val.TempRefCellid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDiodeEtid'
        if ~isnan(col.TempDiodeEtid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDiodeEtid);
            else
                x=uint16(statusData(:,col.TempDiodeEtid)); eval(['ydata=',fcts2val.TempDiodeEtid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'Temp4Lid'
        if ~isnan(col.Temp4Lid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.Temp4Lid);
            else
                x=uint16(statusData(:,col.Temp4Lid)); eval(['ydata=',fcts2val.Temp4Lid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDiodeUVid'
        if ~isnan(col.TempDiodeUVid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDiodeUVid);
            else
                x=uint16(statusData(:,col.TempDiodeUVid)); eval(['ydata=',fcts2val.TempDiodeUVid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempSensCardLiftid'
        if ~isnan(col.TempSensCardLiftid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempSensCardLiftid);
            else
                x=uint16(statusData(:,col.TempSensCardLiftid)); eval(['ydata=',fcts2val.TempSensCardLiftid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempCPULiftid'
        if ~isnan(col.TempCPULiftid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempCPULiftid);
            else
                x=uint16(statusData(:,col.TempCPULiftid)); eval(['ydata=',fcts2val.TempCPULiftid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDiodeGrid'
        if ~isnan(col.TempDiodeGrid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDiodeGrid);
            else
                x=uint16(statusData(:,col.TempDiodeGrid)); eval(['ydata=',fcts2val.TempDiodeGrid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempPumpid'
        if ~isnan(col.TempPumpid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempPumpid);
            else
                x=uint16(statusData(:,col.TempPumpid)); eval(['ydata=',fcts2val.TempPumpid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempPumpOilid'
        if ~isnan(col.TempPumpOilid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempPumpOilid);
            else
                x=uint16(statusData(:,col.TempPumpOilid)); eval(['ydata=',fcts2val.TempPumpOilid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDiodeWZ1outid'
        if ~isnan(col.TempDiodeWZ1outid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDiodeWZ1outid);
            else
                x=uint16(statusData(:,col.TempDiodeWZ1outid)); eval(['ydata=',fcts2val.TempDiodeWZ1outid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempAxisPlateid'
        if ~isnan(col.TempAxisPlateid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempAxisPlateid);
            else
                x=uint16(statusData(:,col.TempAxisPlateid)); eval(['ydata=',fcts2val.TempAxisPlateid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempArmPSid'
        if ~isnan(col.TempArmPSid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempArmPSid);
            else
                x=uint16(statusData(:,col.TempArmPSid)); eval(['ydata=',fcts2val.TempArmPSid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDiodeWZ1inid'
        if ~isnan(col.TempDiodeWZ1inid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDiodeWZ1inid);
            else
                x=uint16(statusData(:,col.TempDiodeWZ1inid)); eval(['ydata=',fcts2val.TempDiodeWZ1inid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDiodeWZ2inid'
        if ~isnan(col.TempDiodeWZ2inid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDiodeWZ2inid);
            else
                x=uint16(statusData(:,col.TempDiodeWZ2inid)); eval(['ydata=',fcts2val.TempDiodeWZ2inid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempTMPSensCardid'
        if ~isnan(col.TempTMPSensCardid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempTMPSensCardid);
            else
                x=uint16(statusData(:,col.TempTMPSensCardid)); eval(['ydata=',fcts2val.TempTMPSensCardid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempLaserSyncid'
        if ~isnan(col.TempLaserSyncid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempLaserSyncid);
            else
                x=uint16(statusData(:,col.TempLaserSyncid)); eval(['ydata=',fcts2val.TempLaserSyncid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempInverterid'
        if ~isnan(col.TempInverterid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempInverterid);
            else
                x=uint16(statusData(:,col.TempInverterid)); eval(['ydata=',fcts2val.TempInverterid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempArmBackWallid'
        if ~isnan(col.TempArmBackWallid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempArmBackWallid);
            else
                x=uint16(statusData(:,col.TempArmBackWallid)); eval(['ydata=',fcts2val.TempArmBackWallid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempBlowerid'
        if ~isnan(col.TempBlowerid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempBlowerid);
            else
                x=uint16(statusData(:,col.TempBlowerid)); eval(['ydata=',fcts2val.TempBlowerid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempDiodeWZ2outid'
        if ~isnan(col.TempDiodeWZ2outid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempDiodeWZ2outid);
            else
                x=uint16(statusData(:,col.TempDiodeWZ2outid)); eval(['ydata=',fcts2val.TempDiodeWZ2outid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempCalPlateid'
        if ~isnan(col.TempCalPlateid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempCalPlateid);
            else
                x=uint16(statusData(:,col.TempCalPlateid)); eval(['ydata=',fcts2val.TempCalPlateid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempHVid'
        if ~isnan(col.TempHVid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempHVid);
            else
                x=uint16(statusData(:,col.TempHVid)); eval(['ydata=',fcts2val.TempHVid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempKuvetteid'
        if ~isnan(col.TempKuvetteid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempKuvetteid);
            else
                x=uint16(statusData(:,col.TempKuvetteid)); eval(['ydata=',fcts2val.TempKuvetteid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempPenrayid'
        if ~isnan(col.TempPenrayid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempPenrayid);
            else
                x=uint16(statusData(:,col.TempPenrayid)); eval(['ydata=',fcts2val.TempPenrayid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempMCP1id'
        if ~isnan(col.TempMCP1id)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempMCP1id);
            else
                x=uint16(statusData(:,col.TempMCP1id)); eval(['ydata=',fcts2val.TempMCP1id,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempMCP2id'
        if ~isnan(col.TempMCP2id)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempMCP2id);
            else
                x=uint16(statusData(:,col.TempMCP2id)); eval(['ydata=',fcts2val.TempMCP2id,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempPreamp1id'
        if ~isnan(col.TempPreamp1id)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempPreamp1id);
            else
                x=uint16(statusData(:,col.TempPreamp1id)); eval(['ydata=',fcts2val.TempPreamp1id,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempPrallplid'
        if ~isnan(col.TempPrallplid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempPrallplid);
            else
                x=uint16(statusData(:,col.TempPrallplid)); eval(['ydata=',fcts2val.TempPrallplid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempAxisid'
        if ~isnan(col.TempAxisid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempAxisid);
            else
                x=uint16(statusData(:,col.TempAxisid)); eval(['ydata=',fcts2val.TempAxisid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempScrollMotorid'
        if ~isnan(col.TempScrollMotorid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempScrollMotorid);
            else
                x=uint16(statusData(:,col.TempScrollMotorid)); eval(['ydata=',fcts2val.TempScrollMotorid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempScrollContrid'
        if ~isnan(col.TempScrollContrid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempScrollContrid);
            else
                x=uint16(statusData(:,col.TempScrollContrid)); eval(['ydata=',fcts2val.TempScrollContrid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempArmTempCardid'
        if ~isnan(col.TempArmTempCardid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempArmTempCardid);
            else
                x=uint16(statusData(:,col.TempArmTempCardid)); eval(['ydata=',fcts2val.TempArmTempCardid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempPhotoOppid'
        if ~isnan(col.TempPhotoOppid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempPhotoOppid);
            else
                x=uint16(statusData(:,col.TempPhotoOppid)); eval(['ydata=',fcts2val.TempPhotoOppid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempPrallhzid'
        if ~isnan(col.TempPrallhzid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempPrallhzid);
            else
                x=uint16(statusData(:,col.TempPrallhzid)); eval(['ydata=',fcts2val.TempPrallhzid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'TempFiberTubeid'
        if ~isnan(col.TempFiberTubeid)
            if get(handles.toggleY,'Value')==0
                ydata=statusData(:,col.TempFiberTubeid);
            else
                x=uint16(statusData(:,col.TempFiberTubeid)); eval(['ydata=',fcts2val.TempFiberTubeid,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'EtalonAction'
        if ~isnan(col.EtalonAction)
            if get(handles.toggleX,'Value')==0
                ydata=statusData(:,col.EtalonAction);
            else
                x=uint16(statusData(:,col.EtalonAction)); eval(['ydata=',fcts2val.EtalonAction,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'InstrumentAction'
        if ~isnan(col.InstrumentAction)
            if get(handles.toggleX,'Value')==0
                ydata=statusData(:,col.InstrumentAction);
            else
                x=uint16(statusData(:,col.InstrumentAction)); eval(['ydata=',fcts2val.InstrumentAction,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'ButterflyCurrentPosition'
        if ~isnan(col.ButterflyCurrentPosition)
            if get(handles.toggleX,'Value')==0
                ydata=statusData(:,col.ButterflyCurrentPosition);
            else
                x=uint16(statusData(:,col.ButterflyCurrentPosition)); eval(['ydata=',fcts2val.ButterflyCurrentPosition,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'MirrorGr1XAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorGr1XAxisHi)),16);
        MirrorGr1XAxis=double(statusData(:,col.MirrorGr1XAxisHi)).*65536+double(statusData(:,col.MirrorGr1XAxisLo));
        MirrorGr1XAxis(Mirrorhelp==1)=bitset(floor(MirrorGr1XAxis(Mirrorhelp==1)),32,0)-2^32/2;
        ydata=MirrorGr1XAxis;
    case 'MirrorGr1YAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorGr1YAxisHi)),16);
        MirrorGr1YAxis=double(statusData(:,col.MirrorGr1YAxisHi)).*65536+double(statusData(:,col.MirrorGr1YAxisLo));
        MirrorGr1YAxis(Mirrorhelp==1)=bitset(floor(MirrorGr1YAxis(Mirrorhelp==1)),32,0)-2^32/2;
        ydata=MirrorGr1YAxis;
    case 'MirrorGr2XAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorGr2XAxisHi)),16);
        MirrorGr2XAxis=double(statusData(:,col.MirrorGr2XAxisHi)).*65536+double(statusData(:,col.MirrorGr2XAxisLo));
        MirrorGr2XAxis(Mirrorhelp==1)=bitset(floor(MirrorGr2XAxis(Mirrorhelp==1)),32,0)-2^32/2;
        ydata=MirrorGr2XAxis;
    case 'MirrorGr2YAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorGr2YAxisHi)),16);
        MirrorGr2YAxis=double(statusData(:,col.MirrorGr2YAxisHi)).*65536+double(statusData(:,col.MirrorGr2YAxisLo));
        MirrorGr2YAxis(Mirrorhelp==1)=bitset(floor(MirrorGr2YAxis(Mirrorhelp==1)),32,0)-2^32/2;
        ydata=MirrorGr2YAxis;
    case 'MirrorUV1XAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorUV1XAxisHi)),16);
        MirrorUV1XAxis=double(statusData(:,col.MirrorUV1XAxisHi)).*65536+double(statusData(:,col.MirrorUV1XAxisLo));
        MirrorUV1XAxis(Mirrorhelp==1)=bitset(floor(MirrorUV1XAxis(Mirrorhelp==1)),32,0)-2^32/2;
        ydata=MirrorUV1XAxis;
    case 'MirrorUV1YAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorUV1YAxisHi)),16);
        MirrorUV1YAxis=double(statusData(:,col.MirrorUV1YAxisHi)).*65536+double(statusData(:,col.MirrorUV1YAxisLo));
        MirrorUV1YAxis(Mirrorhelp==1)=bitset(floor(MirrorUV1YAxis(Mirrorhelp==1)),32,0)-2^32/2;
        ydata=MirrorUV1YAxis;
    case 'MirrorUV2XAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorUV2XAxisHi)),16);
        MirrorUV2XAxis=double(statusData(:,col.MirrorUV2XAxisHi)).*65536+double(statusData(:,col.MirrorUV2XAxisLo));
        MirrorUV2XAxis(Mirrorhelp==1)=bitset(floor(MirrorUV2XAxis(Mirrorhelp==1)),32,0)-2^32/2;
        ydata=MirrorUV2XAxis;
    case 'MirrorUV2YAxis'
        Mirrorhelp=bitget(uint16(statusData(:,col.MirrorUV2YAxisHi)),16);
        MirrorUV2YAxis=double(statusData(:,col.MirrorUV2YAxisHi)).*65536+double(statusData(:,col.MirrorUV2YAxisLo));
        MirrorUV2YAxis(Mirrorhelp==1)=bitset(floor(MirrorUV2YAxis(Mirrorhelp==1)),32,0)-2^32/2;
        ydata=MirrorUV2YAxis;
    case 'MirrorRealigning'
        if ~isnan(col.MirrorRealigning)
            if get(handles.toggleX,'Value')==0
                ydata=statusData(:,col.MirrorRealigning);
            else
                x=uint16(statusData(:,col.MirrorRealigning)); eval(['ydata=',fcts2val.MirrorRealigning,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'MirrorMovingFlags'
        if ~isnan(col.MirrorMovingFlags)
            if get(handles.toggleX,'Value')==0
                ydata=statusData(:,col.MirrorMovingFlags);
            else
                x=uint16(statusData(:,col.MirrorMovingFlags)); eval(['ydata=',fcts2val.MirrorMovingFlags,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'MirrorMinUVDiffCts'
        if ~isnan(col.MirrorMinUVDiffCts)
            if get(handles.toggleX,'Value')==0
                ydata=statusData(:,col.MirrorMinUVDiffCts);
            else
                x=uint16(statusData(:,col.MirrorMinUVDiffCts)); eval(['ydata=',fcts2val.MirrorMinUVDiffCts,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
    case 'MirrorRealignMinutes'
        if ~isnan(col.MirrorRealignMinutes)
            if get(handles.toggleX,'Value')==0
                ydata=statusData(:,col.MirrorRealignMinutes);
            else
                x=uint16(statusData(:,col.MirrorRealignMinutes)); eval(['ydata=',fcts2val.MirrorRealignMinutes,';']);
            end
        else
            ydata=statustime;
            ydata(:)=NaN;
        end
end

% display latest x and y values
set(handles.txtxvalue,'String',num2str(xdata(lastrow,:)));
set(handles.txtyvalue,'String',num2str(ydata(lastrow,:)));

xlim1=double(str2double(get(handles.editxlim1,'String')));
xlim2=double(str2double(get(handles.editxlim2,'String')));
ylim1=double(str2double(get(handles.editylim1,'String')));
ylim2=double(str2double(get(handles.editylim2,'String')));
if isa(xdata(lastrow),'numeric') & isa(ydata(lastrow),'numeric')
    plot(handles.axes1,xdata,ydata,'o');
    xlimits=[xlim1/100*(max(xdata)-min(xdata))+min(xdata), xlim2/100*(max(xdata)-min(xdata))+min(xdata)];
    ylimits=[ylim1/100*(max(ydata)-min(ydata))+min(ydata), ylim2/100*(max(ydata)-min(ydata))+min(ydata)];
    if isnan(xlimits) xlimits=[0,1]; end
    if xlimits(1)==xlimits(2) xlimits(2)=xlimits(1)+1; end
    if isnan(ylimits) ylimits=[0,1]; end
    if ylimits(1)==ylimits(2) ylimits(2)=ylimits(1)+1; end
    set(handles.axes1,'xlim',xlimits);
    set(handles.axes1,'ylim',ylimits);
    grid(handles.axes1);
end



% --- Outputs from this function are returned to the command line.
function varargout = Sensors_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on selection change in listboxX.
function listboxX_Callback(hObject, eventdata, handles)
% hObject    handle to listboxX (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns listboxX contents as cell array
%        contents{get(hObject,'Value')} returns selected item from listboxX


% --- Executes during object creation, after setting all properties.
function listboxX_CreateFcn(hObject, eventdata, handles)
% hObject    handle to listboxX (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: listbox controls usually have a white background on Windows.
set(hObject,'BackgroundColor','white');
vars={'Time';'DiodeGr';'DiodeUV';'DiodeEtalon';'DiodeWZ1in';'DiodeWZ1out';'DiodeWZ2in';'DiodeWZ2out';...
    'Ophir';'P20';'P1000';'PRef';'PDyelaser';'PVent';'PNO';'PCuvette';'IFilament';...
    'VHV';'LaserTrigThresh';'PMTThresh';'MCP1Thresh';'MCP2Thresh';'+5VLift';'+28VLift';...
    '+3.3VLift';'+1.5VLift';'+5VarmAxis';'+28VarmAxis';'+3.3VarmAxis';'+1.5VarmAxis';'I28V';...
    'EtaSpd';'EtaSetPos';'EtaCurPos';'EtaEncPos';'EtaIndPos';'EtaOnlinePos';'ValveVoltLift';...
    'Valve1VoltarmAxis';'Valve2VoltarmAxis';'MFCFlow';'MFCSetFlow';'PitotAbs';'PitotDiff';...
    'PhototubeLamp1';'PhototubeLamp2';'ccMasterDelayLift';'ccMasterDelayarmAxis';'ccShiftDelay0';...
    'ccCounts0';'ccPulses0';'ccShiftDelay1';'ccGateDelay1';...
    'ccGateWidth1';'ccCounts1';'ccPulses1';'ccShiftDelay2';'ccGateDelay2';...
    'ccGateWidth2';'ccCounts2';'ccPulses2';
    'GPSMsecondsUTC';'GPSMLongitude';'GPSMLatitude';'GPSMAltitude';'GPSMHDOP';'GPSMnumSat';...
    'GPSMLastValidData';'GPSMGroundSpeed';'GPSMHeading';...
    'GPSSsecondsUTC';'GPSSLongitude';'GPSSLatitude';'GPSSAltitude';'GPSSHDOP';'GPSSnumSat';...
    'GPSSLastValidData';'GPSSGroundSpeed';'GPSSHeading';...
    'TDet';'TempMissedLift';'TempNumberLift';'TempErrCRCLift';'TempNoResponseLift';...
    'TempLaserPlate';'TempDyelaser';'TempRefCell';'TempDiodeEt';'Temp4L';...
    'TempDiodeUV';'TempSensCardLift';'TempCPULift';'TempDiodeGr';...
    'TempMissedarmAxis';'TempNumberarmAxis';'TempErrCRCarmAxis';'TempNoResponsearmAxis';
    'TempPump';'TempPumpOil';'TempDiodeWZ1out';'TempAxisPlate';'TempArmPS';'TempDiodeWZ1in';...
    'TempDiodeWZ2in';'TempTMPSensCard';'TempLaserSync';'TempInverter';'TempArmBackWall';'TempBlower';...
    'TempDiodeWZ2out';'TempCalPlate';'TempHV';'TempKuvette';'TempPenray';'TempMCP1';'TempMCP2';'TempPreamp1';...
    'TempPrallpl';'TempAxis';'TempScrollMotor';'TempScrollContr';'TempArmTempCard';'TempPhotoOpp';'TempPrallhz';'TempFiberTube';...
    'TempLaserPlateid';'TempDyelaserid';'TempRefCellid';'TempDiodeEtid';'Temp4Lid';...
    'TempDiodeUVid';'TempSensCardLiftid';'TempCPULiftid';'TempDiodeGrid';...
    'TempPumpid';'TempPumpOilid';'TempDiodeWZ1outid';'TempAxisPlateid';'TempArmPSid';'TempDiodeWZ1inid';...
    'TempDiodeWZ2inid';'TempTMPSensCardid';'TempLaserSyncid';'TempInverterid';'TempArmBackWallid';'TempBlowerid';...
    'TempDiodeWZ2outid';'TempCalPlateid';'TempHVid';'TempKuvetteid';'TempPenrayid';'TempMCP1id';'TempMCP2id';'TempPreamp1id';...
    'TempPrallplid';'TempAxisid';'TempScrollMotorid';'TempScrollContrid';'TempArmTempCardid';'TempPhotoOppid';'TempPrallhzid';'TempFiberTubeid';...
    'EtalonAction';'InstrumentAction';'ButterflyCurrentPosition';...
    'MirrorGr1XAxis';'MirrorGr1YAxis';'MirrorGr2XAxis';'MirrorGr2YAxis';'MirrorUV1XAxis';'MirrorUV1YAxis';'MirrorUV2XAxis';'MirrorUV2YAxis';...
    'MirrorRealigning';'MirrorMovingFlags';'MirrorMinUVDiffCts';'MirrorRealignMinutes'};
set(hObject,'String',vars);


% --- Executes on selection change in listboxY.
function listboxY_Callback(hObject, eventdata, handles)
% hObject    handle to listboxY (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns listboxY contents as cell array
%        contents{get(hObject,'Value')} returns selected item from listboxY


% --- Executes during object creation, after setting all properties.
function listboxY_CreateFcn(hObject, eventdata, handles)
% hObject    handle to listboxY (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: listbox controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
set(hObject,'BackgroundColor','white');
vars={'Time';'DiodeGr';'DiodeUV';'DiodeEtalon';'DiodeWZ1in';'DiodeWZ1out';'DiodeWZ2in';'DiodeWZ2out';...
    'Ophir';'P20';'P1000';'PRef';'PDyelaser';'PVent';'PNO';'PCuvette';'IFilament';...
    'VHV';'LaserTrigThresh';'PMTThresh';'MCP1Thresh';'MCP2Thresh';'+5VLift';'+28VLift';...
    '+3.3VLift';'+1.5VLift';'+5VarmAxis';'+28VarmAxis';'+3.3VarmAxis';'+1.5VarmAxis';'I28V';...
    'EtaSpd';'EtaSetPos';'EtaCurPos';'EtaEncPos';'EtaIndPos';'EtaOnlinePos';'ValveVoltLift';...
    'Valve1VoltarmAxis';'Valve2VoltarmAxis';'MFCFlow';'MFCSetFlow';'PitotAbs';'PitotDiff';...
    'PhototubeLamp1';'PhototubeLamp2';'ccMasterDelayLift';'ccMasterDelayarmAxis';'ccShiftDelay0';...
    'ccCounts0';'ccPulses0';'ccShiftDelay1';'ccGateDelay1';...
    'ccGateWidth1';'ccCounts1';'ccPulses1';'ccShiftDelay2';'ccGateDelay2';...
    'ccGateWidth2';'ccCounts2';'ccPulses2';
    'GPSMsecondsUTC';'GPSMLongitude';'GPSMLatitude';'GPSMAltitude';'GPSMHDOP';'GPSMnumSat';...
    'GPSMLastValidData';'GPSMGroundSpeed';'GPSMHeading';...
    'GPSSsecondsUTC';'GPSSLongitude';'GPSSLatitude';'GPSSAltitude';'GPSSHDOP';'GPSSnumSat';...
    'GPSSLastValidData';'GPSSGroundSpeed';'GPSSHeading';...
    'TDet';'TempMissedLift';'TempNumberLift';'TempErrCRCLift';'TempNoResponseLift';...
    'TempLaserPlate';'TempDyelaser';'TempRefCell';'TempDiodeEt';'Temp4L';...
    'TempDiodeUV';'TempSensCardLift';'TempCPULift';'TempDiodeGr';...
    'TempMissedarmAxis';'TempNumberarmAxis';'TempErrCRCarmAxis';'TempNoResponsearmAxis';
    'TempPump';'TempPumpOil';'TempDiodeWZ1out';'TempAxisPlate';'TempArmPS';'TempDiodeWZ1in';...
    'TempDiodeWZ2in';'TempTMPSensCard';'TempLaserSync';'TempInverter';'TempArmBackWall';'TempBlower';...
    'TempDiodeWZ2out';'TempCalPlate';'TempHV';'TempKuvette';'TempPenray';'TempMCP1';'TempMCP2';'TempPreamp1';...
    'TempPrallpl';'TempAxis';'TempScrollMotor';'TempScrollContr';'TempArmTempCard';'TempPhotoOpp';'TempPrallhz';'TempFiberTube';...
    'TempLaserPlateid';'TempDyelaserid';'TempRefCellid';'TempDiodeEtid';'Temp4Lid';...
    'TempDiodeUVid';'TempSensCardLiftid';'TempCPULiftid';'TempDiodeGrid';...
    'TempPumpid';'TempPumpOilid';'TempDiodeWZ1outid';'TempAxisPlateid';'TempArmPSid';'TempDiodeWZ1inid';...
    'TempDiodeWZ2inid';'TempTMPSensCardid';'TempLaserSyncid';'TempInverterid';'TempArmBackWallid';'TempBlowerid';...
    'TempDiodeWZ2outid';'TempCalPlateid';'TempHVid';'TempKuvetteid';'TempPenrayid';'TempMCP1id';'TempMCP2id';'TempPreamp1id';...
    'TempPrallplid';'TempAxisid';'TempScrollMotorid';'TempScrollContrid';'TempArmTempCardid';'TempPhotoOppid';'TempPrallhzid';'TempFiberTubeid';...
    'EtalonAction';'InstrumentAction';'ButterflyCurrentPosition';...
    'MirrorGr1XAxis';'MirrorGr1YAxis';'MirrorGr2XAxis';'MirrorGr2YAxis';'MirrorUV1XAxis';'MirrorUV1YAxis';'MirrorUV2XAxis';'MirrorUV2YAxis';...
    'MirrorRealigning';'MirrorMovingFlags';'MirrorMinUVDiffCts';'MirrorRealignMinutes'};
set(hObject,'String',vars);




% --- Executes on button press in togglebutton1.
function toggleX_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton1
if get(hObject,'Value')==0 
    set(hObject,'String','Counts')
else
    set(hObject,'String','Value')
end

% --- Executes on button press in toggleY.
function toggleY_Callback(hObject, eventdata, handles)
% hObject    handle to toggleY (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleY
if get(hObject,'Value')==0 
    set(hObject,'String','Counts')
else
    set(hObject,'String','Value')
end




% --- Executes on button press in pshExit.
function pshExit_Callback(hObject, eventdata, handles)
% hObject    handle to pshExit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
stop(handles.Timer);
delete(handles.Timer);
close(handles.figure1);



function editxlim1_Callback(hObject, eventdata, handles)
% hObject    handle to editxlim1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editxlim1 as text
%        str2double(get(hObject,'String')) returns contents of editxlim1 as a double
xlim1=uint8(str2double(get(hObject,'String')));
if (xlim1>100) set(hObject,'String','100');
else set(hObject,'String',num2str(xlim1));
end



function editxlim2_Callback(hObject, eventdata, handles)
% hObject    handle to editxlim2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editxlim2 as text
%        str2double(get(hObject,'String')) returns contents of editxlim2 as a double
xlim2=uint8(str2double(get(hObject,'String')));
if xlim2>100 set(hObject,'String','100');
else set(hObject,'String',num2str(xlim2));
end



function editylim1_Callback(hObject, eventdata, handles)
% hObject    handle to editylim1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editylim1 as text
%        str2double(get(hObject,'String')) returns contents of editylim1 as a double
ylim1=uint8(str2double(get(hObject,'String')));
if ylim1>100 set(hObject,'String','100');
else set(hObject,'String',num2str(ylim1));
end


function editylim2_Callback(hObject, eventdata, handles)
% hObject    handle to editylim2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editylim2 as text
%        str2double(get(hObject,'String')) returns contents of editylim2 as a double
ylim2=uint8(str2double(get(hObject,'String')));
if ylim2>100 set(hObject,'String','100');
else set(hObject,'String',num2str(ylim2));
end


