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

% Last Modified by GUIDE v2.5 08-Feb-2005 16:18:14

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

% load data, otherwise timer makes error
data = getappdata(handles.output, 'Sensdata');

horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
AvgData=horusdata.AvgData;

% Calculate time as sum of day, hour, min, etc.
statustime=double(statusData(:,1))+ ...
           double(statusData(:,2))./1.0+ ...
           double(statusData(:,3))./24.0+...
           double(statusData(:,4))./1440.0+...
           double(statusData(:,5))./86400.0;

% Calculate parameter values

ccADCBase=7;
ADCBase0=656;
ADCBase1=689;
TempBase=729;
EtalonBase=643; 

DiodeUV=4.6863E-6*double(statusData(:,ADCBase0+2*3)).^2-8.5857E-2*double(statusData(:,ADCBase0+2*3))+390.41;
DiodeWZout=(double(statusData(:,ADCBase1+5*3))-9790.0)/193.2836;
P20=single(statusData(:,ADCBase1+1*3))*0.00891-89.55;
TDet=single(statusData(:,TempBase+7))./100-273.15;

help=int32(statusData(:,EtalonBase));
EtalonSetPos=(help)+int32(statusData(:,EtalonBase+1)); EtalonSetPos(help>32767)=EtalonSetPos(help>32767)-65535;

help=int32(statusData(:,EtalonBase+2));
EtalonCurPos=(help)+int32(statusData(:,EtalonBase+3)); EtalonCurPos(help>32767)=EtalonCurPos(help>32767)-65535;

help=int32(statusData(:,EtalonBase+4));
EtalonEncPos=(help)+int32(statusData(:,EtalonBase+5)); EtalonEncPos(help>32767)=EtalonEncPos(help>32767)-65535;

help=int32(statusData(:,EtalonBase+6));
EtalonIndPos=(help)+int32(statusData(:,EtalonBase+7)); EtalonIndPos(help>32767)=EtalonIndPos(help>32767)-65535;

EtalonSpeed=statusData(:,EtalonBase+10);
EtalonStatus=statusData(:,EtalonBase+11);

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
        xdata=statusData(:,ADCBase0+5*3);
    case 'DiodeUV'
        if get(handles.toggleX,'Value')
            xdata=statusData(:,ADCBase0+2*3);
        else
            xdata=DiodeUV;
        end
    case 'DiodeEtalon'
        xdata=statusData(:,ADCBase0+4*3);
    case 'DiodeWZin' % not yet available
        xdata=statusData(:,ADCBase1+5*3);
        xdata(:)=NaN;
    case 'DiodeWZout'
        if get(handles.toggleX,'Value')
            xdata=statusData(:,ADCBase1+5*3);
        else
            xdata=DiodeWZout;
        end
    case 'Ophir'
        xdata=statusData(:,ADCBase1+7*3);
    case 'P20'
        if get(handles.toggleX,'Value')
            xdata=statusData(:,ADCBase1+1*3);
        else
            xdata=P20;
        end
    case 'P1000'
        xdata=single(statusData(:,ADCBase1));
    case 'PRef'
        xdata=statusData(:,ADCBase0+3*3);
    case 'PDyelaser'
        xdata=statusData(:,ADCBase0+1*3);
    case 'PVent'
        xdata=statusData(:,ADCBase0+7*3);
    case 'PNO'
        xdata=statusData(:,ADCBase1+2*3);
    case 'IFilament'
        xdata=statusData(:,ADCBase1+6*3);
    case 'TDet'
        if get(handles.toggleX,'Value')
            xdata=statusData(:,TempBase+7);
        else
            xdata=TDet;
        end
    case 'VHV'
        xdata=statusData(:,ADCBase1+3*3);
    case 'PMTThr'
        xdata=statusData(:,ccADCBase+0);
    case 'LastTrigThr'
        xdata=statusData(:,ccADCBase+1);
    case 'MCP1Thr'
        xdata=statusData(:,ccADCBase+2);
    case 'MCP2Thr'
        xdata=statusData(:,ccADCBase+3);
    case '+5V'
        xdata=statusData(:,ccADCBase+4);
    case '+28V'
        xdata=statusData(:,ccADCBase+5);
    case '+3.3V'
        xdata=statusData(:,ccADCBase+6);
    case '+1.5V'
        xdata=statusData(:,ccADCBase+7);
    case 'EtaSpd'
        xdata=EtalonSpeed;
    case 'EtaSetPos'
        xdata=EtalonSetPos;
    case 'EtaCurPos'
        xdata=EtalonCurPos;
    case 'EtaEncPos'
        xdata=EtalonEncPos;
    case 'EtaIndPos'
        xdata=EtalonIndPos;
end
        
% assign data to y-axis
switch char(ypar)
    case 'Time'
        ydata=statustime;
    case 'DiodeGr'
        ydata=statusData(:,ADCBase0+5*3);
    case 'DiodeUV'
        if get(handles.toggleY,'Value')
            ydata=statusData(:,ADCBase0+2*3);
        else
            ydata=DiodeUV;
        end
    case 'DiodeEtalon'
        ydata=statusData(:,ADCBase0+4*3);
    case 'DiodeWZin' % not yet available
        ydata=statusData(:,ADCBase1+5*3);
        ydata(:)=NaN;
    case 'DiodeWZout'
        if get(handles.toggleY,'Value')
            ydata=statusData(:,ADCBase1+5*3);
        else
            ydata=DiodeWZout;
        end
    case 'Ophir'
        ydata=statusData(:,ADCBase1+7*3);
    case 'P20'
        if get(handles.toggleY,'Value')
            ydata=statusData(:,ADCBase1+1*3);
        else
            ydata=P20;
        end
    case 'P1000'
        ydata=single(statusData(:,ADCBase1));
    case 'PRef'
        ydata=statusData(:,ADCBase0+3*3);
    case 'PDyelaser'
        ydata=statusData(:,ADCBase0+1*3);
    case 'PVent'
        ydata=statusData(:,ADCBase0+7*3);
    case 'PNO'
        ydata=statusData(:,ADCBase1+2*3);
    case 'IFilament'
        ydata=statusData(:,ADCBase1+6*3);
    case 'TDet'
        if get(handles.toggleY,'Value')
            ydata=statusData(:,TempBase+7);
        else
            ydata=TDet;
        end
    case 'VHV'
        ydata=statusData(:,ADCBase1+3*3);
    case 'PMTThr'
        ydata=statusData(:,ccADCBase+0);
    case 'LastTrigThr'
        ydata=statusData(:,ccADCBase+1);
    case 'MCP1Thr'
        ydata=statusData(:,ccADCBase+2);
    case 'MCP2Thr'
        ydata=statusData(:,ccADCBase+3);
    case '+5V'
        ydata=statusData(:,ccADCBase+4);
    case '+28V'
        ydata=statusData(:,ccADCBase+5);
    case '+3.3V'
        ydata=statusData(:,ccADCBase+6);
    case '+1.5V'
        ydata=statusData(:,ccADCBase+7);
    case 'EtaSpd'
        ydata=EtalonSpeed;
    case 'EtaSetPos'
        ydata=EtalonSetPos;
    case 'EtaCurPos'
        ydata=EtalonCurPos;
    case 'EtaEncPos'
        ydata=EtalonEncPos;
    case 'EtaIndPos'
        ydata=EtalonIndPos;
end

plot(handles.axes1,xdata,ydata,'.');
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
%       See ISPC and COMPUTER.
set(hObject,'BackgroundColor','white');
vars={'Time';'DiodeGr';'DiodeUV';'DiodeEtalon';'DiodeWZin';'DiodeWZout';...
    'Ophir';'P20';'P1000';'PRef';'PDyelaser';'PVent';'PNO';'IFilament';...
    'TDet';'VHV';'PMTThr';'LastTrigThr';'MCP1Thr';'MCP2Thr';'+5V';'+28V';...
    '+3.3V';'+1.5V';'EtaSpd';'EtaSetPos';'EtaCurPos';'EtaEncPos';'EtaIndPos'};
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
    'TDet';'VHV';'PMTThr';'LastTrigThr';'MCP1Thr';'MCP2Thr';'+5V';'+28V';...
    '+3.3V';'+1.5V';'EtaSpd';'EtaSetPos';'EtaCurPos';'EtaEncPos';'EtaIndPos'};
set(hObject,'String',vars);




% --- Executes on button press in togglebutton1.
function toggleX_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton1
if get(hObject,'Value') 
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
if get(hObject,'Value') 
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


