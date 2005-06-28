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
statustime=double(statusData(:,1))+ ...
           double(statusData(:,2))./1.0+ ...
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
    case 'Temp1L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp1L);
        else
            x=double(statusData(:,col.Temp1L)); eval(['xdata=',fcts2val.Temp1L,';']);
        end
    case 'Temp2L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp2L);
        else
            x=double(statusData(:,col.Temp2L)); eval(['xdata=',fcts2val.Temp2L,';']);
        end
    case 'Temp3L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp3L);
        else
            x=double(statusData(:,col.Temp3L)); eval(['xdata=',fcts2val.Temp3L,';']);
        end
    case 'Temp4L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp4L);
        else
            x=double(statusData(:,col.Temp4L)); eval(['xdata=',fcts2val.Temp4L,';']);
        end
    case 'Temp5L'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp5L);
        else
            x=double(statusData(:,col.Temp5L)); eval(['xdata=',fcts2val.Temp5L,';']);
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
    case 'Temp0WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp0WP);
        else
            x=double(statusData(:,col.Temp0WP)); eval(['xdata=',fcts2val.Temp0WP,';']);
        end
    case 'Temp1WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp1WP);
        else
            x=double(statusData(:,col.Temp1WP)); eval(['xdata=',fcts2val.Temp1WP,';']);
        end
    case 'Temp2WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp2WP);
        else
            x=double(statusData(:,col.Temp2WP)); eval(['xdata=',fcts2val.Temp2WP,';']);
        end
    case 'Temp3WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp3WP);
        else
            x=double(statusData(:,col.Temp3WP)); eval(['xdata=',fcts2val.Temp3WP,';']);
        end
    case 'Temp4WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp4WP);
        else
            x=double(statusData(:,col.Temp4WP)); eval(['xdata=',fcts2val.Temp4WP,';']);
        end
    case 'Temp5WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp5WP);
        else
            x=double(statusData(:,col.Temp5WP)); eval(['xdata=',fcts2val.Temp5WP,';']);
        end
    case 'Temp6WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp6WP);
        else
            x=double(statusData(:,col.Temp6WP)); eval(['xdata=',fcts2val.Temp6WP,';']);
        end
    case 'Temp7WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp7WP);
        else
            x=double(statusData(:,col.Temp7WP)); eval(['xdata=',fcts2val.Temp7WP,';']);
        end
    case 'Temp8WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp8WP);
        else
            x=double(statusData(:,col.Temp8WP)); eval(['xdata=',fcts2val.Temp8WP,';']);
        end
    case 'Temp9WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp9WP);
        else
            x=double(statusData(:,col.Temp9WP)); eval(['xdata=',fcts2val.Temp9WP,';']);
        end
    case 'Temp10WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp10WP);
        else
            x=double(statusData(:,col.Temp10WP)); eval(['xdata=',fcts2val.Temp10WP,';']);
        end
    case 'Temp11WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp11WP);
        else
            x=double(statusData(:,col.Temp11WP)); eval(['xdata=',fcts2val.Temp11WP,';']);
        end
    case 'Temp12WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp12WP);
        else
            x=double(statusData(:,col.Temp12WP)); eval(['xdata=',fcts2val.Temp12WP,';']);
        end
    case 'Temp13WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp13WP);
        else
            x=double(statusData(:,col.Temp13WP)); eval(['xdata=',fcts2val.Temp13WP,';']);
        end
    case 'Temp14WP'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp14WP);
        else
            x=double(statusData(:,col.Temp14WP)); eval(['xdata=',fcts2val.Temp14WP,';']);
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
    case 'Temp1Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp1Lid);
        else
            x=double(statusData(:,col.Temp1Lid)); eval(['xdata=',fcts2val.Temp1Lid,';']);
        end
    case 'Temp2Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp2Lid);
        else
            x=double(statusData(:,col.Temp2Lid)); eval(['xdata=',fcts2val.Temp2Lid,';']);
        end
    case 'Temp3Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp3Lid);
        else
            x=double(statusData(:,col.Temp3Lid)); eval(['xdata=',fcts2val.Temp3Lid,';']);
        end
    case 'Temp4Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp4Lid);
        else
            x=double(statusData(:,col.Temp4Lid)); eval(['xdata=',fcts2val.Temp4Lid,';']);
        end
    case 'Temp5Lid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp5Lid);
        else
            x=double(statusData(:,col.Temp5Lid)); eval(['xdata=',fcts2val.Temp5Lid,';']);
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
    case 'Temp0WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp0WPid);
        else
            x=double(statusData(:,col.Temp0WPid)); eval(['xdata=',fcts2val.Temp0WPid,';']);
        end
    case 'Temp1WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp1WPid);
        else
            x=double(statusData(:,col.Temp1WPid)); eval(['xdata=',fcts2val.Temp1WPid,';']);
        end
    case 'Temp2WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp2WPid);
        else
            x=double(statusData(:,col.Temp2WPid)); eval(['xdata=',fcts2val.Temp2WPid,';']);
        end
    case 'Temp3WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp3WPid);
        else
            x=double(statusData(:,col.Temp3WPid)); eval(['xdata=',fcts2val.Temp3WPid,';']);
        end
    case 'Temp4WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp4WPid);
        else
            x=double(statusData(:,col.Temp4WPid)); eval(['xdata=',fcts2val.Temp4WPid,';']);
        end
    case 'Temp5WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp5WPid);
        else
            x=double(statusData(:,col.Temp5WPid)); eval(['xdata=',fcts2val.Temp5WPid,';']);
        end
    case 'Temp6WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp6WPid);
        else
            x=double(statusData(:,col.Temp6WPid)); eval(['xdata=',fcts2val.Temp6WPid,';']);
        end
    case 'Temp7WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp7WPid);
        else
            x=double(statusData(:,col.Temp7WPid)); eval(['xdata=',fcts2val.Temp7WPid,';']);
        end
    case 'Temp8WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp8WPid);
        else
            x=double(statusData(:,col.Temp8WPid)); eval(['xdata=',fcts2val.Temp8WPid,';']);
        end
    case 'Temp9WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp9WPid);
        else
            x=double(statusData(:,col.Temp9WPid)); eval(['xdata=',fcts2val.Temp9WPid,';']);
        end
    case 'Temp10WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp10WPid);
        else
            x=double(statusData(:,col.Temp10WPid)); eval(['xdata=',fcts2val.Temp10WPid,';']);
        end
    case 'Temp11WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp11WPid);
        else
            x=double(statusData(:,col.Temp11WPid)); eval(['xdata=',fcts2val.Temp11WPid,';']);
        end
    case 'Temp12WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp12WPid);
        else
            x=double(statusData(:,col.Temp12WPid)); eval(['xdata=',fcts2val.Temp12WPid,';']);
        end
    case 'Temp13WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp13WPid);
        else
            x=double(statusData(:,col.Temp13WPid)); eval(['xdata=',fcts2val.Temp13WPid,';']);
        end
    case 'Temp14WPid'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp14WPid);
        else
            x=double(statusData(:,col.Temp14WPid)); eval(['xdata=',fcts2val.Temp14WPid,';']);
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
    case 'Temp1L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp1L);
        else
            x=double(statusData(:,col.Temp1L)); eval(['ydata=',fcts2val.Temp1L,';']);
        end
    case 'Temp2L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp2L);
        else
            x=double(statusData(:,col.Temp2L)); eval(['ydata=',fcts2val.Temp2L,';']);
        end
    case 'Temp3L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp3L);
        else
            x=double(statusData(:,col.Temp3L)); eval(['ydata=',fcts2val.Temp3L,';']);
        end
    case 'Temp4L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp4L);
        else
            x=double(statusData(:,col.Temp4L)); eval(['ydata=',fcts2val.Temp4L,';']);
        end
    case 'Temp5L'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp5L);
        else
            x=double(statusData(:,col.Temp5L)); eval(['ydata=',fcts2val.Temp5L,';']);
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
    case 'Temp0WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp0WP);
        else
            x=double(statusData(:,col.Temp0WP)); eval(['ydata=',fcts2val.Temp0WP,';']);
        end
    case 'Temp1WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp1WP);
        else
            x=double(statusData(:,col.Temp1WP)); eval(['ydata=',fcts2val.Temp1WP,';']);
        end
    case 'Temp2WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp2WP);
        else
            x=double(statusData(:,col.Temp2WP)); eval(['ydata=',fcts2val.Temp2WP,';']);
        end
    case 'Temp3WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp3WP);
        else
            x=double(statusData(:,col.Temp3WP)); eval(['ydata=',fcts2val.Temp3WP,';']);
        end
    case 'Temp4WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp4WP);
        else
            x=double(statusData(:,col.Temp4WP)); eval(['ydata=',fcts2val.Temp4WP,';']);
        end
    case 'Temp5WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp5WP);
        else
            x=double(statusData(:,col.Temp5WP)); eval(['ydata=',fcts2val.Temp5WP,';']);
        end
    case 'Temp6WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp6WP);
        else
            x=double(statusData(:,col.Temp6WP)); eval(['ydata=',fcts2val.Temp6WP,';']);
        end
    case 'Temp7WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp7WP);
        else
            x=double(statusData(:,col.Temp7WP)); eval(['ydata=',fcts2val.Temp7WP,';']);
        end
    case 'Temp8WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp8WP);
        else
            x=double(statusData(:,col.Temp8WP)); eval(['ydata=',fcts2val.Temp8WP,';']);
        end
    case 'Temp9WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp9WP);
        else
            x=double(statusData(:,col.Temp9WP)); eval(['ydata=',fcts2val.Temp9WP,';']);
        end
    case 'Temp10WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp10WP);
        else
            x=double(statusData(:,col.Temp10WP)); eval(['ydata=',fcts2val.Temp10WP,';']);
        end
    case 'Temp11WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp11WP);
        else
            x=double(statusData(:,col.Temp11WP)); eval(['ydata=',fcts2val.Temp11WP,';']);
        end
    case 'Temp12WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp12WP);
        else
            x=double(statusData(:,col.Temp12WP)); eval(['ydata=',fcts2val.Temp12WP,';']);
        end
    case 'Temp13WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp13WP);
        else
            x=double(statusData(:,col.Temp13WP)); eval(['ydata=',fcts2val.Temp13WP,';']);
        end
    case 'Temp14WP'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp14WP);
        else
            x=double(statusData(:,col.Temp14WP)); eval(['ydata=',fcts2val.Temp14WP,';']);
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
    case 'Temp1Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp1Lid);
        else
            x=double(statusData(:,col.Temp1Lid)); eval(['ydata=',fcts2val.Temp1Lid,';']);
        end
    case 'Temp2Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp2Lid);
        else
            x=double(statusData(:,col.Temp2Lid)); eval(['ydata=',fcts2val.Temp2Lid,';']);
        end
    case 'Temp3Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp3Lid);
        else
            x=double(statusData(:,col.Temp3Lid)); eval(['ydata=',fcts2val.Temp3Lid,';']);
        end
    case 'Temp4Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp4Lid);
        else
            x=double(statusData(:,col.Temp4Lid)); eval(['ydata=',fcts2val.Temp4Lid,';']);
        end
    case 'Temp5Lid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp5Lid);
        else
            x=double(statusData(:,col.Temp5Lid)); eval(['ydata=',fcts2val.Temp5Lid,';']);
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
    case 'Temp0WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp0WPid);
        else
            x=double(statusData(:,col.Temp0WPid)); eval(['ydata=',fcts2val.Temp0WPid,';']);
        end
    case 'Temp1WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp1WPid);
        else
            x=double(statusData(:,col.Temp1WPid)); eval(['ydata=',fcts2val.Temp1WPid,';']);
        end
    case 'Temp2WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp2WPid);
        else
            x=double(statusData(:,col.Temp2WPid)); eval(['ydata=',fcts2val.Temp2WPid,';']);
        end
    case 'Temp3WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp3WPid);
        else
            x=double(statusData(:,col.Temp3WPid)); eval(['ydata=',fcts2val.Temp3WPid,';']);
        end
    case 'Temp4WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp4WPid);
        else
            x=double(statusData(:,col.Temp4WPid)); eval(['ydata=',fcts2val.Temp4WPid,';']);
        end
    case 'Temp5WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp5WPid);
        else
            x=double(statusData(:,col.Temp5WPid)); eval(['ydata=',fcts2val.Temp5WPid,';']);
        end
    case 'Temp6WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp6WPid);
        else
            x=double(statusData(:,col.Temp6WPid)); eval(['ydata=',fcts2val.Temp6WPid,';']);
        end
    case 'Temp7WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp7WPid);
        else
            x=double(statusData(:,col.Temp7WPid)); eval(['ydata=',fcts2val.Temp7WPid,';']);
        end
    case 'Temp8WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp8WPid);
        else
            x=double(statusData(:,col.Temp8WPid)); eval(['ydata=',fcts2val.Temp8WPid,';']);
        end
    case 'Temp9WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp9WPid);
        else
            x=double(statusData(:,col.Temp9WPid)); eval(['ydata=',fcts2val.Temp9WPid,';']);
        end
    case 'Temp10WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp10WPid);
        else
            x=double(statusData(:,col.Temp10WPid)); eval(['ydata=',fcts2val.Temp10WPid,';']);
        end
    case 'Temp11WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp11WPid);
        else
            x=double(statusData(:,col.Temp11WPid)); eval(['ydata=',fcts2val.Temp11WPid,';']);
        end
    case 'Temp12WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp12WPid);
        else
            x=double(statusData(:,col.Temp12WPid)); eval(['ydata=',fcts2val.Temp12WPid,';']);
        end
    case 'Temp13WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp13WPid);
        else
            x=double(statusData(:,col.Temp13WPid)); eval(['ydata=',fcts2val.Temp13WPid,';']);
        end
    case 'Temp14WPid'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp14WPid);
        else
            x=double(statusData(:,col.Temp14WPid)); eval(['ydata=',fcts2val.Temp14WPid,';']);
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
set(handles.txtxvalue,'String',num2str(xdata(lastrow)));
set(handles.txtyvalue,'String',num2str(ydata(lastrow)));

xlim1=double(str2double(get(handles.editxlim1,'String')));
xlim2=double(str2double(get(handles.editxlim2,'String')));
ylim1=double(str2double(get(handles.editylim1,'String')));
ylim2=double(str2double(get(handles.editylim2,'String')));
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
    'Valve1VoltarmAxis';'Valve2VoltarmAxis';'MFCFlow';...
    'GPSsecondsUTC';'GPSLongitude';'GPSLatitude';'GPSAltitude';'GPSHDOP';'GPSnumSat';...
    'GPSLastValidData';'GPSGroundSpeed';'GPSHeading';'TDet';...
    'Temp0L';'Temp1L';'Temp2L';'Temp3L';'Temp4L';'Temp5L';'Temp6L';'Temp7L';'Temp8L';...
    'Temp9L';'Temp10L';'Temp11L';'Temp12L';'Temp13L';'Temp14L';'Temp15L';'Temp16L';...
    'Temp17L';'Temp18L';'Temp19L';'Temp20L';'Temp21L';'Temp22L';...
    'Temp0WP';'Temp1WP';'Temp2WP';'Temp3WP';'Temp4WP';'Temp5WP';'Temp6WP';'Temp7WP';'Temp8WP';...
    'Temp9WP';'Temp10WP';'Temp11WP';'Temp12WP';'Temp13WP';'Temp14WP';'Temp15WP';'Temp16WP';...
    'Temp17WP';'Temp18WP';'Temp19WP';'Temp20WP';'Temp21WP';'Temp22WP';...
    'Temp0Lid';'Temp1Lid';'Temp2Lid';'Temp3Lid';'Temp4Lid';'Temp5Lid';'Temp6Lid';'Temp7Lid';'Temp8Lid';...
    'Temp9Lid';'Temp10Lid';'Temp11Lid';'Temp12Lid';'Temp13Lid';'Temp14Lid';'Temp15Lid';'Temp16Lid';...
    'Temp17Lid';'Temp18Lid';'Temp19Lid';'Temp20Lid';'Temp21Lid';'Temp22Lid';...
    'Temp0WPid';'Temp1WPid';'Temp2WPid';'Temp3WPid';'Temp4WPid';'Temp5WPid';'Temp6WPid';'Temp7WPid';'Temp8WPid';...
    'Temp9WPid';'Temp10WPid';'Temp11WPid';'Temp12WPid';'Temp13WPid';'Temp14WPid';'Temp15WPid';'Temp16WPid';...
    'Temp17WPid';'Temp18WPid';'Temp19WPid';'Temp20WPid';'Temp21WPid';'Temp22WPid'};
fset(hObject,'String',vars);


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
    'Valve1VoltarmAxis';'Valve2VoltarmAxis';'MFCFlow';...
    'GPSsecondsUTC';'GPSLongitude';'GPSLatitude';'GPSAltitude';'GPSHDOP';'GPSnumSat';...
    'GPSLastValidData';'GPSGroundSpeed';'GPSHeading';'TDet';...
    'Temp0L';'Temp1L';'Temp2L';'Temp3L';'Temp4L';'Temp5L';'Temp6L';'Temp7L';'Temp8L';...
    'Temp9L';'Temp10L';'Temp11L';'Temp12L';'Temp13L';'Temp14L';'Temp15L';'Temp16L';...
    'Temp17L';'Temp18L';'Temp19L';'Temp20L';'Temp21L';'Temp22L';...
    'Temp0WP';'Temp1WP';'Temp2WP';'Temp3WP';'Temp4WP';'Temp5WP';'Temp6WP';'Temp7WP';'Temp8WP';...
    'Temp9WP';'Temp10WP';'Temp11WP';'Temp12WP';'Temp13WP';'Temp14WP';'Temp15WP';'Temp16WP';...
    'Temp17WP';'Temp18WP';'Temp19WP';'Temp20WP';'Temp21WP';'Temp22WP';...
    'Temp0Lid';'Temp1Lid';'Temp2Lid';'Temp3Lid';'Temp4Lid';'Temp5Lid';'Temp6Lid';'Temp7Lid';'Temp8Lid';...
    'Temp9Lid';'Temp10Lid';'Temp11Lid';'Temp12Lid';'Temp13Lid';'Temp14Lid';'Temp15Lid';'Temp16Lid';...
    'Temp17Lid';'Temp18Lid';'Temp19Lid';'Temp20Lid';'Temp21Lid';'Temp22Lid';...
    'Temp0WPid';'Temp1WPid';'Temp2WPid';'Temp3WPid';'Temp4WPid';'Temp5WPid';'Temp6WPid';'Temp7WPid';'Temp8WPid';...
    'Temp9WPid';'Temp10WPid';'Temp11WPid';'Temp12WPid';'Temp13WPid';'Temp14WPid';'Temp15WPid';'Temp16WPid';...
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


