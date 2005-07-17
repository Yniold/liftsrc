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
           double(statusData(:,5))./86400.0;

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
    case 'EtaSpd'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.etaCurSpd);
        else
            x=double(statusData(:,col.etaCurSpd)); eval(['xdata=',fcts2val.etaCurSpd,';']);
        end
    case 'EtaSetPos'
        Etalonhelp=int32(statusData(:,col.etaSetPosLow));
        EtalonSetPos=(Etalonhelp)+int32(statusData(:,col.etaSetPosHigh));
        EtalonSetPos(Etalonhelp>32767)=EtalonSetPos(Etalonhelp>32767)-65535;
        xdata=EtalonSetPos;
    case 'EtaCurPos'
        Etalonhelp=int32(statusData(:,col.etaCurPosLow+2));
        EtalonCurPos=(Etalonhelp)+int32(statusData(:,col.etaCurPosHigh)); 
        EtalonCurPos(Etalonhelp>32767)=EtalonCurPos(Etalonhelp>32767)-65535;
        xdata=EtalonCurPos;
    case 'EtaEncPos'
        Etalonhelp=int32(statusData(:,col.etaEncoderPosLow)); 
        EtalonEncPos=(Etalonhelp)+int32(statusData(:,col.etaEncoderPosHigh)); 
        EtalonEncPos(Etalonhelp>32767)=EtalonEncPos(Etalonhelp>32767)-65535;
        xdata=EtalonEncPos;
    case 'EtaIndPos'
        Etalonhelp=int32(statusData(:,col.etaIndexPosLow)); 
        EtalonIndPos=(Etalonhelp)+int32(statusData(:,col.etaIndexPosHigh)); 
        EtalonIndPos(Etalonhelp>32767)=EtalonIndPos(Etalonhelp>32767)-65535;
        xdata=EtalonIndPos;
    case 'EtaOnlinePos'
        Etalonhelp=int32(statusData(:,col.etaOnlinePosLow)); 
        EtalonOnlinePos=(Etalonhelp)+int32(statusData(:,col.etaOnlinePosHigh)); 
        EtalonOnlinePos(Etalonhelp>32767)=EtalonOnlinePos(Etalonhelp>32767)-65535;
        xdata=EtalonOnlinePos;
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
    case 'GPSsecondsUTC'
        xdata=statusData(:,col.GPSsecondsUTC);
    case 'GPSLongitude'
        xdata=double(statusData(:,col.GPSLongitude))/60-180 ...
            +double(statusData(:,col.GPSLongitudeDecimals))/10000;
    case 'GPSLatitude'
        xdata=double(statusData(:,col.GPSLatitude))/60-90 ...
            +double(statusData(:,col.GPSLatitudeDecimals))/10000;
    case 'GPSAltitude'
        xdata=statusData(:,col.GPSAltitude);
    case 'GPSHDOP'
        xdata=statusData(:,col.GPSHDOP);
    case 'GPSnumSat'
        xdata=statusData(:,col.GPSnumSat);
    case 'GPSLastValidData'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.GPSLastValidData);
        else
            x=double(statusData(:,col.GPSLastValidData)); eval(['xdata=',fcts2val.GPSLastValidData,';']);
        end
    case 'GPSGroundSpeed'
        xdata=double(statusData(:,col.GPSGroundSpeed))/100;
    case 'GPSHeading'
        xdata=double(statusData(:,col.GPSHeading))/10;
    case 'TDet'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TDet);
        else
            x=double(statusData(:,col.TDet)); eval(['xdata=',fcts2val.TDet,';']);
        end
    case 'Temp0L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp0L);
        else
            x=double(statusData(:,col.Temp0L)); eval(['xdata=',fcts2val.Temp0L,';']);
        end
    case 'TempDyelaser'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempDyelaser);
        else
            x=double(statusData(:,col.Temp1L)); eval(['xdata=',fcts2val.TempDyelaser,';']);
        end
    case 'TempRefCell'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempRefCell);
        else
            x=double(statusData(:,col.TempRefCell)); eval(['xdata=',fcts2val.TempRefCell,';']);
        end
    case 'TempDiodeEt'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempDiodeEt);
        else
            x=double(statusData(:,col.TempDiodeEt)); eval(['xdata=',fcts2val.TempDiodeEt,';']);
        end
    case 'Temp4L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp4L);
        else
            x=double(statusData(:,col.Temp4L)); eval(['xdata=',fcts2val.Temp4L,';']);
        end
    case 'TempDiodeUV'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempDiodeUV);
        else
            x=double(statusData(:,col.TempDiodeUV)); eval(['xdata=',fcts2val.TempDiodeUV,';']);
        end
    case 'Temp6L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp6L);
        else
            x=double(statusData(:,col.Temp6L)); eval(['xdata=',fcts2val.Temp6L,';']);
        end
    case 'Temp7L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp7L);
        else
            x=double(statusData(:,col.Temp7L)); eval(['xdata=',fcts2val.Temp7L,';']);
        end
    case 'Temp8L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp8L);
        else
            x=double(statusData(:,col.Temp8L)); eval(['xdata=',fcts2val.Temp8L,';']);
        end
    case 'Temp9L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp9L);
        else
            x=double(statusData(:,col.Temp9L)); eval(['xdata=',fcts2val.Temp9L,';']);
        end
    case 'Temp10L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp10L);
        else
            x=double(statusData(:,col.Temp10L)); eval(['xdata=',fcts2val.Temp10L,';']);
        end
    case 'Temp11L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp11L);
        else
            x=double(statusData(:,col.Temp11L)); eval(['xdata=',fcts2val.Temp11L,';']);
        end
    case 'Temp12L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp12L);
        else
            x=double(statusData(:,col.Temp12L)); eval(['xdata=',fcts2val.Temp12L,';']);
        end
    case 'Temp13L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp13L);
        else
            x=double(statusData(:,col.Temp13L)); eval(['xdata=',fcts2val.Temp13L,';']);
        end
    case 'Temp14L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp14L);
        else
            x=double(statusData(:,col.Temp14L)); eval(['xdata=',fcts2val.Temp14L,';']);
        end
    case 'Temp15L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp15L);
        else
            x=double(statusData(:,col.Temp15L)); eval(['xdata=',fcts2val.Temp15L,';']);
        end
    case 'Temp16L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp16L);
        else
            x=double(statusData(:,col.Temp16L)); eval(['xdata=',fcts2val.Temp16L,';']);
        end
    case 'Temp17L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp17L);
        else
            x=double(statusData(:,col.Temp17L)); eval(['xdata=',fcts2val.Temp17L,';']);
        end
    case 'Temp18L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp18L);
        else
            x=double(statusData(:,col.Temp18L)); eval(['xdata=',fcts2val.Temp18L,';']);
        end
    case 'Temp19L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp19L);
        else
            x=double(statusData(:,col.Temp19L)); eval(['xdata=',fcts2val.Temp19L,';']);
        end
    case 'Temp20L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp20L);
        else
            x=double(statusData(:,col.Temp20L)); eval(['xdata=',fcts2val.Temp20L,';']);
        end
    case 'Temp21L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp21L);
        else
            x=double(statusData(:,col.Temp21L)); eval(['xdata=',fcts2val.Temp21L,';']);
        end
    case 'Temp22L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp22L);
        else
            x=double(statusData(:,col.Temp22L)); eval(['xdata=',fcts2val.Temp22L,';']);
        end
    case 'TempPump'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempPump);
        else
            x=double(statusData(:,col.TempPump)); eval(['xdata=',fcts2val.TempPump,';']);
        end
    case 'TempPumpOil'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempPumpOil);
        else
            x=double(statusData(:,col.TempPumpOil)); eval(['xdata=',fcts2val.TempPumpOil,';']);
        end
    case 'TempDiodeWZ1out'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempDiodeWZ1out);
        else
            x=double(statusData(:,col.TempDiodeWZ1out)); eval(['xdata=',fcts2val.TempDiodeWZ1out,';']);
        end
    case 'TempAxisPlate'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempAxisPlate);
        else
            x=double(statusData(:,col.TempAxisPlate)); eval(['xdata=',fcts2val.TempAxisPlate,';']);
        end
    case 'TempArmPS'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempArmPS);
        else
            x=double(statusData(:,col.TempArmPS)); eval(['xdata=',fcts2val.TempArmPS,';']);
        end
    case 'TempDiodeWZ1in'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempDiodeWZ1in);
        else
            x=double(statusData(:,col.TempDiodeWZ1in)); eval(['xdata=',fcts2val.TempDiodeWZ1in,';']);
        end
    case 'TempDiodeWZ2in'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempDiodeWZ2in);
        else
            x=double(statusData(:,col.TempDiodeWZ2in)); eval(['xdata=',fcts2val.TempDiodeWZ2in,';']);
        end
    case 'TempTMPSensCard'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempTMPSensCard);
        else
            x=double(statusData(:,col.TempTMPSensCard)); eval(['xdata=',fcts2val.TempTMPSensCard,';']);
        end
    case 'TempLaserSync'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempLaserSync);
        else
            x=double(statusData(:,col.TempLaserSync)); eval(['xdata=',fcts2val.TempLaserSync,';']);
        end
    case 'TempInverter'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempInverter);
        else
            x=double(statusData(:,col.TempInverter)); eval(['xdata=',fcts2val.TempInverter,';']);
        end
    case 'TempArmBackWall'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempArmBackWall);
        else
            x=double(statusData(:,col.TempArmBackWall)); eval(['xdata=',fcts2val.TempArmBackWall,';']);
        end
    case 'TempBlower'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempBlower);
        else
            x=double(statusData(:,col.TempBlower)); eval(['xdata=',fcts2val.TempBlower,';']);
        end
    case 'TempDiodeWZ2out'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempDiodeWZ2out);
        else
            x=double(statusData(:,col.TempDiodeWZ2out)); eval(['xdata=',fcts2val.TempDiodeWZ2out,';']);
        end
    case 'TempCalPlate'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempCalPlate);
        else
            x=double(statusData(:,col.TempCalPlate)); eval(['xdata=',fcts2val.TempCalPlate,';']);
        end
    case 'TempHV'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempHV);
        else
            x=double(statusData(:,col.TempHV)); eval(['xdata=',fcts2val.TempHV,';']);
        end
    case 'Temp15WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp15WP);
        else
            x=double(statusData(:,col.Temp15WP)); eval(['xdata=',fcts2val.Temp15WP,';']);
        end
    case 'Temp16WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp16WP);
        else
            x=double(statusData(:,col.Temp16WP)); eval(['xdata=',fcts2val.Temp16WP,';']);
        end
    case 'Temp17WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp17WP);
        else
            x=double(statusData(:,col.Temp17WP)); eval(['xdata=',fcts2val.Temp17WP,';']);
        end
    case 'Temp18WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp18WP);
        else
            x=double(statusData(:,col.Temp18WP)); eval(['xdata=',fcts2val.Temp18WP,';']);
        end
    case 'Temp19WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp19WP);
        else
            x=double(statusData(:,col.Temp19WP)); eval(['xdata=',fcts2val.Temp19WP,';']);
        end
    case 'Temp20WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp20WP);
        else
            x=double(statusData(:,col.Temp20WP)); eval(['xdata=',fcts2val.Temp20WP,';']);
        end
    case 'Temp21WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp21WP);
        else
            x=double(statusData(:,col.Temp21WP)); eval(['xdata=',fcts2val.Temp21WP,';']);
        end
    case 'Temp22WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp22WP);
        else
            x=double(statusData(:,col.Temp22WP)); eval(['xdata=',fcts2val.Temp22WP,';']);
        end
    case 'Temp0Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp0Lid);
        else
            x=double(statusData(:,col.Temp0Lid)); eval(['xdata=',fcts2val.Temp0Lid,';']);
        end
    case 'TempDyelaserid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempDyelaserid);
        else
            x=double(statusData(:,col.TempDyelaserid)); eval(['xdata=',fcts2val.TempDyelaserid,';']);
        end
    case 'TempRefCellid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempRefCellid);
        else
            x=double(statusData(:,col.TempRefCellid)); eval(['xdata=',fcts2val.TempRefCellid,';']);
        end
    case 'TempDiodeEtid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempDiodeEtid);
        else
            x=double(statusData(:,col.TempDiodeEtid)); eval(['xdata=',fcts2val.TempDiodeEtid,';']);
        end
    case 'Temp4Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp4Lid);
        else
            x=double(statusData(:,col.Temp4Lid)); eval(['xdata=',fcts2val.Temp4Lid,';']);
        end
    case 'TempDiodeUVid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempDiodeUVid);
        else
            x=double(statusData(:,col.TempDiodeUVid)); eval(['xdata=',fcts2val.TempDiodeUVid,';']);
        end
    case 'Temp6Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp6Lid);
        else
            x=double(statusData(:,col.Temp6Lid)); eval(['xdata=',fcts2val.Temp6Lid,';']);
        end
    case 'Temp7Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp7Lid);
        else
            x=double(statusData(:,col.Temp7Lid)); eval(['xdata=',fcts2val.Temp7Lid,';']);
        end
    case 'Temp8Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp8Lid);
        else
            x=double(statusData(:,col.Temp8Lid)); eval(['xdata=',fcts2val.Temp8Lid,';']);
        end
    case 'Temp9Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp9Lid);
        else
            x=double(statusData(:,col.Temp9Lid)); eval(['xdata=',fcts2val.Temp9Lid,';']);
        end
    case 'Temp10Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp10Lid);
        else
            x=double(statusData(:,col.Temp10Lid)); eval(['xdata=',fcts2val.Temp10Lid,';']);
        end
    case 'Temp11Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp11Lid);
        else
            x=double(statusData(:,col.Temp11Lid)); eval(['xdata=',fcts2val.Temp11Lid,';']);
        end
    case 'Temp12Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp12Lid);
        else
            x=double(statusData(:,col.Temp12Lid)); eval(['xdata=',fcts2val.Temp12Lid,';']);
        end
    case 'Temp13Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp13Lid);
        else
            x=double(statusData(:,col.Temp13Lid)); eval(['xdata=',fcts2val.Temp13Lid,';']);
        end
    case 'Temp14Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp14Lid);
        else
            x=double(statusData(:,col.Temp14Lid)); eval(['xdata=',fcts2val.Temp14Lid,';']);
        end
    case 'Temp15Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp15Lid);
        else
            x=double(statusData(:,col.Temp15Lid)); eval(['xdata=',fcts2val.Temp15Lid,';']);
        end
    case 'Temp16Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp16Lid);
        else
            x=double(statusData(:,col.Temp16Lid)); eval(['xdata=',fcts2val.Temp16Lid,';']);
        end
    case 'Temp17Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp17Lid);
        else
            x=double(statusData(:,col.Temp17Lid)); eval(['xdata=',fcts2val.Temp17Lid,';']);
        end
    case 'Temp18Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp18Lid);
        else
            x=double(statusData(:,col.Temp18Lid)); eval(['xdata=',fcts2val.Temp18Lid,';']);
        end
    case 'Temp19Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp19Lid);
        else
            x=double(statusData(:,col.Temp19Lid)); eval(['xdata=',fcts2val.Temp19Lid,';']);
        end
    case 'Temp20Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp20Lid);
        else
            x=double(statusData(:,col.Temp20Lid)); eval(['xdata=',fcts2val.Temp20Lid,';']);
        end
    case 'Temp21Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp21Lid);
        else
            x=double(statusData(:,col.Temp21Lid)); eval(['xdata=',fcts2val.Temp21Lid,';']);
        end
    case 'Temp22Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp22Lid);
        else
            x=double(statusData(:,col.Temp22Lid)); eval(['xdata=',fcts2val.Temp22Lid,';']);
        end
    case 'TempPumpid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempPumpid);
        else
            x=double(statusData(:,col.TempPumpid)); eval(['xdata=',fcts2val.TempPumpid,';']);
        end
    case 'TempPumpOilid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempPumpOilid);
        else
            x=double(statusData(:,col.TempPumpOilid)); eval(['xdata=',fcts2val.TempPumpOilid,';']);
        end
    case 'TempDiodeWZ1outid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempDiodeWZ1outid);
        else
            x=double(statusData(:,col.TempDiodeWZ1outid)); eval(['xdata=',fcts2val.TempDiodeWZ1outid,';']);
        end
    case 'TempAxisPlateid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempAxisPlateid);
        else
            x=double(statusData(:,col.TempAxisPlateid)); eval(['xdata=',fcts2val.TempAxisPlateid,';']);
        end
    case 'TempArmPSid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempArmPSid);
        else
            x=double(statusData(:,col.TempArmPSid)); eval(['xdata=',fcts2val.TempArmPSid,';']);
        end
    case 'TempDiodeWZ1inid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempDiodeWZ1inid);
        else
            x=double(statusData(:,col.TempDiodeWZ1inid)); eval(['xdata=',fcts2val.TempDiodeWZ1inid,';']);
        end
    case 'TempDiodeWZ2inid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempDiodeWZ2inid);
        else
            x=double(statusData(:,col.TempDiodeWZ2inid)); eval(['xdata=',fcts2val.TempDiodeWZ2inid,';']);
        end
    case 'TempTMPSensCardid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempTMPSensCardid);
        else
            x=double(statusData(:,col.TempTMPSensCardid)); eval(['xdata=',fcts2val.TempTMPSensCardid,';']);
        end
    case 'TempLaserSyncid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempLaserSyncid);
        else
            x=double(statusData(:,col.TempLaserSyncid)); eval(['xdata=',fcts2val.TempLaserSyncid,';']);
        end
    case 'TempInverterid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempInverterid);
        else
            x=double(statusData(:,col.TempInverterid)); eval(['xdata=',fcts2val.TempInverterid,';']);
        end
    case 'TempArmBackWallid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempArmBackWallid);
        else
            x=double(statusData(:,col.TempArmBackWallid)); eval(['xdata=',fcts2val.TempArmBackWallid,';']);
        end
    case 'TempBlowerid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempBlowerid);
        else
            x=double(statusData(:,col.TempBlowerid)); eval(['xdata=',fcts2val.TempBlowerid,';']);
        end
    case 'TempDiodeWZ2outid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempDiodeWZ2outid);
        else
            x=double(statusData(:,col.TempDiodeWZ2outid)); eval(['xdata=',fcts2val.TempDiodeWZ2outid,';']);
        end
    case 'TempCalPlateid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempCalPlateid);
        else
            x=double(statusData(:,col.TempCalPlateid)); eval(['xdata=',fcts2val.TempCalPlateid,';']);
        end
    case 'TempHVid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TempHVid);
        else
            x=double(statusData(:,col.TempHVid)); eval(['xdata=',fcts2val.TempHVid,';']);
        end
    case 'Temp15WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp15WPid);
        else
            x=double(statusData(:,col.Temp15WPid)); eval(['xdata=',fcts2val.Temp15WPid,';']);
        end
    case 'Temp16WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp16WPid);
        else
            x=double(statusData(:,col.Temp16WPid)); eval(['xdata=',fcts2val.Temp16WPid,';']);
        end
    case 'Temp17WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp17WPid);
        else
            x=double(statusData(:,col.Temp17WPid)); eval(['xdata=',fcts2val.Temp17WPid,';']);
        end
    case 'Temp18WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp18WPid);
        else
            x=double(statusData(:,col.Temp18WPid)); eval(['xdata=',fcts2val.Temp18WPid,';']);
        end
    case 'Temp19WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp19WPid);
        else
            x=double(statusData(:,col.Temp19WPid)); eval(['xdata=',fcts2val.Temp19WPid,';']);
        end
    case 'Temp20WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp20WPid);
        else
            x=double(statusData(:,col.Temp20WPid)); eval(['xdata=',fcts2val.Temp20WPid,';']);
        end
    case 'Temp21WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp21WPid);
        else
            x=double(statusData(:,col.Temp21WPid)); eval(['xdata=',fcts2val.Temp21WPid,';']);
        end
    case 'Temp22WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp22WPid);
        else
            x=double(statusData(:,col.Temp22WPid)); eval(['xdata=',fcts2val.Temp22WPid,';']);
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
    case 'EtaSpd'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.etaCurSpd);
        else
            x=double(statusData(:,col.etaCurSpd)); eval(['ydata=',fcts2val.etaCurSpd,';']);
        end
    case 'EtaSetPos'
        Etalonhelp=int32(statusData(:,col.etaSetPosLow));
        EtalonSetPos=(Etalonhelp)+int32(statusData(:,col.etaSetPosHigh));
        EtalonSetPos(Etalonhelp>32767)=EtalonSetPos(Etalonhelp>32767)-65535;
        ydata=EtalonSetPos;
    case 'EtaCurPos'
        Etalonhelp=int32(statusData(:,col.etaCurPosLow+2));
        EtalonCurPos=(Etalonhelp)+int32(statusData(:,col.etaCurPosHigh)); 
        EtalonCurPos(Etalonhelp>32767)=EtalonCurPos(Etalonhelp>32767)-65535;
        ydata=EtalonCurPos;
    case 'EtaEncPos'
        Etalonhelp=int32(statusData(:,col.etaEncoderPosLow)); 
        EtalonEncPos=(Etalonhelp)+int32(statusData(:,col.etaEncoderPosHigh)); 
        EtalonEncPos(Etalonhelp>32767)=EtalonEncPos(Etalonhelp>32767)-65535;
        ydata=EtalonEncPos;
    case 'EtaIndPos'
        Etalonhelp=int32(statusData(:,col.etaIndexPosLow)); 
        EtalonIndPos=(Etalonhelp)+int32(statusData(:,col.etaIndexPosHigh)); 
        EtalonIndPos(Etalonhelp>32767)=EtalonIndPos(Etalonhelp>32767)-65535;
        ydata=EtalonIndPos;
    case 'EtaOnlinePos'
        Etalonhelp=int32(statusData(:,col.etaOnlinePosLow)); 
        EtalonOnlinePos=(Etalonhelp)+int32(statusData(:,col.etaOnlinePosHigh)); 
        EtalonOnlinePos(Etalonhelp>32767)=EtalonOnlinePos(Etalonhelp>32767)-65535;
        ydata=EtalonOnlinePos;
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
    case 'GPSsecondsUTC'
        ydata=statusData(:,col.GPSsecondsUTC);
    case 'GPSLongitude'
        ydata=double(statusData(:,col.GPSLongitude))/60-180 ...
            +double(statusData(:,col.GPSLongitudeDecimals))/10000;
    case 'GPSLatitude'
        ydata=double(statusData(:,col.GPSLatitude))/60-90 ...
            +double(statusData(:,col.GPSLatitudeDecimals))/10000;
    case 'GPSAltitude'
        ydata=statusData(:,col.GPSAltitude);
    case 'GPSHDOP'
        ydata=statusData(:,col.GPSHDOP);
    case 'GPSnumSat'
        ydata=statusData(:,col.GPSnumSat);
    case 'GPSLastValidData'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.GPSLastValidData);
        else
            x=double(statusData(:,col.GPSLastValidData)); eval(['ydata=',fcts2val.GPSLastValidData,';']);
        end
    case 'GPSGroundSpeed'
        ydata=double(statusData(:,col.GPSGroundSpeed))/100;
    case 'GPSHeading'
        ydata=double(statusData(:,col.GPSHeading))/10;
    case 'TDet'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TDet);
        else
            x=double(statusData(:,col.TDet)); eval(['ydata=',fcts2val.TDet,';']);
        end
    case 'Temp0L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp0L);
        else
            x=double(statusData(:,col.Temp0L)); eval(['ydata=',fcts2val.Temp0L,';']);
        end
    case 'TempDyelaser'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempDyelaser);
        else
            x=double(statusData(:,col.TempDyelaser)); eval(['ydata=',fcts2val.TempDyelaser,';']);
        end
    case 'TempRefCell'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempRefCell);
        else
            x=double(statusData(:,col.TempRefCell)); eval(['ydata=',fcts2val.TempRefCell,';']);
        end
    case 'TempDiodeEt'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempDiodeEt);
        else
            x=double(statusData(:,col.TempDiodeEt)); eval(['ydata=',fcts2val.TempDiodeEt,';']);
        end
    case 'Temp4L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp4L);
        else
            x=double(statusData(:,col.Temp4L)); eval(['ydata=',fcts2val.Temp4L,';']);
        end
    case 'TempDiodeUV'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempDiodeUV);
        else
            x=double(statusData(:,col.TempDiodeUV)); eval(['ydata=',fcts2val.TempDiodeUV,';']);
        end
    case 'Temp6L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp6L);
        else
            x=double(statusData(:,col.Temp6L)); eval(['ydata=',fcts2val.Temp6L,';']);
        end
    case 'Temp7L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp7L);
        else
            x=double(statusData(:,col.Temp7L)); eval(['ydata=',fcts2val.Temp7L,';']);
        end
    case 'Temp8L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp8L);
        else
            x=double(statusData(:,col.Temp8L)); eval(['ydata=',fcts2val.Temp8L,';']);
        end
    case 'Temp9L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp9L);
        else
            x=double(statusData(:,col.Temp9L)); eval(['ydata=',fcts2val.Temp9L,';']);
        end
    case 'Temp10L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp10L);
        else
            x=double(statusData(:,col.Temp10L)); eval(['ydata=',fcts2val.Temp10L,';']);
        end
    case 'Temp11L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp11L);
        else
            x=double(statusData(:,col.Temp11L)); eval(['ydata=',fcts2val.Temp11L,';']);
        end
    case 'Temp12L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp12L);
        else
            x=double(statusData(:,col.Temp12L)); eval(['ydata=',fcts2val.Temp12L,';']);
        end
    case 'Temp13L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp13L);
        else
            x=double(statusData(:,col.Temp13L)); eval(['ydata=',fcts2val.Temp13L,';']);
        end
    case 'Temp14L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp14L);
        else
            x=double(statusData(:,col.Temp14L)); eval(['ydata=',fcts2val.Temp14L,';']);
        end
    case 'Temp15L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp15L);
        else
            x=double(statusData(:,col.Temp15L)); eval(['ydata=',fcts2val.Temp15L,';']);
        end
    case 'Temp16L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp16L);
        else
            x=double(statusData(:,col.Temp16L)); eval(['ydata=',fcts2val.Temp16L,';']);
        end
    case 'Temp17L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp17L);
        else
            x=double(statusData(:,col.Temp17L)); eval(['ydata=',fcts2val.Temp17L,';']);
        end
    case 'Temp18L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp18L);
        else
            x=double(statusData(:,col.Temp18L)); eval(['ydata=',fcts2val.Temp18L,';']);
        end
    case 'Temp19L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp19L);
        else
            x=double(statusData(:,col.Temp19L)); eval(['ydata=',fcts2val.Temp19L,';']);
        end
    case 'Temp20L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp20L);
        else
            x=double(statusData(:,col.Temp20L)); eval(['ydata=',fcts2val.Temp20L,';']);
        end
    case 'Temp21L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp21L);
        else
            x=double(statusData(:,col.Temp21L)); eval(['ydata=',fcts2val.Temp21L,';']);
        end
    case 'Temp22L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp22L);
        else
            x=double(statusData(:,col.Temp22L)); eval(['ydata=',fcts2val.Temp22L,';']);
        end
    case 'TempPump'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempPump);
        else
            x=double(statusData(:,col.TempPump)); eval(['ydata=',fcts2val.TempPump,';']);
        end
    case 'TempPumpOil'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempPumpOil);
        else
            x=double(statusData(:,col.TempPumpOil)); eval(['ydata=',fcts2val.TempPumpOil,';']);
        end
    case 'TempDiodeWZ1out'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempDiodeWZ1out);
        else
            x=double(statusData(:,col.TempDiodeWZ1out)); eval(['ydata=',fcts2val.TempDiodeWZ1out,';']);
        end
    case 'TempAxisPlate'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempAxisPlate);
        else
            x=double(statusData(:,col.TempAxisPlate)); eval(['ydata=',fcts2val.TempAxisPlate,';']);
        end
    case 'TempArmPS'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempArmPS);
        else
            x=double(statusData(:,col.TempArmPS)); eval(['ydata=',fcts2val.TempArmPS,';']);
        end
    case 'TempDiodeWZ1in'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempDiodeWZ1in);
        else
            x=double(statusData(:,col.TempDiodeWZ1in)); eval(['ydata=',fcts2val.TempDiodeWZ1in,';']);
        end
    case 'TempDiodeWZ2in'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempDiodeWZ2in);
        else
            x=double(statusData(:,col.TempDiodeWZ2in)); eval(['ydata=',fcts2val.TempDiodeWZ2in,';']);
        end
    case 'TempTMPSensCard'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempTMPSensCard);
        else
            x=double(statusData(:,col.TempTMPSensCard)); eval(['ydata=',fcts2val.TempTMPSensCard,';']);
        end
    case 'TempLaserSync'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempLaserSync);
        else
            x=double(statusData(:,col.TempLaserSync)); eval(['ydata=',fcts2val.TempLaserSync,';']);
        end
    case 'TempInverter'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempInverter);
        else
            x=double(statusData(:,col.TempInverter)); eval(['ydata=',fcts2val.TempInverter,';']);
        end
    case 'TempArmBackWall'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempArmBackWall);
        else
            x=double(statusData(:,col.TempArmBackWall)); eval(['ydata=',fcts2val.TempArmBackWall,';']);
        end
    case 'TempBlower'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempBlower);
        else
            x=double(statusData(:,col.TempBlower)); eval(['ydata=',fcts2val.TempBlower,';']);
        end
    case 'TempDiodeWZ2out'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempDiodeWZ2out);
        else
            x=double(statusData(:,col.TempDiodeWZ2out)); eval(['ydata=',fcts2val.TempDiodeWZ2out,';']);
        end
    case 'TempCalPlate'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempCalPlate);
        else
            x=double(statusData(:,col.TempCalPlate)); eval(['ydata=',fcts2val.TempCalPlate,';']);
        end
    case 'TempHV'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempHV);
        else
            x=double(statusData(:,col.TempHV)); eval(['ydata=',fcts2val.TempHV,';']);
        end
    case 'Temp15WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp15WP);
        else
            x=double(statusData(:,col.Temp15WP)); eval(['ydata=',fcts2val.Temp15WP,';']);
        end
    case 'Temp16WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp16WP);
        else
            x=double(statusData(:,col.Temp16WP)); eval(['ydata=',fcts2val.Temp16WP,';']);
        end
    case 'Temp17WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp17WP);
        else
            x=double(statusData(:,col.Temp17WP)); eval(['ydata=',fcts2val.Temp17WP,';']);
        end
    case 'Temp18WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp18WP);
        else
            x=double(statusData(:,col.Temp18WP)); eval(['ydata=',fcts2val.Temp18WP,';']);
        end
    case 'Temp19WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp19WP);
        else
            x=double(statusData(:,col.Temp19WP)); eval(['ydata=',fcts2val.Temp19WP,';']);
        end
    case 'Temp20WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp20WP);
        else
            x=double(statusData(:,col.Temp20WP)); eval(['ydata=',fcts2val.Temp20WP,';']);
        end
    case 'Temp21WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp21WP);
        else
            x=double(statusData(:,col.Temp21WP)); eval(['ydata=',fcts2val.Temp21WP,';']);
        end
    case 'Temp22WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp22WP);
        else
            x=double(statusData(:,col.Temp22WP)); eval(['ydata=',fcts2val.Temp22WP,';']);
        end
    case 'Temp0Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp0Lid);
        else
            x=double(statusData(:,col.Temp0Lid)); eval(['ydata=',fcts2val.Temp0Lid,';']);
        end
    case 'TempDyelaserid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempDyelaserid);
        else
            x=double(statusData(:,col.TempDyelaserid)); eval(['ydata=',fcts2val.TempDyelaserid,';']);
        end
    case 'TempRefCellid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempRefCellid);
        else
            x=double(statusData(:,col.TempRefCellid)); eval(['ydata=',fcts2val.TempRefCellid,';']);
        end
    case 'TempDiodeEtid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempDiodeEtid);
        else
            x=double(statusData(:,col.TempDiodeEtid)); eval(['ydata=',fcts2val.TempDiodeEtid,';']);
        end
    case 'Temp4Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp4Lid);
        else
            x=double(statusData(:,col.Temp4Lid)); eval(['ydata=',fcts2val.Temp4Lid,';']);
        end
    case 'TempDiodeUVid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempDiodeUVid);
        else
            x=double(statusData(:,col.TempDiodeUVid)); eval(['ydata=',fcts2val.TempDiodeUVid,';']);
        end
    case 'Temp6Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp6Lid);
        else
            x=double(statusData(:,col.Temp6Lid)); eval(['ydata=',fcts2val.Temp6Lid,';']);
        end
    case 'Temp7Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp7Lid);
        else
            x=double(statusData(:,col.Temp7Lid)); eval(['ydata=',fcts2val.Temp7Lid,';']);
        end
    case 'Temp8Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp8Lid);
        else
            x=double(statusData(:,col.Temp8Lid)); eval(['ydata=',fcts2val.Temp8Lid,';']);
        end
    case 'Temp9Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp9Lid);
        else
            x=double(statusData(:,col.Temp9Lid)); eval(['ydata=',fcts2val.Temp9Lid,';']);
        end
    case 'Temp10Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp10Lid);
        else
            x=double(statusData(:,col.Temp10Lid)); eval(['ydata=',fcts2val.Temp10Lid,';']);
        end
    case 'Temp11Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp11Lid);
        else
            x=double(statusData(:,col.Temp11Lid)); eval(['ydata=',fcts2val.Temp11Lid,';']);
        end
    case 'Temp12Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp12Lid);
        else
            x=double(statusData(:,col.Temp12Lid)); eval(['ydata=',fcts2val.Temp12Lid,';']);
        end
    case 'Temp13Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp13Lid);
        else
            x=double(statusData(:,col.Temp13Lid)); eval(['ydata=',fcts2val.Temp13Lid,';']);
        end
    case 'Temp14Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp14Lid);
        else
            x=double(statusData(:,col.Temp14Lid)); eval(['ydata=',fcts2val.Temp14Lid,';']);
        end
    case 'Temp15Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp15Lid);
        else
            x=double(statusData(:,col.Temp15Lid)); eval(['ydata=',fcts2val.Temp15Lid,';']);
        end
    case 'Temp16Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp16Lid);
        else
            x=double(statusData(:,col.Temp16Lid)); eval(['ydata=',fcts2val.Temp16Lid,';']);
        end
    case 'Temp17Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp17Lid);
        else
            x=double(statusData(:,col.Temp17Lid)); eval(['ydata=',fcts2val.Temp17Lid,';']);
        end
    case 'Temp18Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp18Lid);
        else
            x=double(statusData(:,col.Temp18Lid)); eval(['ydata=',fcts2val.Temp18Lid,';']);
        end
    case 'Temp19Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp19Lid);
        else
            x=double(statusData(:,col.Temp19Lid)); eval(['ydata=',fcts2val.Temp19Lid,';']);
        end
    case 'Temp20Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp20Lid);
        else
            x=double(statusData(:,col.Temp20Lid)); eval(['ydata=',fcts2val.Temp20Lid,';']);
        end
    case 'Temp21Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp21Lid);
        else
            x=double(statusData(:,col.Temp21Lid)); eval(['ydata=',fcts2val.Temp21Lid,';']);
        end
    case 'Temp22Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp22Lid);
        else
            x=double(statusData(:,col.Temp22Lid)); eval(['ydata=',fcts2val.Temp22Lid,';']);
        end
    case 'TempPumpid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempPumpid);
        else
            x=double(statusData(:,col.TempPumpid)); eval(['ydata=',fcts2val.TempPumpid,';']);
        end
    case 'TempPumpOilid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempPumpOilid);
        else
            x=double(statusData(:,col.TempPumpOilid)); eval(['ydata=',fcts2val.Temp1WPid,';']);
        end
    case 'TempDiodeWZ1outid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempDiodeWZ1outid);
        else
            x=double(statusData(:,col.TempDiodeWZ1outid)); eval(['ydata=',fcts2val.TempDiodeWZ1outid,';']);
        end
    case 'TempAxisPlateid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempAxisPlateid);
        else
            x=double(statusData(:,col.TempAxisPlateid)); eval(['ydata=',fcts2val.TempAxisPlateid,';']);
        end
    case 'TempArmPSid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempArmPSid);
        else
            x=double(statusData(:,col.TempArmPSid)); eval(['ydata=',fcts2val.TempArmPSid,';']);
        end
    case 'TempDiodeWZ1inid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempDiodeWZ1inid);
        else
            x=double(statusData(:,col.TempDiodeWZ1inid)); eval(['ydata=',fcts2val.TempDiodeWZ1inid,';']);
        end
    case 'TempDiodeWZ2inid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempDiodeWZ2inid);
        else
            x=double(statusData(:,col.TempDiodeWZ2inid)); eval(['ydata=',fcts2val.TempDiodeWZ2inid,';']);
        end
    case 'TempTMPSensCardid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempTMPSensCardid);
        else
            x=double(statusData(:,col.TempTMPSensCardid)); eval(['ydata=',fcts2val.TempTMPSensCardid,';']);
        end
    case 'TempLaserSyncid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempLaserSyncid);
        else
            x=double(statusData(:,col.TempLaserSyncid)); eval(['ydata=',fcts2val.TempLaserSyncid,';']);
        end
    case 'TempInverterid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempInverterid);
        else
            x=double(statusData(:,col.TempInverterid)); eval(['ydata=',fcts2val.TempInverterid,';']);
        end
    case 'TempArmBackWallid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempArmBackWallid);
        else
            x=double(statusData(:,col.TempArmBackWallid)); eval(['ydata=',fcts2val.TempArmBackWallid,';']);
        end
    case 'Temp1PumpOilid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp1PumpOilid);
        else
            x=double(statusData(:,col.TempBlowerid)); eval(['ydata=',fcts2val.TempBlowerid,';']);
        end
    case 'TempDiodeWZ2outid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempDiodeWZ2outid);
        else
            x=double(statusData(:,col.TempDiodeWZ2outid)); eval(['ydata=',fcts2val.TempDiodeWZ2outid,';']);
        end
    case 'TempCalPlateid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempCalPlateid);
        else
            x=double(statusData(:,col.TempCalPlateid)); eval(['ydata=',fcts2val.TempCalPlateid,';']);
        end
    case 'TempHVid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TempHVid);
        else
            x=double(statusData(:,col.TempHVid)); eval(['ydata=',fcts2val.TempHVid,';']);
        end
    case 'Temp15WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp15WPid);
        else
            x=double(statusData(:,col.Temp15WPid)); eval(['ydata=',fcts2val.Temp15WPid,';']);
        end
    case 'Temp16WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp16WPid);
        else
            x=double(statusData(:,col.Temp16WPid)); eval(['ydata=',fcts2val.Temp16WPid,';']);
        end
    case 'Temp17WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp17WPid);
        else
            x=double(statusData(:,col.Temp17WPid)); eval(['ydata=',fcts2val.Temp17WPid,';']);
        end
    case 'Temp18WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp18WPid);
        else
            x=double(statusData(:,col.Temp18WPid)); eval(['ydata=',fcts2val.Temp18WPid,';']);
        end
    case 'Temp19WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp19WPid);
        else
            x=double(statusData(:,col.Temp19WPid)); eval(['ydata=',fcts2val.Temp19WPid,';']);
        end
    case 'Temp20WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp20WPid);
        else
            x=double(statusData(:,col.Temp20WPid)); eval(['ydata=',fcts2val.Temp20WPid,';']);
        end
    case 'Temp21WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp21WPid);
        else
            x=double(statusData(:,col.Temp21WPid)); eval(['ydata=',fcts2val.Temp21WPid,';']);
        end
    case 'Temp22WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp22WPid);
        else
            x=double(statusData(:,col.Temp22WPid)); eval(['ydata=',fcts2val.Temp22WPid,';']);
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
    plot(handles.axes1,xdata,ydata,'.');
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
    'Ophir';'P20';'P1000';'PRef';'PDyelaser';'PVent';'PNO';'IFilament';...
    'VHV';'LaserTrigThresh';'PMTThresh';'MCP1Thresh';'MCP2Thresh';'+5VLift';'+28VLift';...
    '+3.3VLift';'+1.5VLift';'+5VarmAxis';'+28VarmAxis';'+3.3VarmAxis';'+1.5VarmAxis';...
    'EtaSpd';'EtaSetPos';'EtaCurPos';'EtaEncPos';'EtaIndPos';'EtaOnlinePos';'ValveVoltLift';...
    'Valve1VoltarmAxis';'Valve2VoltarmAxis';'MFCFlow';'MFCSetFlow';...
    'ccMasterDelayLift';'ccMasterDelayarmAxis';'ccShiftDelay0';...
    'ccCounts0';'ccPulses0';'ccShiftDelay1';'ccGateDelay1';...
    'ccGateWidth1';'ccCounts1';'ccPulses1';'ccShiftDelay2';'ccGateDelay2';...
    'ccGateWidth2';'ccCounts2';'ccPulses2';
    'GPSsecondsUTC';'GPSLongitude';'GPSLatitude';'GPSAltitude';'GPSHDOP';'GPSnumSat';...
    'GPSLastValidData';'GPSGroundSpeed';'GPSHeading';'TDet';...
    'Temp0L';'TempDyelaser';'Temp2L';'TempDiodeEt';'Temp4L';'TempDiodeUV';'Temp6L';'Temp7L';'Temp8L';...
    'Temp9L';'Temp10L';'Temp11L';'Temp12L';'Temp13L';'Temp14L';'Temp15L';'Temp16L';...
    'Temp17L';'Temp18L';'Temp19L';'Temp20L';'Temp21L';'Temp22L';...
    'TempPump';'TempPumpOil';'TempDiodeWZ1out';'TempAxisPlate';'TempArmPS';'TempDiodeWZ1in';'TempDiodeWZ2in';'TempTMPSensCard';'TempLaserSync';...
    'TempInverter';'TempArmBackWall';'TempBlower';'TempDiodeWZ2out';'TempCalPlate';'TempHV';'Temp15WP';'Temp16WP';...
    'Temp17WP';'Temp18WP';'Temp19WP';'Temp20WP';'Temp21WP';'Temp22WP';...
    'Temp0Lid';'TempDyelaserid';'TempRefCellid';'TempDiodeEtid';'Temp4Lid';'TempDiodeUVid';'Temp6Lid';'Temp7Lid';'Temp8Lid';...
    'Temp9Lid';'Temp10Lid';'Temp11Lid';'Temp12Lid';'Temp13Lid';'Temp14Lid';'Temp15Lid';'Temp16Lid';...
    'Temp17Lid';'Temp18Lid';'Temp19Lid';'Temp20Lid';'Temp21Lid';'Temp22Lid';...
    'TempPumpid';'TempPumpOilid';'TempDiodeWZ1outid';'TempAxisPlateid';'TempArmPSid';'TempDiodeWZ1inid';'TempDiodeWZ2inid';'TempTMPSensCardid';'TempLaserSyncid';...
    'TempInverterid';'TempArmBackWallid';'TempBlowerid';'TempDiodeWZ2outid';'TempCalPlateid';'TempHVid';'Temp15WPid';'Temp16WPid';...
    'Temp17WPid';'Temp18WPid';'Temp19WPid';'Temp20WPid';'Temp21WPid';'Temp22WPid'};
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
    'Ophir';'P20';'P1000';'PRef';'PDyelaser';'PVent';'PNO';'IFilament';...
    'VHV';'LaserTrigThresh';'PMTThresh';'MCP1Thresh';'MCP2Thresh';'+5VLift';'+28VLift';...
    '+3.3VLift';'+1.5VLift';'+5VarmAxis';'+28VarmAxis';'+3.3VarmAxis';'+1.5VarmAxis';...
    'EtaSpd';'EtaSetPos';'EtaCurPos';'EtaEncPos';'EtaIndPos';'EtaOnlinePos';'ValveVoltLift';...
    'Valve1VoltarmAxis';'Valve2VoltarmAxis';'MFCFlow';'MFCSetFlow';...
    'ccMasterDelayLift';'ccMasterDelayarmAxis';'ccShiftDelay0';...
    'ccCounts0';'ccPulses0';'ccShiftDelay1';'ccGateDelay1';...
    'ccGateWidth1';'ccCounts1';'ccPulses1';'ccShiftDelay2';'ccGateDelay2';...
    'ccGateWidth2';'ccCounts2';'ccPulses2';
    'GPSsecondsUTC';'GPSLongitude';'GPSLatitude';'GPSAltitude';'GPSHDOP';'GPSnumSat';...
    'GPSLastValidData';'GPSGroundSpeed';'GPSHeading';'TDet';...
    'Temp0L';'TempDyelaser';'TempRefCell';'TempDiodeEt';'Temp4L';'TempDiodeUV';'Temp6L';'Temp7L';'Temp8L';...
    'Temp9L';'Temp10L';'Temp11L';'Temp12L';'Temp13L';'Temp14L';'Temp15L';'Temp16L';...
    'Temp17L';'Temp18L';'Temp19L';'Temp20L';'Temp21L';'Temp22L';...
    'TempPump';'TempPumpOil';'TempDiodeWZ1out';'TempAxisPlate';'TempArmPS';'TempDiodeWZ1in';'TempDiodeWZ2in';'TempTMPSensCard';'TempLaserSync';...
    'TempInverter';'TempArmBackWall';'TempBlower';'TempDiodeWZ2out';'TempCalPlate';'TempHV';'Temp15WP';'Temp16WP';...
    'Temp17WP';'Temp18WP';'Temp19WP';'Temp20WP';'Temp21WP';'Temp22WP';...
    'Temp0Lid';'TempDyelaserid';'TempRefCellid';'TempDiodeEtid';'Temp4Lid';'TempDiodeUVid';'Temp6Lid';'Temp7Lid';'Temp8Lid';...
    'Temp9Lid';'Temp10Lid';'Temp11Lid';'Temp12Lid';'Temp13Lid';'Temp14Lid';'Temp15Lid';'Temp16Lid';...
    'Temp17Lid';'Temp18Lid';'Temp19Lid';'Temp20Lid';'Temp21Lid';'Temp22Lid';...
    'TempPumpid';'TempPumpOilid';'TempDiodeWZ1outid';'TempAxisPlateid';'TempArmPSid';'TempDiodeWZ1inid';'TempDiodeWZ2inid';'TempTMPSensCardid';'TempLaserSyncid';...
    'TempInverterid';'TempArmBackWallid';'TempBlowerid';'TempDiodeWZ2outid';'TempCalPlateid';'TempHVid';'Temp15WPid';'Temp16WPid';...
    'Temp17WPid';'Temp18WPid';'Temp19WPid';'Temp20WPid';'Temp21WPid';'Temp22WPid'};
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


