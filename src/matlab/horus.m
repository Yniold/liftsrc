function varargout = horus(varargin)
% HORUS M-file for horus.fig
%      HORUS, by itself, creates a new HORUS or raises the existing
%      singleton*.
%
%      H = HORUS returns the handle to a new HORUS or the handle to
%      the existing singleton*.
%
%      HORUS('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in HORUS.M with the given input arguments.
%
%      HORUS('Property','Value',...) creates a new HORUS or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before horus_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to horus_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Copyright 2002-2003 The MathWorks, Inc.

% Edit the above text to modify the response to help horus

% Last Modified by GUIDE v2.5 28-Jun-2005 18:12:53

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @horus_OpeningFcn, ...
                   'gui_OutputFcn',  @horus_OutputFcn, ...
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






% --- Executes just before horus is made visible.
function horus_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to horus (see VARARGIN)

% Choose default command line output for horus
handles.output = hObject;

%setup Timer function
handles.ActTimer = timer('ExecutionMode','fixedDelay',...
      'Period',0.7,...    
      'BusyMode','drop',...
      'TimerFcn', {@ReadStatus,handles});   
  
data.ActTimer=handles.ActTimer;

% call function varassign.m to create structures containing column numbers
% and conversion functions for the parameters in the data files
[data.col,data.fcts2val]=varassign;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes ADC wait for user response (see UIRESUME)
% uiwait(handles.figDataGUI);
setappdata(handles.output, 'horusdata', data);
start(handles.ActTimer);




% --- Outputs from this function are returned to the command line.
function varargout = horus_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;







function ReadStatus(arg1,arg2,handles)

% read data
data = getappdata(handles.output, 'horusdata');
try     % start ReadAvgdata only if status.bin exists
    %[statusData,AvgData]=ReadDataAvg('filename',5*(time period to average in s),(min. online ref signal));
    [data.statusData,data.AvgData]=ReadDataAvg('/lift/ramdisk/status.bin',50,500);
catch
    disp(['error trying to read data from status.bin: ',lasterr])
end

statusData=data.statusData;

col=data.col;
%fcts2val=data.fcts2val;

statustime=double(statusData(:,1))+ ...
           double(statusData(:,2))./1.0+ ...
           double(statusData(:,3))./24.0+...
           double(statusData(:,4))./1440.0+...
           double(statusData(:,5))./86400.0;

[SortZeit,indexZeit]=sort(statustime);
maxLen=size(statustime,1);
lastrow=indexZeit(maxLen);

% show if armAxis is working and switch on LED on armAxis
if statusData(lastrow,col.ValidSlaveDataFlag) % if armAxis is active
    set(handles.txtarmAxis,'BackgroundColor','g','String','armAxis is ON');
    if bitget(statusData(lastrow,col.Valve2armAxis),14)==0  % if LED is off
        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),14);
        system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
        system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
    end
else
    set(handles.txtarmAxis,'BackgroundColor','r','String','armAxis is OFF');                
end

% switch off filament if reference cell pressure is too high
if bitget(statusData(lastrow,col.ValveLift),14)==1; %if filament is on
    if statusData(lastrow,col.PRef)>10500 % check if pressure in reference cell is too high
        Valveword=bitset(statusData(lastrow,col.ValveLift),14,0);
        system(['/lift/bin/eCmd @Lift w 0xa408 ', num2str(Valveword)]);
    end
end


% check which child GUIs are active and color push buttons accordingly
if isfield(data,'hCounterCards')
    if ishandle(str2double(data.hCounterCards)) 
        set(handles.CounterCards,'BackgroundColor','g');
    else
        set(handles.CounterCards,'BackgroundColor','c');
    end
end
if isfield(data,'hDyelaser')
    if ishandle(str2double(data.hDyelaser)) 
        set(handles.Dyelaser,'BackgroundColor','g');
    else
        set(handles.Dyelaser,'BackgroundColor','c');
    end
end
if isfield(data,'hLaser')
    if ishandle(str2double(data.hLaser)) 
        set(handles.Laser,'BackgroundColor','g');
    else
        set(handles.Laser,'BackgroundColor','c');
    end
end
if isfield(data,'hDetection')
    if ishandle(str2double(data.hDetection)) 
        set(handles.Detection,'BackgroundColor','g');
    else
        set(handles.Detection,'BackgroundColor','c');
    end
end
if isfield(data,'hSensors')
    if ishandle(str2double(data.hSensors)) 
        set(handles.Sensors,'BackgroundColor','g');
    else
        set(handles.Sensors,'BackgroundColor','c');
    end
end

data.lastrow=lastrow;
setappdata(handles.output, 'horusdata', data);








% --- Executes on button press in CounterCards.
function CounterCards_Callback(hObject, eventdata, handles)
% hObject    handle to CounterCards (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
data = getappdata(gcbf, 'horusdata');
% open CounterCards only if it is not already open and if Detection is open
if isfield(data,'hDetection')
    if ishandle(str2double(data.hDetection))
        if ~isfield(data,'hCounterCards')
            handleCounterCards=CounterCards('handle',num2str(gcbf,16));
            data.hCounterCards=num2str(handleCounterCards,16);
        elseif ~ishandle(str2double(data.hCounterCards))
            handleCounterCards=CounterCards('handle',num2str(gcbf,16));
            data.hCounterCards=num2str(handleCounterCards,16);
        end
    end
end
setappdata(gcbf, 'horusdata', data); 








% --- Executes on button press in Exit.
function Exit_Callback(hObject, eventdata, handles)
% hObject    handle to Exit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
data = getappdata(gcbf, 'horusdata');
statusData=data.statusData;
col=data.col;
lastrow=data.lastrow;

stop(handles.ActTimer);

% shut Filament and Laser Valves Off
system('/lift/bin/eCmd @Lift w 0xa408 0x0000');

if statusData(lastrow,col.ValidSlaveDataFlag)    % shut Axis Valves Off
    system(['/lift/bin/eCmd @armAxis w 0xa408 0x0000']);
    % shut HV Off
    Valveword=bitset(statusData(lastrow,col.Valve2armAxis),8,0);  % switch HV off
    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch HV
    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
    % switch Gain off for MCP1
    word=bitset(statusData(lastrow,col.ccGateDelay1),16,0);
    system(['/lift/bin/eCmd @armAxis w 0xa318 ',num2str(word)]);
    % switch Gain off for MCP2
    word=bitset(statusData(lastrow,col.ccGateDelay2),16,0);
    system(['/lift/bin/eCmd @armAxis w 0xa31c ',num2str(word)]);
    % switch LED off
    Valveword=bitset(statusData(lastrow,col.Valve2armAxis),14,0);
    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
end
% home Etalon 
system('/lift/bin/eCmd @Lift w 0xa510 0');

% close child GUIs

if isfield(data,'hDyelaser')
    hDyelaser=str2double(data.hDyelaser);
    if ishandle(hDyelaser), 
        Dyelaserdata = getappdata(hDyelaser, 'Dyelaserdata');
        if isfield(Dyelaserdata,'ActTimer')
            stop(Dyelaserdata.ActTimer);
            delete(Dyelaserdata.ActTimer);
        end
        close(hDyelaser); 
    end
end

if isfield(data,'hCounterCards')
    hCounterCards=str2double(data.hCounterCards);
    if ishandle(hCounterCards)
        Gatedata = getappdata(hCounterCards, 'Gatedata');
        if isfield(Gatedata,'Timer')
            stop(Gatedata.Timer);
            delete(Gatedata.Timer);
        end
        close(hCounterCards); 
    end
end

if isfield(data,'hLaser')
    hLaser=str2double(data.hLaser);
    if ishandle(hLaser), 
        Laserdata = getappdata(hLaser, 'tcpdata');
        fclose(Laserdata.tport);
        delete(Laserdata.tport);
        echotcpip('off');
        close(hLaser); 
    end
end

if isfield(data,'hDetection')
    hDetection=str2double(data.hDetection);
    if ishandle(hDetection), 
        Detdata = getappdata(hDetection, 'Detdata');
        if isfield(Detdata,'Timer')
            stop(Detdata.Timer);
            delete(Detdata.Timer);
        end
        close(hDetection); 
    end
end

if isfield(data,'hSensors')
    hSensors=str2double(data.hSensors);
    if ishandle(hSensors), 
        Sensdata = getappdata(hSensors, 'Sensdata');
        if isfield(Sensdata,'Timer')
            stop(Sensdata.Timer);
            delete(Sensdata.Timer);
        end
        close(hSensors); 
    end
end

delete(handles.ActTimer);
close(gcbf);






% --- Executes on button press in Dyelaser.
function Dyelaser_Callback(hObject, eventdata, handles)
% hObject    handle to Dyelaser (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
data = getappdata(gcbf, 'horusdata');
% open Dyelaser only if it is not already open
if ~isfield(data,'hDyelaser')
    handleDyelaser=Dyelaser('handle',num2str(gcbf,16));
    data.hDyelaser=num2str(handleDyelaser,16);
elseif ~ishandle(str2double(data.hDyelaser)) 
    handleDyelaser=Dyelaser('handle',num2str(gcbf,16));
    data.hDyelaser=num2str(handleDyelaser,16);
end
setappdata(gcbf, 'horusdata', data); 







% --- Executes on button press in Laser.
function Laser_Callback(hObject, eventdata, handles)
% hObject    handle to Laser (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
data = getappdata(gcbf, 'horusdata');
% open Laser only if it is not already open
if ~isfield(data,'hLaser')
    handleLaser=Laser('handle',num2str(gcbf,16));
    data.hLaser=num2str(handleLaser,16);
elseif ~ishandle(str2double(data.hLaser)) 
    handleLaser=Laser('handle',num2str(gcbf,16));
    data.hLaser=num2str(handleLaser,16);
end
setappdata(gcbf, 'horusdata', data); 






% --- Executes on button press in Detection.
function Detection_Callback(hObject, eventdata, handles)
% hObject    handle to Detection (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of Detection
data = getappdata(gcbf, 'horusdata');
% open Detection only if it is not already open
if ~isfield(data,'hDetection')
    handleDetection=Detection('handle',num2str(gcbf,16));
    data.hDetection=num2str(handleDetection,16);
elseif ~ishandle(str2double(data.hDetection)) 
    handleDetection=Detection('handle',num2str(gcbf,16));
    data.hDetection=num2str(handleDetection,16);
end
setappdata(gcbf, 'horusdata', data);







% --- Executes on button press in Sensors.
function Sensors_Callback(hObject, eventdata, handles)
% hObject    handle to Sensors (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
data = getappdata(gcbf, 'horusdata');
% open Sensors only if it is not already open
if ~isfield(data,'hSensors')
    handleSensors=Sensors('handle',num2str(gcbf,16));
    data.hSensors=num2str(handleSensors,16);
elseif ~ishandle(str2double(data.hSensors)) 
    handleSensors=Sensors('handle',num2str(gcbf,16));
    data.hSensors=num2str(handleSensors,16);
end
setappdata(gcbf, 'horusdata', data); 






% --- Executes on button press in txtarmAxis.
%function txtarmAxis_Callback(hObject, eventdata, handles)
% hObject    handle to txtarmAxis (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
%data = getappdata(gcbf, 'horusdata');
%data.armAxis=get(hObject,'Value');
%if data.armAxis
%    set(hObject,'BackgroundColor','g','String','armAxis is ON');
%else
%    set(hObject,'BackgroundColor','c','String','armAxis is OFF');                
%end
%setappdata(gcbf, 'horusdata', data); 



