function varargout = Dyelaser(varargin)
% DYELASER M-file for Dyelaser.fig
%      DYELASER, by itself, creates a new DYELASER or raises the existing
%      singleton*.
%
%      H = DYELASER returns the handle to a new DYELASER or the handle to
%      the existing singleton*.
%
%      DYELASER('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in DYELASER.M with the given input arguments.
%
%      DYELASER('Property','Value',...) creates a new DYELASER or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before Dyelaser_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to Dyelaser_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Copyright 2002-2003 The MathWorks, Inc.

% Edit the above text to modify the response to help Dyelaser

% Last Modified by GUIDE v2.5 27-May-2005 13:19:11

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @Dyelaser_OpeningFcn, ...
                   'gui_OutputFcn',  @Dyelaser_OutputFcn, ...
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


% --- Executes just before Dyelaser is made visible.
function Dyelaser_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to Dyelaser (see VARARGIN)

% Choose default command line output for Dyelaser
handles.output = hObject;

% get horus handle
if length(varargin)==2 & varargin{1}=='handle'
    handles.parenthandle=str2double(varargin{2});
end

% setup Timer function
handles.ActTimer = timer('ExecutionMode','fixedDelay',...
      'Period',0.7,...    
      'BusyMode','drop',...
      'TimerFcn', {@DyelaserRefresh,handles});   

start(handles.ActTimer);
data.ActTimer=handles.ActTimer;


% open communication with picomotors
handles.serport=serial('/dev/ttyS0','BaudRate',19200,'Terminator','CR');
set(handles.serport,'BytesAvailableFcn',{'serialdatacallback'});
try fopen(handles.serport);
catch 
    delete(handles.serport);
    rmfield(handles,'serport');
end;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes Dyelaser wait for user response (see UIRESUME)
% uiwait(handles.figure1);
setappdata(handles.output, 'Dyelaserdata', data);


function DyelaserRefresh(arg1,arg2,handles)
data = getappdata(handles.output, 'Dyelaserdata');

horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
AvgData=horusdata.AvgData;
col=horusdata.col;
fcts2val=horusdata.fcts2val;

statustime=double(statusData(:,1))+ ...
           double(statusData(:,2))./1.0+ ...
           double(statusData(:,3))./24.0+...
           double(statusData(:,4))./1440.0+...
           double(statusData(:,5))./86400.0;

[SortZeit,indexZeit]=sort(statustime);
maxLen=size(statustime,1);
lastrow=indexZeit(maxLen);

PlotWidth=maxLen;
stopPlot=maxLen;
startPlot=1;
iZeit=indexZeit(startPlot:stopPlot);
minTime=statustime(iZeit(1));
maxTime=statustime(iZeit(size(iZeit,1)));

% display ADC values

x=double(statusData(:,col.DiodeUV)); eval(['DiodeUV=',fcts2val.DiodeUV,';']);

set(handles.txtDiodeGr,'String',statusData(lastrow,col.DiodeGr));
set(handles.txtDiodeUV,'String',statusData(lastrow,col.DiodeUV));
set(handles.txtDiodeEt,'String',statusData(lastrow,col.DiodeEtalon));
set(handles.txtPDyelaser,'String',statusData(lastrow,col.PDyelaser));
set(handles.txtPVent,'String',statusData(lastrow,col.PVent));
set(handles.txtIFilament,'String',statusData(lastrow,col.IFilament));
set(handles.txtPRef,'String',statusData(lastrow,col.PRef));
if statusData(lastrow,col.PRef)>10500
    set(handles.txtPRef,'BackgroundColor','r');
else
    set(handles.txtPRef,'BackgroundColor',[0.7,0.7,0.7]);
end

Etalonhelp=int32(statusData(:,col.etaSetPosLow));
EtalonSetPos=(Etalonhelp)+int32(statusData(:,col.etaSetPosHigh));
EtalonSetPos(Etalonhelp>32767)=EtalonSetPos(Etalonhelp>32767)-65535;

Etalonhelp=int32(statusData(:,col.etaCurPosLow));
EtalonCurPos=(Etalonhelp)+int32(statusData(:,col.etaCurPosHigh)); 
EtalonCurPos(Etalonhelp>32767)=EtalonCurPos(Etalonhelp>32767)-65535;

Etalonhelp=int32(statusData(:,col.etaEncoderPosLow)); 
EtalonEncPos=(Etalonhelp)+int32(statusData(:,col.etaEncoderPosHigh)); 
EtalonEncPos(Etalonhelp>32767)=EtalonEncPos(Etalonhelp>32767)-65535;

EtalonSpeed=statusData(:,col.etaSetSpd);
EtalonStatus=statusData(:,col.etaStatus);

set(handles.txtEtCurPos,'String',EtalonCurPos(lastrow));
set(handles.txtEtSetPos,'String',EtalonSetPos(lastrow));
set(handles.txtEtEncPos,'String',EtalonEncPos(lastrow));

if bitget(EtalonStatus(lastrow),9)
    set(handles.txtLimitSwitch,'String','left','BackgroundColor','r');
elseif bitget(EtalonStatus(lastrow),10)
    set(handles.txtLimitSwitch,'String','right','BackgroundColor','r');
else
    set(handles.txtLimitSwitch,'String','none','BackgroundColor','c');
end

% plot parameters in graph 1

hold(handles.axes1,'off'); 

if get(handles.checkDiodeGr,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.DiodeGr),'r');
    hold(handles.axes1,'on');
end 

if get(handles.checkDiodeUV,'Value')
    plot(handles.axes1,statustime(iZeit),DiodeUV(iZeit));
    hold(handles.axes1,'on');
end 

if get(handles.checkDiodeEt,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.DiodeEtalon),'r');
    hold(handles.axes1,'on');
end 

if get(handles.checkPDyelaser,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.PDyelaser),'r');
    hold(handles.axes1,'on');
end 

if get(handles.checkPVent,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.PVent),'r');
    hold(handles.axes1,'on');
end 

if get(handles.chkEtCurPos,'Value')
    plot(handles.axes1,statustime(iZeit),EtalonCurPos(iZeit));
    hold(handles.axes1,'on');
end 

if get(handles.chkEtSetPos,'Value')
    plot(handles.axes1,statustime(iZeit),EtalonSetPos(iZeit));
    hold(handles.axes1,'on');
end 

if get(handles.chkEtEncPos,'Value')
    plot(handles.axes1,statustime(iZeit),EtalonEncPos(iZeit));
    hold(handles.axes1,'on');
end 

xlim(handles.axes1,[minTime maxTime]);
grid(handles.axes1);

% plot ref. signal in graph 2

[SortEtpos,indexEtpos]=sort(EtalonCurPos);
iEtpos=indexEtpos(startPlot:stopPlot);
minEtpos=EtalonCurPos(iEtpos(1));
maxEtpos=EtalonCurPos(iEtpos(size(iEtpos,1)));
maxEtpos=max(maxEtpos,minEtpos+1);

if get(handles.radioTime,'Value');
    hold(handles.axes2,'off');
    plot(handles.axes2,statustime(iZeit),statusData(iZeit,col.ccCounts0)); 
    hold(handles.axes2,'on');
    xlim(handles.axes2,[minTime maxTime]);
else
    hold(handles.axes2,'off');
    plot(handles.axes2,EtalonCurPos(iEtpos),statusData(iEtpos,col.ccCounts0)); 
    hold(handles.axes2,'on');
    xlim(handles.axes2,[minEtpos maxEtpos]);
end
grid(handles.axes2);


% check filament status (e.g. if it was swiched off by horus)
if bitget(statusData(lastrow,col.Valve),14)==0;
    set(handles.toggleFilament,'Value',0,'string','Filament is OFF');
    set(handles.toggleFilament,'BackgroundColor','c');
else 
    set(handles.toggleFilament,'Value',1,'string','Filament is ON');
    set(handles.toggleFilament,'BackgroundColor','g');
end

% check shutter status
if bitget(statusData(lastrow,col.Valve),13)==0;
    set(handles.toggleShutter,'Value',0,'string','Shutter is OPEN');
    set(handles.toggleShutter,'BackgroundColor','g');
else 
    set(handles.toggleShutter,'Value',1,'string','Shutter is CLOSED');
    set(handles.toggleShutter,'BackgroundColor','c');
end

data.lastrow=lastrow;
setappdata(handles.output, 'Dyelaserdata', data);


% --- Outputs from this function are returned to the command line.
function varargout = Dyelaser_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in checkDiodeGr.
function checkDiodeGr_Callback(hObject, eventdata, handles)
% hObject    handle to checkDiodeGr (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkDiodeGr


% --- Executes on button press in checkDiodeUV.
function checkDiodeUV_Callback(hObject, eventdata, handles)
% hObject    handle to checkDiodeUV (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkDiodeUV


% --- Executes on button press in checkDiodeEt.
function checkDiodeEt_Callback(hObject, eventdata, handles)
% hObject    handle to checkDiodeEt (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkDiodeEt


% --- Executes on button press in checkPDyelaser.
function checkPDyelaser_Callback(hObject, eventdata, handles)
% hObject    handle to checkPDyelaser (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkPDyelaser


% --- Executes on button press in checkPVent.
function checkPVent_Callback(hObject, eventdata, handles)
% hObject    handle to checkPVent (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkPVent


% --- Executes on button press in toggleDyelaser.
function toggleDyelaser_Callback(hObject, eventdata, handles)
% hObject    handle to toggleDyelaser (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleDyelaser
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Dyelaserdata');
lastrow=data.lastrow;

if get(hObject,'Value')
    Valveword=bitset(statusData(lastrow,col.Valve),10+1);
    set(hObject,'BackgroundColor','g','String','Valve Dyelaser ON');
else
    Valveword=bitset(statusData(lastrow,col.Valve),10+1,0);
    set(hObject,'BackgroundColor','c','String','Valve Dyelaser OFF');
end
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
system(['/lift/bin/eCmd w 0xa408 ', num2str(Valveword)]);
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open


% --- Executes on button press in toggleVacuum.
function toggleVacuum_Callback(hObject, eventdata, handles)
% hObject    handle to toggleVacuum (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleVacuum
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Dyelaserdata');
lastrow=data.lastrow;

if get(hObject,'Value')
    Valveword=bitset(statusData(lastrow,col.Valve),7+1);
    set(hObject,'BackgroundColor','g','String','Valve Vacuum ON');
else
    Valveword=bitset(statusData(lastrow,col.Valve),7+1,0);
    set(hObject,'BackgroundColor','c','String','Valve Vacuum OFF');
end
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
system(['/lift/bin/eCmd w 0xa408 ', num2str(Valveword)]);
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open


% --- Executes on button press in toggleN2.
function toggleN2_Callback(hObject, eventdata, handles)
% hObject    handle to toggleN2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleN2
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Dyelaserdata');
lastrow=data.lastrow;

if get(hObject,'Value')
    Valveword=bitset(statusData(lastrow,col.Valve),9+1);
    set(hObject,'BackgroundColor','g','String','Valve N2 ON');
else
    Valveword=bitset(statusData(lastrow,col.Valve),9+1,0);
    set(hObject,'BackgroundColor','c','String','Valve N2 OFF');
end
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
system(['/lift/bin/eCmd w 0xa408 ', num2str(Valveword)]);
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open


% --- Executes on button press in toggleAmbient.
function toggleAmbient_Callback(hObject, eventdata, handles)
% hObject    handle to toggleAmbient (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleAmbient
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Dyelaserdata');
lastrow=data.lastrow;

if get(hObject,'Value')
    Valveword=bitset(statusData(lastrow,col.Valve),8+1);
    set(hObject,'BackgroundColor','g','String','Valve Ambient ON');
else
    Valveword=bitset(statusData(lastrow,col.Valve),8+1,0);
    set(hObject,'BackgroundColor','c','String','Valve Ambient OFF');
end
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
system(['/lift/bin/eCmd w 0xa408 ', num2str(Valveword)]);
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open


% --- Executes on button press in Exit.
function Exit_Callback(hObject, eventdata, handles)
% hObject    handle to Exit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
stop(handles.ActTimer);
delete(handles.ActTimer);
if isvalid(handles.serport)
    fclose(handles.serport);
    delete(handles.serport);
end;
close(handles.figure1);





function online_pos_Callback(hObject, eventdata, handles)
% hObject    handle to online_pos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of online_pos as text
%        str2double(get(hObject,'String')) returns contents of online_pos as a double
onlinepos=uint16(str2double(get(hObject,'String')));
if isnan(onlinepos)
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','white');
    set(hObject,'string',num2str(onlinepos));
end



function scan_start_pos_Callback(hObject, eventdata, handles)
% hObject    handle to scan_start_pos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of scan_start_pos as text
%        str2double(get(hObject,'String')) returns contents of scan_start_pos as a double
scanstartpos=uint16(str2double(get(hObject,'String')));
if isnan(scanstartpos)
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','white');
    set(hObject,'string',num2str(scanstartpos));
end



function scan_stop_pos_Callback(hObject, eventdata, handles)
% hObject    handle to scan_stop_pos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of scan_stop_pos as text
%        str2double(get(hObject,'String')) returns contents of scan_stop_pos as a double
scanstoppos=uint16(str2double(get(hObject,'String')));
if isnan(scanstoppos)
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','white');
    set(hObject,'string',num2str(scanstoppos));
end


function scan_step_Callback(hObject, eventdata, handles)
% hObject    handle to scan_step (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of scan_step as text
%        str2double(get(hObject,'String')) returns contents of scan_step as a double
scanstep=uint8(str2double(get(hObject,'String')));
if isnan(scanstep)
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','white');
    scanstep=scanstep/16*16;
    if scanstep<=0, scanstep=16; end;
    set(hObject,'string',num2str(scanstep));
end


% --- Executes on button press in scan_pushbutton.
function scan_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to scan_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
scanstartpos=uint16(str2double(get(handles.scan_start_pos,'String')));
scanstoppos=uint16(str2double(get(handles.scan_stop_pos,'String')));
scanstep=uint8(str2double(get(handles.scan_step,'String')));
if (isnan(scanstartpos)| isnan(scanstoppos) | isnan(scanstep))
    error('invalid values');
else
    system(['/lift/bin/eCmd s etalonnop']);    
    system(['/lift/bin/eCmd s etalonscanstart ',num2str(scanstartpos)])
    system(['/lift/bin/eCmd s etalonscanstop ',num2str(scanstoppos)]);
    system(['/lift/bin/eCmd s etalonscanstep ',num2str(scanstep)]);
    system('/lift/bin/eCmd s etalonscan');
end


% --- Executes on button press in online_pushbutton.
function online_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to online_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
onlinepos=uint16(str2double(get(handles.online_pos,'String')));
if isnan(onlinepos)
    error('invalid values');
else
    system(['/lift/bin/eCmd s etalonnop']);    
    system(['/lift/bin/eCmd w 0xa510 ',num2str(onlinepos)]);
    system(['/lift/bin/eCmd s etalononline ',num2str(onlinepos)]);
end


% --- Executes on button press in offline_pushbutton.
function offline_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to offline_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
onlinepos=uint16(str2double(get(handles.online_pos,'String')));
offlinepos=onlinepos+1000;
if isnan(onlinepos)
    error('invalid values');
else
    system(['/lift/bin/eCmd s etalonnop']);    
    system(['/lift/bin/eCmd w 0xa510 ',num2str(offlinepos)]);
end


% --- Executes on button press in toggle_pushbutton.
function toggle_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to toggle_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
onlinepos=uint16(str2double(get(handles.online_pos,'String')));
if isnan(onlinepos)
    error('invalid values');
else
    system(['/lift/bin/eCmd s etalonnop']);    
    system(['/lift/bin/eCmd s etalononline ',num2str(onlinepos)]);
    system(['/lift/bin/eCmd s etalonofflineleft 1000']);
    system(['/lift/bin/eCmd s etalonofflineright 1000']);
    system('/lift/bin/eCmd s etalondither 8');
    system('/lift/bin/eCmd s etalontoggle');
end


% --- Executes on button press in home_pushbutton.
function home_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to home_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
system(['/lift/bin/eCmd s etalonnop']);    
system('/lift/bin/eCmd w 0xa510 0');



% --- Executes on button press in toggleFilament.
function toggleFilament_Callback(hObject, eventdata, handles)
% hObject    handle to toggleFilament (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleFilament
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Dyelaserdata');
lastrow=data.lastrow;

if get(hObject,'Value')
    if statusData(lastrow,col.PRef)<=10500 % check if pressure in reference cell is low enough
        Valveword=bitset(statusData(lastrow,col.Valve),14);
        system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(15*140))]);% 15V needed to switch filament relay on
        system(['/lift/bin/eCmd w 0xa408 ', num2str(Valveword)]);
        system('sleep 1');
        system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open
        set(hObject,'String','Filament is ON');
        set(hObject,'BackgroundColor','g');
    else 
        set(hObject,'Value',0);
    end
else
    Valveword=bitset(statusData(lastrow,col.Valve),14,0);
    system(['/lift/bin/eCmd w 0xa408 ', num2str(Valveword)]);
    set(hObject,'String','Filament is OFF');
    set(hObject,'BackgroundColor','c');
end
        


% --- Executes on button press in radioTime.
function radioTime_Callback(hObject, eventdata, handles)
% hObject    handle to radioTime (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radioTime
if get(hObject,'Value')
    set(handles.radioEtpos,'Value',0);
end


% --- Executes on button press in radioEtpos.
function radioEtpos_Callback(hObject, eventdata, handles)
% hObject    handle to radioEtpos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radioEtpos
if get(hObject,'Value')
    set(handles.radioTime,'Value',0);
end


% --- Executes on button press in chkEtCurPos.
function chkEtCurPos_Callback(hObject, eventdata, handles)
% hObject    handle to chkEtCurPos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkEtCurPos


% --- Executes on button press in chkEtEncPos.
function chkEtEncPos_Callback(hObject, eventdata, handles)
% hObject    handle to chkEtEncPos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkEtEncPos


% --- Executes on button press in chkEtSetPos.
function chkEtSetPos_Callback(hObject, eventdata, handles)
% hObject    handle to chkEtSetPos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkEtSetPos





% --- Executes on button press in toggleShutter.
function toggleShutter_Callback(hObject, eventdata, handles)
% hObject    handle to toggleShutter (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleShutter
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Dyelaserdata');
lastrow=data.lastrow;
if get(hObject,'Value')
    Valveword=bitset(statusData(lastrow,col.Valve),13);
    system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(20*140))]);% 20V needed to close shutter
    system(['/lift/bin/eCmd w 0xa408 ', num2str(Valveword)]);
    set(hObject,'String','Shutter is CLOSED');
    set(hObject,'BackgroundColor','c');
    system('sleep 1');
    system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open
else    
    Valveword=bitset(statusData(lastrow,col.Valve),13,0);
    system(['/lift/bin/eCmd w 0xa408 ', num2str(Valveword)]);
    set(hObject,'String','Shutter is OPEN');
    set(hObject,'BackgroundColor','g');
end


% --- Executes on selection change in popupmirror.
function popupmirror_Callback(hObject, eventdata, handles)
% hObject    handle to popupmirror (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns popupmirror contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popupmirror
set(handles.textPos,'String','0');

% --- Executes during object creation, after setting all properties.
function popupmirror_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmirror (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
set(hObject,'Value',2);

% --- Executes on button press in radiohor.
function radiohor_Callback(hObject, eventdata, handles)
% hObject    handle to radiohor (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if get(hObject,'Value')
    set(handles.radiover,'Value',0);
end
set(handles.textPos,'String','0');


% --- Executes on button press in radiover.
function radiover_Callback(hObject, eventdata, handles)
% hObject    handle to radiover (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if get(hObject,'Value')
    set(handles.radiohor,'Value',0);
end
set(handles.textPos,'String','0');


% --- Executes on button press in radiofor.
function radiofor_Callback(hObject, eventdata, handles)
% hObject    handle to radiofor (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if get(hObject,'Value')
    set(handles.radiorev,'Value',0);
end


% --- Executes on button press in radiorev.
function radiorev_Callback(hObject, eventdata, handles)
% hObject    handle to radiorev (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if get(hObject,'Value')
    set(handles.radiofor,'Value',0);
end


function editsteps_Callback(hObject, eventdata, handles)
% hObject    handle to editsteps (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editsteps as text
%        str2double(get(hObject,'String')) returns contents of editsteps as a double
steps=uint16(str2double(get(hObject,'String')));
if isnan(steps)
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','white');
    set(hObject,'string',num2str(steps));
end



% --- Executes on button press in pushgo.
function pushgo_Callback(hObject, eventdata, handles)
% hObject    handle to pushgo (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(handles.textPos,'BackgroundColor','r');
set(handles.pushgo,'BackgroundColor','r');

serport=handles.serport;
steps=get(handles.editsteps,'String');
hor=get(handles.radiohor,'Value');
forw=get(handles.radiofor,'Value');
oldpos=str2double(get(handles.textPos,'String'));

switch get(handles.popupmirror,'Value')
    case 1
        driver='a1';
        if hor==1 chl='0';
        else chl='1';
        end
    case 2
        if hor==1;
            driver='a1';
            chl='2';
        else 
            driver='a2';
            chl='0';
        end
    case 3 
        driver='a2';
        if hor==1 chl='1';
        else chl='2';
        end
end
fprintf(serport,['chl ',driver,'=',chl]);
if forw==1
    fprintf(serport,['rel ',driver,' ',steps]);
else
    fprintf(serport,['rel ',driver,' -',steps]);
end
fprintf(serport,['go ',driver]);

% check if motor is still moving
fprintf(serport,['pos ',driver]);
x=find(serport.UserData=='=');
pos2=str2double(serport.UserData(x+1:length(serport.UserData)-1));
%serport.UserData
pos1=pos2-1;
while pos2~=pos1
    pos1=pos2;
    pause(1);
    fprintf(serport,['pos ',driver]);
    x=find(serport.UserData=='=');
    pos2=str2double(serport.UserData(x+1:length(serport.UserData)-1));
end
% display new motor position after motor has stopped
newpos=oldpos+pos2;
set(handles.textPos,'String',num2str(newpos),'BackgroundColor','w');
set(handles.pushgo,'BackgroundColor','w');




% --- Executes on button press in pushStop.
function pushStop_Callback(hObject, eventdata, handles)
% hObject    handle to pushStop (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
fprintf(handles.serport,'hal');


