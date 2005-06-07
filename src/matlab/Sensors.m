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
    case 'DiodeWZin' % not yet available
        xdata=single(statusData(:,col.DiodeWZout));
        xdata(:)=NaN;
    case 'DiodeWZout'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.DiodeWZout);
        else
            x=double(statusData(:,col.DiodeWZout)); eval(['xdata=',fcts2val.DiodeWZout,';']);
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
    case '+5V'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.V5V);
        else
            x=double(statusData(:,col.V5V)); eval(['xdata=',fcts2val.V5V,';']);
        end
    case '+28V'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.V28V);
        else
            x=double(statusData(:,col.V28V)); eval(['xdata=',fcts2val.V28V,';']);
        end
    case '+3.3V'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.V3_3V);
        else
            x=double(statusData(:,col.V3_3V)); eval(['xdata=',fcts2val.V3_3V,';']);
        end
    case '+1.5V'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.V1_5V);
        else
            x=double(statusData(:,col.V1_5V)); eval(['xdata=',fcts2val.V1_5V,';']);
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
    case 'ValveVolt'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.ValveVolt);
        else
            x=double(statusData(:,col.ValveVolt)); eval(['xdata=',fcts2val.ValveVolt,';']);
        end
    case 'MFCFlow'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.MFCFlow);
        else
            x=double(statusData(:,col.MFCFlow)); eval(['xdata=',fcts2val.MFCFlow,';']);
        end
    case 'TDet'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TDet);
        else
            x=double(statusData(:,col.TDet)); eval(['xdata=',fcts2val.TDet,';']);
        end
    case 'Temp0'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp0);
        else
            x=double(statusData(:,col.Temp0)); eval(['xdata=',fcts2val.Temp0,';']);
        end
    case 'TDetaxis'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.TDetaxis);
        else
            x=double(statusData(:,col.TDetaxis)); eval(['xdata=',fcts2val.TDetaxis,';']);
        end
    case 'Temp2'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp2);
        else
            x=double(statusData(:,col.Temp2)); eval(['xdata=',fcts2val.Temp2,';']);
        end
    case 'Temp3'
        if get(handles.toggleX,'Value')==0
            xdata=statusData(:,col.Temp3);
        else
            x=double(statusData(:,col.Temp3)); eval(['xdata=',fcts2val.Temp3,';']);
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
    case 'DiodeWZin' % not yet available
        ydata=single(statusData(:,col.DiodeWZout));
        ydata(:)=NaN;
    case 'DiodeWZout'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.DiodeWZout);
        else
            x=double(statusData(:,col.DiodeWZout)); eval(['ydata=',fcts2val.DiodeWZout,';']);
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
    case '+5V'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.V5V);
        else
            x=double(statusData(:,col.V5V)); eval(['ydata=',fcts2val.V5V,';']);
        end
    case '+28V'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.V28V);
        else
            x=double(statusData(:,col.V28V)); eval(['ydata=',fcts2val.V28V,';']);
        end
    case '+3.3V'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.V3_3V);
        else
            x=double(statusData(:,col.V3_3V)); eval(['ydata=',fcts2val.V3_3V,';']);
        end
    case '+1.5V'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.V1_5V);
        else
            x=double(statusData(:,col.V1_5V)); eval(['ydata=',fcts2val.V1_5V,';']);
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
    case 'ValveVolt'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.ValveVolt);
        else
            x=double(statusData(:,col.ValveVolt)); eval(['ydata=',fcts2val.ValveVolt,';']);
        end
    case 'MFCFlow'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.MFCFlow);
        else
            x=double(statusData(:,col.MFCFlow)); eval(['ydata=',fcts2val.MFCFlow,';']);
        end
    case 'TDet'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TDet);
        else
            x=double(statusData(:,col.TDet)); eval(['ydata=',fcts2val.TDet,';']);
        end
    case 'Temp0'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp0);
        else
            x=double(statusData(:,col.Temp0)); eval(['ydata=',fcts2val.Temp0,';']);
        end
    case 'TDetaxis'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.TDetaxis);
        else
            x=double(statusData(:,col.TDetaxis)); eval(['ydata=',fcts2val.TDetaxis,';']);
        end
    case 'Temp2'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp2);
        else
            x=double(statusData(:,col.Temp2)); eval(['ydata=',fcts2val.Temp2,';']);
        end
    case 'Temp3'
        if get(handles.toggleY,'Value')==0
            ydata=statusData(:,col.Temp3);
        else
            x=double(statusData(:,col.Temp3)); eval(['ydata=',fcts2val.Temp3,';']);
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
end

% display latest x and y values
set(handles.txtxvalue,'String',num2str(xdata(lastrow)));
set(handles.txtyvalue,'String',num2str(ydata(lastrow)));

xlim1=uint8(str2double(get(handles.editxlim1,'String')));
xlim2=uint8(str2double(get(handles.editxlim2,'String')));
ylim1=uint8(str2double(get(handles.editylim1,'String')));
ylim2=uint8(str2double(get(handles.editylim2,'String')));
plot(handles.axes1,xdata,ydata,'.');
xlim([xlim1*(max(xdata)-min(xdata))+min(xdata), xlim2*(max(xdata)-min(xdata))+min(xdata)]);
ylim([ylim1*(max(ydata)-min(ydata))+min(ydata), ylim2*(max(ydata)-min(ydata))+min(ydata)]);
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
vars={'Time';'DiodeGr';'DiodeUV';'DiodeEtalon';'DiodeWZin';'DiodeWZout';...
    'Ophir';'P20';'P1000';'PRef';'PDyelaser';'PVent';'PNO';'IFilament';...
    'VHV';'LaserTrigThresh';'PMTThresh';'MCP1Thresh';'MCP2Thresh';'+5V';'+28V';...
    '+3.3V';'+1.5V';'EtaSpd';'EtaSetPos';'EtaCurPos';'EtaEncPos';'EtaIndPos';'EtaOnlinePos';...
    'ValveVolt';'MFCFlow';'TDet';'Temp0';'TDetaxis';'Temp2';'Temp3';...
    'GPSsecondsUTC';'GPSLongitude';'GPSLatitude';'GPSAltitude';'GPSHDOP';'GPSnumSat';...
    'GPSLastValidData';'GPSGroundSpeed';'GPSHeading'};
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
vars={'Time';'DiodeGr';'DiodeUV';'DiodeEtalon';'DiodeWZin';'DiodeWZout';...
    'Ophir';'P20';'P1000';'PRef';'PDyelaser';'PVent';'PNO';'IFilament';...
    'VHV';'LaserTrigThresh';'PMTThresh';'MCP1Thresh';'MCP2Thresh';'+5V';'+28V';...
    '+3.3V';'+1.5V';'EtaSpd';'EtaSetPos';'EtaCurPos';'EtaEncPos';'EtaIndPos';'EtaOnlinePos';...
    'ValveVolt';'MFCFlow';'TDet';'Temp0';'TDetaxis';'Temp2';'Temp3';...
    'GPSsecondsUTC';'GPSLongitude';'GPSLatitude';'GPSAltitude';'GPSHDOP';'GPSnumSat';...
    'GPSLastValidData';'GPSGroundSpeed';'GPSHeading'};
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


