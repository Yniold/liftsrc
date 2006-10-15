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

% Last Modified by GUIDE v2.5 19-Sep-2005 23:08:20

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
[data.statusData,data.AvgData]=ReadDataAvg('/lift/ramdisk/status.bin',50,80);
[data.col,data.fcts2val]=varassign(data.statusData);

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
    [data.statusData,data.AvgData]=ReadDataAvg('/lift/ramdisk/status.bin',50,80);
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

[SortZeit,indexZeit]=sort(statustime);
maxLen=size(statustime,1);
lastrow=indexZeit(maxLen);

% display system time
disptime=statustime(lastrow)-double(statusData(lastrow,6))/86400000.0;
set(handles.txtTime,'String',strcat(datestr(disptime,13),'.',num2str(statusData(lastrow,6)/100)));

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

% zero pitot every 5 min for 10 s if lamp is off
if statusData(lastrow,col.ValidSlaveDataFlag)
    if bitget(statusData(lastrow,col.Valve2armAxis),11)==0 % lamp off ?
        if ( mod(double(statusData(lastrow,4)),5)==0 & double(statusData(lastrow,5))<10 ) %first 10 sec period of every five min
            if bitget(statusData(lastrow,col.Valve1armAxis),12)==0 % if pitot 0 still off, switch it on
                Valveword=bitset(statusData(lastrow,col.Valve1armAxis),12);
                system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch
                system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
                system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to hold solenoids
            end
        else % 10 sec are over
            if bitget(statusData(lastrow,col.Valve1armAxis),12)==1 % if pitot 0 is still on, switch it off
                Valveword=bitset(statusData(lastrow,col.Valve1armAxis),12,0);
                system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch
                system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
                system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to hold solenoids
            end
        end
    end
end

% check dyelaser pressure and keep it constant on set value
x=double(statusData(:,col.PDyelaser)); eval(['PDyelaser=',fcts2val.PDyelaser,';']);
Pset=uint16(str2double(get(handles.edPset,'String')));
if Pset==0
   Pset=PDyelaser(lastrow);
end
set(handles.txtPset,'String',num2str(Pset));
% warning if PDyelaser differs too much from Pset
if PDyelaser(lastrow)>=Pset+5 | PDyelaser(lastrow)<=Pset-5
    set(handles.txtPset,'BackgroundColor','r')
else
    set(handles.txtPset,'BackgroundColor',[0.7 0.7 0.7])
end
if PDyelaser(lastrow)>=Pset+1; % PDyelaser too high
    Valveword=bitset(statusData(lastrow,col.ValveLift),9,0); % make sure air is switched off
    Valveword=bitset(Valveword,11); % switch vacuum on
    system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @Lift w 0xa408 ', num2str(Valveword)]);
    if isfield(data,'hDyelaser')
        if ishandle(data.hDyelaser), 
            Dyelaserdata = getappdata(data.hDyelaser, 'Dyelaserdata');
            set(Dyelaserdata.toggleVacuum,'BackgroundColor','g','String','Valve Vacuum ON');
        end
    end
    pause(1);
    Valveword=bitset(Valveword,8); % switch Dyelaser valve on
    system(['/lift/bin/eCmd @Lift w 0xa408 ', num2str(Valveword)]);
    if isfield(data,'hDyelaser')
        if ishandle(data.hDyelaser), 
            Dyelaserdata = getappdata(data.hDyelaser, 'Dyelaserdata');
            set(Dyelaserdata.toggleDyelaser,'BackgroundColor','g','String','Valve Dyelaser ON');
        end
    end
    pause(1);
    Valveword=bitset(Valveword,8,0); % switch Dyelaser valve off
    Valveword=bitset(Valveword,11,0); % switch vaccuum off 
    system(['/lift/bin/eCmd @Lift w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open
    if isfield(data,'hDyelaser')
        if ishandle(data.hDyelaser), 
            Dyelaserdata = getappdata(data.hDyelaser, 'Dyelaserdata');
            set(Dyelaserdata.toggleDyelaser,'BackgroundColor','c','String','Valve Dyelaser OFF');
            set(Dyelaserdata.toggleVacuum,'BackgroundColor','c','String','Valve Vacuum OFF');
        end
    end
elseif PDyelaser(lastrow)<=Pset-2; % PDyelaser too low
    Valveword=bitset(statusData(lastrow,col.ValveLift),11,0); % make sure vacuum is switched off
    Valveword=bitset(Valveword,9); % switch air on
    system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @Lift w 0xa408 ', num2str(Valveword)]);
    if isfield(data,'hDyelaser')
        if ishandle(data.hDyelaser), 
            Dyelaserdata = getappdata(data.hDyelaser, 'Dyelaserdata');
            set(Dyelaserdata.toggleN2,'BackgroundColor','g','String','Valve N2 ON');
        end
    end
    pause(0.5);
    Valveword=bitset(Valveword,9,0); % switch air off 
    Valveword=bitset(Valveword,8); % switch Dyelaser valve on
    system(['/lift/bin/eCmd @Lift w 0xa408 ', num2str(Valveword)]);
    if isfield(data,'hDyelaser')
        if ishandle(data.hDyelaser), 
            Dyelaserdata = getappdata(data.hDyelaser, 'Dyelaserdata');
            set(Dyelaserdata.toggleN2,'BackgroundColor','c','String','Valve N2 OFF');
            set(Dyelaserdata.toggleDyelaser,'BackgroundColor','g','String','Valve Dyelaser ON');
        end
    end
    pause(0.5);
    Valveword=bitset(Valveword,8,0); % switch Dyelaser valve off
    Valveword=bitset(Valveword,9,0); % switch air off 
    system(['/lift/bin/eCmd @Lift w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open
    if isfield(data,'hDyelaser')
        if ishandle(data.hDyelaser), 
            Dyelaserdata = getappdata(data.hDyelaser, 'Dyelaserdata');
            set(Dyelaserdata.toggleDyelaser,'BackgroundColor','c','String','Valve Dyelaser OFF');
            set(Dyelaserdata.toggleN2,'BackgroundColor','c','String','Valve N2 OFF');
        end
    end
end

% calculate ADC values needed for warnings
if ~isnan(col.TempDyelaser)
    x=double(statusData(:,col.TempDyelaser)); eval(['TDyelaser=',fcts2val.TempDyelaser,';']);
else
    TDyelaser=statustime; TDyelaser(:)=NaN;
end
x=double(statusData(:,col.P20)); eval(['P20=',fcts2val.P20,';']);
x=double(statusData(:,col.DiodeWZ1out)); eval(['DiodeWZ1out=',fcts2val.DiodeWZ1out,';']);
x=double(statusData(:,col.DiodeWZ2out)); eval(['DiodeWZ2out=',fcts2val.DiodeWZ2out,';']);
x=double(statusData(:,col.DiodeWZ1in)); eval(['DiodeWZ1in=',fcts2val.DiodeWZ1in,';']);
x=double(statusData(:,col.DiodeWZ2in)); eval(['DiodeWZ2in=',fcts2val.DiodeWZ2in,';']);
x=double(statusData(:,col.MFCFlow)); eval(['MFCFlow=',fcts2val.MFCFlow,';']);

PMTOnlineAvg(statusData(:,col.RAvgOnOffFlag)==3)=data.AvgData(statusData(:,col.RAvgOnOffFlag)==3,1);  
PMTOnlineAvg(statusData(:,col.RAvgOnOffFlag)~=3)=NaN;
PMTOfflineAvg(statusData(:,col.RAvgOnOffFlag)==1)=data.AvgData(statusData(:,col.RAvgOnOffFlag)==1,1);  
PMTOfflineAvg(statusData(:,col.RAvgOnOffFlag)~=1)=NaN;

% check which child GUIs are active and color push buttons accordingly
% red color if warning applies to one of the values controlled in the GUI
if isfield(data,'hCounterCards')
    if ishandle(data.hCounterCards) 
        set(handles.CounterCards,'BackgroundColor','g');
    else
        set(handles.CounterCards,'BackgroundColor','c');
    end
end
data.TDyelaserset=50; % define temperature Dye Laser heating is set to
if isfield(data,'hDyelaser')
    if ishandle(data.hDyelaser)
        set(handles.Dyelaser,'BackgroundColor','g');
    else
        if statusData(lastrow,col.PRef)>10500 | TDyelaser(lastrow)>data.TDyelaserset+1 | TDyelaser(lastrow)<data.TDyelaserset-1.5 |statusData(lastrow,col.IFilament)<10100
            set(handles.Dyelaser,'BackgroundColor','r');
        else
            set(handles.Dyelaser,'BackgroundColor','c');
        end
    end
end
if isfield(data,'hLaser')
    if ishandle(data.hLaser)
        set(handles.Laser,'BackgroundColor','g');
    else
        set(handles.Laser,'BackgroundColor','c');
    end
end
%if isfield(data,'hDetection')
%    if ishandle(data.hDetection) 
%        set(handles.FlyDetection,'BackgroundColor','g');
%    else
%        if P20(lastrow)<3 | P20(lastrow)>4 | DiodeWZ1in(lastrow)<3 | DiodeWZ1out(lastrow)<0.75*DiodeWZ1in ...
%                | DiodeWZ2in(lastrow)<0.4 | DiodeWZ2out(lastrow)<0.6*DiodeWZ2in | MFCFlow(lastrow)<5.5 | MFCFlow(lastrow)>6.5 ...
%                | statusData(lastrow,col.VHV)<12400 | PMTOnlineAvg(lastrow)<450 
%            set(handles.FlyDetection,'BackgroundColor','r');
%        else
%            set(handles.FlyDetection,'BackgroundColor','c');
%        end
%    end
%end
if isfield(data,'hFlyDetection')
    if ishandle(data.hFlyDetection) 
        set(handles.FlyDetection,'BackgroundColor','g');
    else
        if P20(lastrow)<1 | P20(lastrow)>5 | DiodeWZ1in(lastrow)<2 | DiodeWZ1out(lastrow)<0.75*DiodeWZ1in ...
                | DiodeWZ2in(lastrow)<0.4 | DiodeWZ2out(lastrow)<0.4*DiodeWZ2in | MFCFlow(lastrow)<4 | MFCFlow(lastrow)>9 ...
                | statusData(lastrow,col.VHV)<12400 | PMTOnlineAvg(lastrow)<2*PMTOfflineAvg(lastrow) | statusData(lastrow,col.PCuvette)<10100
            set(handles.FlyDetection,'BackgroundColor','r');
        else
            set(handles.FlyDetection,'BackgroundColor','c');
        end
    end
end
%if isfield(data,'hFlyOp')
%    if ishandle(data.hFlyOp) 
%        set(handles.FlyOp,'BackgroundColor','g');
%    else
%        if P20(lastrow)<3 | P20(lastrow)>4 | DiodeWZ1in(lastrow)<3 | DiodeWZ1out(lastrow)<0.75*DiodeWZ1in ...
%                | DiodeWZ2in(lastrow)<0.4 | DiodeWZ2out(lastrow)<0.6*DiodeWZ2in | MFCFlow(lastrow)<5.5 | MFCFlow(lastrow)>6.5 ...
%                | statusData(lastrow,col.VHV)<12400 | PMTOnlineAvg(lastrow)<450 ...
%                | statusData(lastrow,col.Lamp1)>10010 | statusData(lastrow,col.Lamp2)>10010 ...
%                | statusData(lastrow,col.PRef)>10500 | TDyelaser(lastrow)>51 | TDyelaser(lastrow)<49 |statusData(lastrow,col.IFilament)<10100
%            set(handles.FlyOp,'BackgroundColor','r');
%        else
%            set(handles.FlyOp,'BackgroundColor','c');
%        end
%    end
%end
if isfield(data,'hSensors')
    if ishandle(data.hSensors)
        set(handles.Sensors,'BackgroundColor','g');
    else
        set(handles.Sensors,'BackgroundColor','c');
    end
end
if isfield(data,'hCalibration')
    if ishandle(data.hCalibration)
        set(handles.Calibration,'BackgroundColor','g');
    else
        set(handles.Calibration,'BackgroundColor','c');
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
% open CounterCards only if it is not already open and if FlyDetection is open
if isfield(data,'hFlyDetection')
    if ishandle(data.hFlyDetection)
        if ~isfield(data,'hCounterCards')
            data.hCounterCards=CounterCards('handle',num2str(gcbf,16));
        elseif ~ishandle(str2double(data.hCounterCards))
            data.hCounterCards=CounterCards('handle',num2str(gcbf,16));
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

if statusData(lastrow,col.ValidSlaveDataFlag)    
    % shut Axis Valves Off
%    system(['/lift/bin/eCmd @armAxis w 0xa408 0x0000']);
    % shut HV Off
%    Valveword=bitset(statusData(lastrow,col.Valve2armAxis),8,0);  % switch HV off
%    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch HV
%    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
    % switch Gain off for MCP1
%    word=bitset(statusData(lastrow,col.ccGateDelay1),16,0);
%    system(['/lift/bin/eCmd @armAxis w 0xa318 ',num2str(word)]);
    % switch Gain off for MCP2
%    word=bitset(statusData(lastrow,col.ccGateDelay2),16,0);
%    system(['/lift/bin/eCmd @armAxis w 0xa31c ',num2str(word)]);
    % switch LED off
    Valveword=bitset(statusData(lastrow,col.Valve2armAxis),14,0);
    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
end

% close child GUIs

if isfield(data,'hDyelaser')
    hDyelaser=data.hDyelaser;
    if ishandle(hDyelaser), 
        Dyelaserdata = getappdata(hDyelaser, 'Dyelaserdata');
        if isfield(Dyelaserdata,'ActTimer')
            stop(Dyelaserdata.ActTimer);
            delete(Dyelaserdata.ActTimer);
        end
        if isvalid(Dyelaserdata.serport)
            fclose(Dyelaserdata.serport);
            delete(Dyelaserdata.serport);
        end;
        if isfield(Dyelaserdata,'picotport')
            fclose(Dyelaserdata.picotport);
            delete(Dyelaserdata.picotport);
        end;
        close(hDyelaser); 
    end
end

if isfield(data,'hCounterCards')
    hCounterCards=data.hCounterCards;
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
    hLaser=data.hLaser;
    if ishandle(hLaser), 
        Laserdata = getappdata(hLaser, 'tcpdata');
        fclose(Laserdata.tport);
        delete(Laserdata.tport);
        echotcpip('off');
        close(hLaser); 
    end
end

%if isfield(data,'hDetection')
%    hDetection=data.hDetection;
%    if ishandle(hDetection), 
%        Detdata = getappdata(hDetection, 'Detdata');
%        if isfield(Detdata,'Timer')
%            stop(Detdata.Timer);
%            delete(Detdata.Timer);
%        end
%        close(hDetection); 
%    end
%end

if isfield(data,'hFlyDetection')
    hFlyDetection=data.hFlyDetection;
    if ishandle(hFlyDetection), 
        FlyDetdata = getappdata(hFlyDetection, 'Detdata');
        if isfield(FlyDetdata,'Timer')
            stop(FlyDetdata.Timer);
            delete(FlyDetdata.Timer);
        end
        close(hFlyDetection); 
    end
end

%if isfield(data,'hFlyOp')
%    hFlyOp=data.hFlyOp;
%    if ishandle(hFlyOp), 
%        FlyOpdata = getappdata(hFlyOp, 'Detdata');
%        if isfield(FlyOpdata,'Timer')
%            stop(FlyOpdata.Timer);
%            delete(FlyOpdata.Timer);
%        end
%        close(hFlyOp); 
%    end
%end

if isfield(data,'hSensors')
    hSensors=data.hSensors;
    if ishandle(hSensors), 
        Sensdata = getappdata(hSensors, 'Sensdata');
        if isfield(Sensdata,'Timer')
            stop(Sensdata.Timer);
            delete(Sensdata.Timer);
        end
        close(hSensors); 
    end
end

if isfield(data,'hCalibration')
    hCalibration=data.hCalibration;
    if ishandle(hCalibration), 
        Sensdata = getappdata(hCalibration, 'Sensdata');
        if isfield(Caldata,'Timer')
            stop(Caldata.Timer);
            delete(Caldata.Timer);
        end
        close(hCalibration); 
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
    data.hDyelaser=Dyelaser('handle',num2str(gcbf,16));
elseif ~ishandle(str2double(data.hDyelaser)) 
    data.hDyelaser=Dyelaser('handle',num2str(gcbf,16));
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
    data.hLaser=Laser('handle',num2str(gcbf,16));
elseif ~ishandle(str2double(data.hLaser)) 
    data.hLaser=Laser('handle',num2str(gcbf,16));
end
setappdata(gcbf, 'horusdata', data); 






% --- Executes on button press in FlyDetection.
%function FlyDetection_Callback(hObject, eventdata, handles)
%data = getappdata(gcbf, 'horusdata');
% open FlyDetection only if it is not already open
%if ~isfield(data,'hDetection')
%    data.hDetection=FlyDetection('handle',num2str(gcbf,16));
%elseif ~ishandle(str2double(data.hDetection)) 
%    data.hDetection=FlyDetection('handle',num2str(gcbf,16));
%end
%setappdata(gcbf, 'horusdata', data);


% --- Executes on button press in FlyDetection.
function FlyDetection_Callback(hObject, eventdata, handles)
% hObject    handle to FlyDetection (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
data = getappdata(gcbf, 'horusdata');
% open FlyDetection only if it is not already open
if ~isfield(data,'hFlyDetection')
    data.hFlyDetection=FlyDetection('handle',num2str(gcbf,16));
elseif ~ishandle(str2double(data.hFlyDetection)) 
    data.hFlyDetection=FlyDetection('handle',num2str(gcbf,16));
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
    data.hSensors=Sensors('handle',num2str(gcbf,16));
elseif ~ishandle(str2double(data.hSensors)) 
    data.hSensors=Sensors('handle',num2str(gcbf,16));
end
setappdata(gcbf, 'horusdata', data); 





function edPset_Callback(hObject, eventdata, handles)
% hObject    handle to edPset (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edPset as text
%        str2double(get(hObject,'String')) returns contents of edPset as a double
Pset=uint16(str2double(get(hObject,'String')));
if isnan(Pset) 
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','white');
    set(hObject,'string',num2str(Pset));
end


% --- Executes during object creation, after setting all properties.
%function edPset_CreateFcn(hObject, eventdata, handles)
% hObject    handle to d2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
%data = getappdata(gcbf, 'horusdata');
%statusData=data.statusData;
%col=data.col;
%lastrow=data.lastrow;
%set(hObject,'String',statusdata(lastrow,col.PDyelaser));




% --- Executes on button press in Calibration.
function Calibration_Callback(hObject, eventdata, handles)
% hObject    handle to Calibration (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
data = getappdata(gcbf, 'horusdata');
% open Sensors only if it is not already open
if ~isfield(data,'hCalibration')
    data.hCalibration=Calibration('handle',num2str(gcbf,16));
elseif ~ishandle(str2double(data.hCalibration)) 
    data.hCalibration=Calibration('handle',num2str(gcbf,16));
end
setappdata(gcbf, 'horusdata', data); 




% --- Executes on button press in FlyOp.
%function FlyOp_Callback(hObject, eventdata, handles)
%data = getappdata(gcbf, 'horusdata');
% open FlyDetection only if it is not already open
%if ~isfield(data,'hFlyOp')
%    data.hFlyOp=FlyOp('handle',num2str(gcbf,16));
%elseif ~ishandle(str2double(data.hFlyOp)) 
%    data.hFlyOp=FlyOp('handle',num2str(gcbf,16));
%end
%setappdata(gcbf, 'horusdata', data);


