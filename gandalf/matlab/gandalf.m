function varargout = gandalf(varargin)
% GANDALF M-file for gandalf.fig
%      GANDALF, by itself, creates a new GANDALF or raises the existing
%      singleton*.
%
%      H = GANDALF returns the handle to a new GANDALF or the handle to
%      the existing singleton*.
%
%      GANDALF('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in GANDALF.M with the given input arguments.
%
%      GANDALF('Property','Value',...) creates a new GANDALF or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before gandalf_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to gandalf_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help gandalf

% Last Modified by GUIDE v2.5 18-Jul-2008 10:37:10

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @gandalf_OpeningFcn, ...
                   'gui_OutputFcn',  @gandalf_OutputFcn, ...
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


% --- Executes just before gandalf is made visible.
function gandalf_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to gandalf (see VARARGIN)

% Choose default command line output for gandalf
handles.output = hObject;

% choose Text size
set(double(get(handles.horus,'Children')),'FontSize',8)

%setup Timer function
%handles.ActTimer = timer('ExecutionMode','fixedDelay',...
%      'Period',0.7,...    
%      'BusyMode','drop',...
%      'TimerFcn', {@ReadStatus,handles});   
  
%data.ActTimer=handles.ActTimer;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes ADC wait for user response (see UIRESUME)
% uiwait(handles.figDataGUI);
%setappdata(handles.output, 'gandalfdata', data);
%start(handles.ActTimer);

% call function varassign.m to create structures containing column numbers
% and conversion functions for the parameters in the data files

%[data.statusData,data.AvgData]=ReadDataAvg('/gollum/ramdisk/status.bin',25,1);
%[data.col,data.fcts2val]=varassign(data.statusData);

% UIWAIT makes gandalf wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = gandalf_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;

function ReadStatus(arg1,arg2,handles)

% read data
data = getappdata(handles.output, 'gandalfdata');
        %start ReadAvgdata only if status.bin exists
        %[statusData,AvgData]=ReadDataAvg('filename',5*(time period to average in s),(min. online ref signal));
try
    [data.statusData,data.AvgData]=ReadDataAvg('/gollum/ramdisk/status.bin',25,1);
catch
    disp(['error trying to read data from status.bin: ',lasterr])
end

statusData=data.statusData;

col=data.col;
fcts2val=data.fcts2val;

% Calculate time as sum of day, hour, min, etc.
statustime=double(statusData(:,2))./1.0+ ...
           double(statusData(:,3))./24.0+...
           double(statusData(:,4))./1440.0+...
           double(statusData(:,5))./86400.0+...
           double(statusData(:,6))./86400000.0;

data.statustime=statustime;       

[SortZeit,data.indexZeit]=sort(statustime);
data.maxLen=size(statustime,1);
lastrow=data.indexZeit(data.maxLen);
data.lastrow=lastrow;

% display system time
disptime=statustime(lastrow)-double(statusData(lastrow,6))/86400000.0;
set(handles.txtTime,'String',strcat(datestr(disptime,13),'.',num2str(statusData(lastrow,6)/100)));

% check which child GUIs are active and color push buttons accordingly
% red color if warning applies to one of the values controlled in the GUI

if isfield(data,'diodelaser')
    if ishandle(data.diodelaser)
        set(handles.diodelaser,'BackgroundColor','g');
    else
        set(handles.diodelaser,'BackgroundColor','c');
    end
end

% --- Executes on button press in diodelaser.
function diodelaser_Callback(hObject, eventdata, handles)
% hObject    handle to diodelaser (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of diodelaser

data = getappdata(gcbf, 'gandalfdata');
% open Laser only if it is not already open
if ~isfield(data,'diodeLaser')
    data.diodelaser=diode_L('handle',num2str(gcbf,16));
elseif ~ishandle(str2double(data.diodelaser)) 
    data.diodeaser=diode_L('handle',num2str(gcbf,16));
end
setappdata(gcbf, 'gandalfdata', data); 


% --- Executes on button press in g_quantification.
function g_quantification_Callback(hObject, eventdata, handles)
% hObject    handle to g_quantification (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of g_quantification

data = getappdata(gcbf, 'gandalfdata');
% open quantification only if it is not already open
if ~isfield(data,'g_quantification')
    data.g_quantification=no2_quant('handle',num2str(gcbf,16));%MatLab file for PMT, diodes and such shall be called no2_quant once it exists!
elseif ~ishandle(str2double(data.g_quantification)) 
    data.g_quantification=no2quant('handle',num2str(gcbf,16));
end
setappdata(gcbf, 'gandalfdata', data); 

% --- Executes on button press in g_pressure_update.
function g_pressure_update_Callback(hObject, eventdata, handles)
% hObject    handle to g_pressure_update (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of g_pressure_update


% --- Executes on button press in g_spectrograph.
function g_spectrograph_Callback(hObject, eventdata, handles)
% hObject    handle to g_spectrograph (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of g_spectrograph

data = getappdata(gcbf, 'gandalfdata');
% open spectrograph only if it is not already open
if ~isfield(data,'g_spectrograph')
    data.g_spectrograph=Blue_Spectrometer_Gui('handle',num2str(gcbf,16));
elseif ~ishandle(str2double(data.g_spectrograph)) 
    data.g_spectrograph=Blue_Spectrometer_Gui('handle',num2str(gcbf,16));
end
setappdata(gcbf, 'gandalfdata', data); 

% --- Executes on button press in g_calibration.
function g_calibration_Callback(hObject, eventdata, handles)
% hObject    handle to g_calibration (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of g_calibration

data = getappdata(gcbf, 'gandalfdata');
% open calibration only if it is not already open
if ~isfield(data,'')
    data.g_calibration=g_cal('handle',num2str(gcbf,16));%Gandalf calibration is g_cal
elseif ~ishandle(str2double(data.g_calibration)) 
    data.g_calibration=g_cal('handle',num2str(gcbf,16));
end
setappdata(gcbf, 'gandalfdata', data); 

% --- Executes on button press in pushterminate.
function pushterminate_Callback(hObject, eventdata, handles)
% hObject    handle to pushterminate (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

%terminate gandalf
