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

% Last Modified by GUIDE v2.5 31-Jan-2005 20:41:30

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

%setup Timer function
handles.ActTimer = timer('ExecutionMode','fixedDelay',...
      'Period',0.7,...    
      'BusyMode','drop',...
      'TimerFcn', {@DyelaserRefresh,handles});   

start(handles.ActTimer);
data.ActTimer=handles.ActTimer;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes Dyelaser wait for user response (see UIRESUME)
% uiwait(handles.figure1);
setappdata(handles.output, 'Dyelaserdata', data);


function DyelaserRefresh(arg1,arg2,handles)
data = getappdata(handles.output, 'Dyelaserdata');

%[s,w] = system('tail -n-10 data/ccStatus.txt > data/status_sub.txt');
%clear status_sub;
%load 'data/status_sub.txt';

%figure(GUI_handles.figDataGUI);
%set(GUI_handles.figDataGUI,'Visible','on');

%statusData=ReadDataAvg('status.bin',50,2500);
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
AvgData=horusdata.AvgData;
%[statusData,AvgData]=ReadDataAvg('/lift/ramdisk/status.bin',50,500);
statustime=double(statusData(:,2))./1.0+ ...
           double(statusData(:,3))./24.0+...
           double(statusData(:,4))./1440.0+...
           double(statusData(:,5))./86400.0;

%lastrow=size(statusData,1);
       
ZeitTage=double(statusData(:,2))/1.0+double(statusData(:,3))/24.0+...
    double(statusData(:,4))/1440.0+...
    double(statusData(:,5))/86400.0+...
    double(statusData(:,6))/86400000.0;
Stunden=double(statusData(:,3))+...
    double(statusData(:,4))/60.0+...
    double(statusData(:,5))/3600.0+...
    double(statusData(:,6))/3600000.0;
Minuten=double(statusData(:,4))+...
    double(statusData(:,5))/60.0+...
    double(statusData(:,6))/60000.0;
Zeit=Stunden/24.0;
[SortZeit,indexZeit]=sort(ZeitTage);
maxLen=size(ZeitTage,1);
lastrow=indexZeit(maxLen);

PlotWidth=maxLen;

stopPlot=maxLen;

startPlot=1;
iZeit=indexZeit(startPlot:stopPlot);
minTime=Zeit(iZeit(1));
maxTime=Zeit(iZeit(size(iZeit,1)));

ADCBase0=656;
ADCBase1=689;

% put data values in text fields

DiodeUV=4.6863E-6*double(statusData(iZeit,ADCBase0+2*3)).^2-8.5857E-2*double(statusData(iZeit,ADCBase0+2*3))+390.41;

set(handles.txtDiodeGr,'String',statusData(lastrow,ADCBase0+5*3));
set(handles.txtDiodeUV,'String',statusData(lastrow,ADCBase0+2*3));
set(handles.txtDiodeEt,'String',statusData(lastrow,ADCBase0+4*3));
set(handles.txtPDyelaser,'String',statusData(lastrow,ADCBase0+1*3));
set(handles.txtPVent,'String',statusData(lastrow,ADCBase0+7*3));
set(handles.txtIFilament,'String',statusData(lastrow,ADCBase1+6*3));
set(handles.txtPRef,'String',statusData(lastrow,ADCBase1+3*3));

EtalonBase=643; 
Etalonhelp=int32(statusData(:,EtalonBase));
EtalonSetPos=(Etalonhelp)+int32(statusData(:,EtalonBase+1));
EtalonSetPos(Etalonhelp>32767)=EtalonSetPos(Etalonhelp>32767)-65535;

Etalonhelp=int32(statusData(:,EtalonBase+2));
EtalonCurPos=(Etalonhelp)+int32(statusData(:,EtalonBase+3)); 
EtalonCurPos(Etalonhelp>32767)=EtalonCurPos(Etalonhelp>32767)-65535;

Etalonhelp=int32(statusData(:,EtalonBase+4)); 
EtalonEncPos=(Etalonhelp)+int32(statusData(:,EtalonBase+5)); 
EtalonEncPos(Etalonhelp>32767)=EtalonEncPos(Etalonhelp>32767)-65535;

set(handles.txtEtCurPos,'String',EtalonCurPos(lastrow));
set(handles.txtEtSetPos,'String',EtalonSetPos(lastrow));
set(handles.txtEtEncPos,'String',EtalonEncPos(lastrow));

% plot data values in graph 1

hold(handles.axes1,'off'); 

if get(handles.checkDiodeGr,'Value')
    plot(handles.axes1,Zeit(iZeit),statusData(iZeit,ADCBase0+5*3));
    hold(handles.axes1,'on');
end 

if get(handles.checkDiodeUV,'Value')
    plot(handles.axes1,Zeit(iZeit),DiodeUV(iZeit));
    hold(handles.axes1,'on');
end 

if get(handles.checkDiodeEt,'Value')
    plot(handles.axes1,Zeit(iZeit),statusData(iZeit,ADCBase0+4*3));
    hold(handles.axes1,'on');
end 

if get(handles.checkPDyelaser,'Value')
    plot(handles.axes1,Zeit(iZeit),statusData(iZeit,ADCBase0+1*3));
    hold(handles.axes1,'on');
end 

if get(handles.checkPVent,'Value')
    plot(handles.axes1,Zeit(iZeit),statusData(iZeit,ADCBase0+7*3));
    hold(handles.axes1,'on');
end 

if get(handles.chkEtCurPos,'Value')
    plot(handles.axes1,Zeit(iZeit),EtalonCurPos(iZeit));
    hold(handles.axes1,'on');
end 

if get(handles.chkEtSetPos,'Value')
    plot(handles.axes1,Zeit(iZeit),EtalonSetPos(iZeit));
    hold(handles.axes1,'on');
end 

if get(handles.chkEtEncPos,'Value')
    plot(handles.axes1,Zeit(iZeit),EtalonEncPos(iZeit));
    hold(handles.axes1,'on');
end 

timeStep=double(10.0/86400.0);
if (maxTime-minTime>0.7/1400.0)
    timeStep=1.0/1440.0;
end
timeXTick=[double(minTime):double(timeStep):double(maxTime)];

xlim(handles.axes1,[minTime maxTime]);
grid(handles.axes1);

% plot ref. signal in graph 2

PMTMaskBase=211;
PMTSumCounts=statusData(:,PMTMaskBase+12);

[SortEtpos,indexEtpos]=sort(EtalonCurPos);
iEtpos=indexEtpos(startPlot:stopPlot);
minEtpos=EtalonCurPos(iEtpos(1));
maxEtpos=EtalonCurPos(iEtpos(size(iEtpos,1)));
maxEtpos=max(maxEtpos,minEtpos+1);

if get(handles.radioTime,'Value');
    hold(handles.axes2,'off');
    plot(handles.axes2,Zeit(iZeit),PMTSumCounts(iZeit)); 
    hold(handles.axes2,'on');
    xlim(handles.axes2,[minTime maxTime]);
else
    hold(handles.axes2,'off');
    plot(handles.axes2,EtalonCurPos(iEtpos),PMTSumCounts(iEtpos)); 
    hold(handles.axes2,'on');
    xlim(handles.axes2,[minEtpos maxEtpos]);
end
    

grid(handles.axes2);


% check if filament was swiched off by horus
if get(handles.toggleFilament,'Value') & bitget(statusData(lastrow,724),14+1)==0;
        set(handles.toggleFilament,'string','Filament Off');
        set(hObject,'BackgroundColor','b');
end


%setappdata(handles.output, 'Dyelaserdata', data);


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
if get(hObject,'Value')
    Valveword=bitset(statusData(lastrow,724),7+1);
    set(hObject,'BackgroundColor','r');
else
    Valveword=bitset(statusData(lastrow,724),7+1,0);
    set(hObject,'BackgroundColor','c');
end
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(24*140))]);
system(['/lift/bin/eCmd w 0xa408 ', num2str(Valveword)]);
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(6*140))]);


% --- Executes on button press in toggleVacuum.
function toggleVacuum_Callback(hObject, eventdata, handles)
% hObject    handle to toggleVacuum (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleVacuum
if get(hObject,'Value')
    Valveword=bitset(statusData(lastrow,724),8+1);
    set(hObject,'BackgroundColor','r');
else
    Valveword=bitset(statusData(lastrow,724),8+1,0);
    set(hObject,'BackgroundColor','c');
end
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(24*140))]);
system(['/lift/bin/eCmd w 0xa408 ', num2str(Valveword)]);
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(6*140))]);


% --- Executes on button press in toggleN2.
function toggleN2_Callback(hObject, eventdata, handles)
% hObject    handle to toggleN2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleN2
if get(hObject,'Value')
    Valveword=bitset(statusData(lastrow,724),9+1);
    set(hObject,'BackgroundColor','r');
else
    Valveword=bitset(statusData(lastrow,724),9+1,0);
    set(hObject,'BackgroundColor','c');
end
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(24*140))]);
system(['/lift/bin/eCmd w 0xa408 ', num2str(Valveword)]);
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(6*140))]);


% --- Executes on button press in toggleAmbient.
function toggleAmbient_Callback(hObject, eventdata, handles)
% hObject    handle to toggleAmbient (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleAmbient
if get(hObject,'Value')
    Valveword=bitset(statusData(lastrow,724),10+1);
    set(hObject,'BackgroundColor','r');
else
    Valveword=bitset(statusData(lastrow,724),10+1,0);
    set(hObject,'BackgroundColor','c');
end
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(24*140))]);
system(['/lift/bin/eCmd w 0xa408 ', num2str(Valveword)]);
system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(6*140))]);


% --- Executes on button press in Exit.
function Exit_Callback(hObject, eventdata, handles)
% hObject    handle to Exit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
stop(handles.ActTimer);
delete(handles.ActTimer);
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
if get(hObject,'Value')
    if str2double(get(handles.txtPRef,'String'))<1000
        Valveword=bitset(statusData(lastrow,724),14+1);
        system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(15*140))]);
        system(['/lift/bin/eCmd w 0xa408 ', num2str(Valveword)]);
        system('sleep 1');
        system(['/lift/bin/eCmd w 0xa468 ', num2str(uint16(6*140))]);
        set(hObject,'string','Filament is ON');
        set(hObject,'BackgroundColor','r');
    end
else
    Valveword=bitset(statusData(lastrow,724),14+1,0);
    system(['/lift/bin/eCmd w 0xa408 ', num2str(Valveword)]);
    set(hObject,'string','Filament is OFF');
    set(hObject,'BackgroundColor','b');
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


% --- Executes on button press in chkEtSetPos.
function c_Callback(hObject, eventdata, handles)
% hObject    handle to chkEtSetPos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkEtSetPos


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



