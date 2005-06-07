function varargout = Laser(varargin)
% GUI for pump laser control
% diode current should be 35.4 for PSSN 120865, HEADSN 2363/710
% diode temperature should be 26.1?C for PSSN 120865, HEADSN 2363/710
% crystal temperature should be 151 for PSSN 120865, HEADSN 2363/710

% LASER M-file for Laser.fig
%      LASER, by itself, creates a new LASER or raises the existing
%      singleton*.
%
%      H = LASER returns the handle to a new LASER or the handle to
%      the existing singleton*.
%
%      LASER('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in LASER.M with the given input arguments.
%
%      LASER('Property','Value',...) creates a new LASER or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before Laser_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to Laser_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Copyright 2002-2003 The MathWorks, Inc.

% Edit the above text to modify the response to help Laser

% Last Modified by GUIDE v2.5 09-Feb-2005 14:06:24

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @Laser_OpeningFcn, ...
                   'gui_OutputFcn',  @Laser_OutputFcn, ...
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


% --- Executes just before Laser is made visible.
function Laser_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to Laser (see VARARGIN)

% Choose default command line output for Laser
handles.output = hObject;

% get horus handle
if length(varargin)==2 & varargin{1}=='handle'
    handles.parenthandle=str2double(varargin{2});
end

% open tcpip port for communication with Laser
echotcpip('on',10001);
tcpdata.tport=tcpip('xpLaser',10001);
set(tcpdata.tport,'ReadAsyncMode','continuous');
set(tcpdata.tport,'BytesAvailableFcn',{'tcpipdatacallback'});
try fopen(tcpdata.tport);
    tport=tcpdata.tport;

% check which laser head and power supply is being used and display what
% the settings should be
    fprintf(tport,'?PSSN'); 
    pause(0.5);
    PSSN=tport.UserData;
    fprintf(tport,'?HEADSN'); 
    pause(0.5);
    HEADSN=tport.UserData;

% diode current should be 35.4 for PSSN 120865, HEADSN 2363/710
% diode temperature should be 26.1?C for PSSN 120865, HEADSN 2363/710
% crystal temperature should be 151 for PSSN 120865, HEADSN 2363/710

% diode current should be 36.0 for PSSN 120881, HEADSN 2366/712
% diode temperature should be 28.6?C for PSSN 120881, HEADSN 2366/712
% crystal temperature should be 130 for PSSN 120881, HEADSN 2366/712

    set(handles.txtsetReprate,'String','3000 kHz'); % pulses/s
    if PSSN(1:6)=='120865'
        set(handles.txtsetDiodeCurrent,'String','35.4 A');
        set(handles.txtsetDiodeTemp,'String','26.1 C');
        set(handles.txtsetIRPower,'String','4.5 W'); % IR intern
    elseif PSSN(1:6)=='120881'
        set(handles.txtsetDiodeCurrent,'String','36.0 A');
        set(handles.txtsetDiodeTemp,'String','28.6 C');
        set(handles.txtsetIRPower,'String','4.45 W'); % IR intern
    end
    if HEADSN(1:8)=='2363/710'
        set(handles.txtsetCrtemp,'String','151'); % ADC Counts
        set(handles.txtsetTowerTemp,'String','31 C');
    elseif HEADSN(1:8)=='2366/712'
        set(handles.txtsetCrtemp,'String','130'); % ADC Counts
        set(handles.txtsetTowerTemp,'String','28.6 C');    
    end

% if communication with laser did not work
catch 
    delete(tcpdata.tport);
    echotcpip('off');
    set(handles.txtCommandAnswer,'String','communication FAILED','BackgroundColor','r');
end

% Update handles structure
guidata(hObject, handles);

setappdata(handles.output, 'tcpdata', tcpdata); 
if isvalid(tcpdata.tport)
    Update_Callback(hObject, eventdata, handles);
end



% --- Outputs from this function are returned to the command line.
function varargout = Laser_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in toggleLaser.
function toggleLaser_Callback(hObject, eventdata, handles)
% hObject    handle to toggleLaser (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(handles.txtStatus,'String','Query Status','ForegroundColor','r');
Zeit=clock;
set(handles.txtZeit,'String',datestr(Zeit,13));

%mark strings to be updated
set(handles.txtDiodeCurrent,'ForegroundColor','r');
set(handles.txtIRPower,'ForegroundColor','r');
set(handles.txtDiodeTemp,'ForegroundColor','r');
set(handles.txtTowerTemp,'ForegroundColor','r');

% switch laser on or off
tcpdata = getappdata(handles.output, 'tcpdata'); 
tport=tcpdata.tport;
if get(hObject,'Value')
    fprintf(tport,'D1'); 
    set(hObject,'BackgroundColor','g');
    set(hObject,'String','Laser switched ON')
    pause(0.5);
else
    fprintf(tport,'D0');
    set(hObject,'BackgroundColor','r');
    set(hObject,'String','Laser switched OFF')
    pause(0.5);
end
pause(3) 
fprintf(tport,'?D'); 
pause(0.5);
DiodeStatus=tport.UserData;
% check if switching was successfull and update laser toggle
if strcmp(DiodeStatus(1:2),'ON')
    set(hObject,'Value',1)
    set(hObject,'BackgroundColor','g');
    set(hObject,'String','Laser is ON')
elseif strcmp(DiodeStatus(1:3),'OFF')
    set(hObject,'Value',0)
    set(hObject,'BackgroundColor','c');
    set(hObject,'String','Laser is OFF')
end

% check diode parameters

% diode current
fprintf(tport,'?C1');
pause(0.5);
DiodeCurrent=tport.UserData;
set(handles.txtDiodeCurrent,'String',DiodeCurrent,'ForegroundColor','k');
% check diode current setting and update power toggle 
fprintf(tport,'?CS1'); 
pause(0.5);
DiodeCurrentSet=tport.UserData;
% current depends on power supply
fprintf(tport,'?PSSN'); 
pause(0.5);
PSSN=tport.UserData;
if PSSN(1:6)=='120865'
    if str2double(DiodeCurrentSet)==35.4
        set(handles.togglePower,'Value',1,'String','FULL Power','BackgroundColor','g');
    elseif str2double(DiodeCurrentSet)==14
        set(handles.togglePower,'Value',0,'String','LOW Power','BackgroundColor','c');
    else
        set(handles.togglePower,'Value',0,'String','medium Power','BackgroundColor','c');
    end
elseif PSSN(1:6)=='120881'
    if str2double(DiodeCurrentSet)==36.0
        set(handles.togglePower,'Value',1,'String','FULL Power','BackgroundColor','g');
    elseif str2double(DiodeCurrentSet)==14
        set(handles.togglePower,'Value',0,'String','LOW Power','BackgroundColor','c');
    else
        set(handles.togglePower,'Value',0,'String','medium Power','BackgroundColor','c');
    end
end

% IR power
fprintf(tport,'?P');
pause(0.5);
IRPower=tport.UserData;
set(handles.txtIRPower,'String',IRPower,'ForegroundColor','k');

% diode temperature
fprintf(tport,'?T1');
pause(0.5);
DiodeTemp=tport.UserData;
set(handles.txtDiodeTemp,'String',DiodeTemp,'ForegroundColor','k');

% tower temperature
fprintf(tport,'?TT');
pause(0.5);
TowerTemp=tport.UserData;
set(handles.txtTowerTemp,'String',TowerTemp,'ForegroundColor','k');

set(handles.txtStatus,'String','Idle','ForegroundColor','k');


% --- Executes on button press in toggleShutter.
function toggleShutter_Callback(hObject, eventdata, handles)
% hObject    handle to toggleShutter (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(handles.txtStatus,'String','Query Status','ForegroundColor','r');
tcpdata = getappdata(handles.output, 'tcpdata'); 
tport=tcpdata.tport;
% switch shutter on or off
if get(hObject,'Value')
    fprintf(tport,'SHT:1'); 
    set(hObject,'BackgroundColor','r')
    set(hObject,'String','Shutter opened')
    pause(0.5);
else
    fprintf(tport,'SHT:0');
    set(hObject,'BackgroundColor','r')
    set(hObject,'String','Shutter closed')
    pause(0.5);
end
%check if switching was successfull
fprintf(tport,'?SHT'); 
pause(3);
ShutterStatus=tport.UserData;
if strcmp(ShutterStatus(1:4),'OPEN') %shutter is open
    set(hObject,'Value',1)
    set(hObject,'BackgroundColor','g')
    set(hObject,'String','Shutter is OPEN')
elseif strcmp(ShutterStatus(1:6),'CLOSED')
    set(hObject,'Value',0)
    set(hObject,'BackgroundColor','c')
    set(hObject,'String','Shutter is CLOSED')
end
set(handles.txtStatus,'String','Idle','ForegroundColor','k');




function edtCommand_Callback(hObject, eventdata, handles)
% hObject    handle to edtCommand (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edtCommand as text
%        str2double(get(hObject,'String')) returns contents of edtCommand as a double
set(handles.txtStatus,'String','Query Status','ForegroundColor','r');

%mark strings to be updated
set(handles.txtReprate,'ForegroundColor','r');
set(handles.txtCrtemp,'ForegroundColor','r');
set(handles.txtDiodeCurrent,'ForegroundColor','r');
set(handles.txtDiodeMaxCurrent,'ForegroundColor','r');
set(handles.txtIRPower,'ForegroundColor','r');
set(handles.txtDiodeTemp,'ForegroundColor','r');
set(handles.txtTowerTemp,'ForegroundColor','r');

tcpdata = getappdata(handles.output, 'tcpdata'); 
tport=tcpdata.tport;

LaserCmd=get(hObject,'String');

% if the command is laser switching, update Laser toggle button
if LaserCmd(1)=='d' 
    if LaserCmd(2)=='1'
        set(handles.toggleLaser,'BackgroundColor','g');
        set(handles.toggleLaser,'String','Laser switched ON')
    elseif LaserCmd(2)=='0'
        set(handles.toggleLaser,'BackgroundColor','r');
        set(handles.toggleLaser,'String','Laser switched OFF')
    end    
end
% send command and read answer, if any
fprintf(tport,LaserCmd);
pause(4);
LaserAns=tport.UserData;
% if command is a query, display answer 
if LaserCmd(1)=='?'
    set(handles.txtCommandAnswer,'String',LaserAns);
else %if command is not a query, send query to check effect and display answer
    fprintf(tport,['?',LaserCmd(isletter(LaserCmd))]);
    pause(0.5);
    LaserAns=tport.UserData;
    if isempty(deblank(LaserAns)) % some queries require specifying #
        fprintf(tport,['?',LaserCmd(isletter(LaserCmd)),'1']); % # is always 1 for our system 
        pause(0.5);
        LaserAns=tport.UserData;
    end
    set(handles.txtCommandAnswer,'String',LaserAns);
end

% update laser status info
Zeit=clock;
set(handles.txtZeit,'String',datestr(Zeit,13));

% check diode switch status and update laser toggle
fprintf(tport,'?D');
pause(0.5);
DiodeStatus=tport.UserData;
if strcmp(DiodeStatus(1:2),'ON')
    set(handles.toggleLaser,'Value',1)
    set(handles.toggleLaser,'BackgroundColor','g');
    set(handles.toggleLaser,'String','Laser is ON')
elseif strcmp(DiodeStatus(1:3),'OFF')
    set(handles.toggleLaser,'Value',0)
    set(handles.toggleLaser,'BackgroundColor','c');
    set(handles.toggleLaser,'String','Laser is OFF')
end

%check shutter and update shutter toggle
fprintf(tport,'?SHT');
pause(0.5);
ShutterStatus=tport.UserData;
if strcmp(ShutterStatus(1:6),'CLOSED')
    set(handles.toggleShutter,'Value',0)
    set(handles.toggleShutter,'BackgroundColor','c')
    set(handles.toggleShutter,'String','Shutter is CLOSED')
elseif strcmp(ShutterStatus(1:4),'OPEN')
    set(handles.toggleShutter,'Value',1)
    set(handles.toggleShutter,'BackgroundColor','g')
    set(handles.toggleShutter,'String','Shutter is OPEN')
end

%check diode current
fprintf(tport,'?C1'); 
pause(0.5);
DiodeCurrent=tport.UserData;
set(handles.txtDiodeCurrent,'String',DiodeCurrent,'ForegroundColor','k');
%check diode current setting and update power toggle
fprintf(tport,'?CS1'); 
pause(0.5);
DiodeCurrentSet=tport.UserData;
% current depends on power supply
fprintf(tport,'?PSSN'); 
pause(0.5);
PSSN=tport.UserData;
if PSSN(1:6)=='120865'
    if str2double(DiodeCurrentSet)==35.4
        set(handles.togglePower,'Value',1,'String','FULL Power','BackgroundColor','g');
    elseif str2double(DiodeCurrentSet)==14
        set(handles.togglePower,'Value',0,'String','LOW Power','BackgroundColor','c');
    else
        set(handles.togglePower,'Value',0,'String','medium Power','BackgroundColor','c');
    end
elseif PSSN(1:6)=='120881'
    if str2double(DiodeCurrentSet)==36.0
        set(handles.togglePower,'Value',1,'String','FULL Power','BackgroundColor','g');
    elseif str2double(DiodeCurrentSet)==14
        set(handles.togglePower,'Value',0,'String','LOW Power','BackgroundColor','c');
    else
        set(handles.togglePower,'Value',0,'String','medium Power','BackgroundColor','c');
    end
end

% check repetition rate
fprintf(tport,'?Q'); 
pause(0.5);
RepRate=tport.UserData;
set(handles.txtReprate,'String',RepRate,'ForegroundColor','k');

% check crystal temp.
fprintf(tport,'?SHG'); 
pause(0.5);
CrystalTmp=tport.UserData;
set(handles.txtCrtemp,'String',CrystalTmp,'ForegroundColor','k');

% check tower temperature
fprintf(tport,'?TT');
pause(0.5);
TowerTemp=tport.UserData;
set(handles.txtTowerTemp,'String',TowerTemp,'ForegroundColor','k');

% check max. allowed diode current
fprintf(tport,'?DCL1');
pause(0.5);
DiodeMaxCurrent=tport.UserData;
set(handles.txtDiodeMaxCurrent,'String',DiodeMaxCurrent,'ForegroundColor','k');
    
% check diode temp.
fprintf(tport,'?T1');
pause(0.5);
DiodeTemp=tport.UserData;
set(handles.txtDiodeTemp,'String',DiodeTemp,'ForegroundColor','k');

% check IR power
fprintf(tport,'?P');
pause(0.5);
IRPower=tport.UserData;
set(handles.txtIRPower,'String',IRPower,'ForegroundColor','k');

set(handles.txtStatus,'String','Idle','ForegroundColor','k');



% --- Executes on button press in pubExit.
function pubExit_Callback(hObject, eventdata, handles)
% hObject    handle to pubExit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% shut down tcpip connection
tcpdata = getappdata(handles.output, 'tcpdata'); 
if isvalid(tcpdata.tport)
    fclose(tcpdata.tport);
    delete(tcpdata.tport);
    echotcpip('off');
end
close(handles.figure1);





% --- Executes on button press in Update.
function Update_Callback(hObject, eventdata, handles)
% hObject    handle to Update (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(handles.txtStatus,'String','Query Status','ForegroundColor','r');

tcpdata = getappdata(handles.output, 'tcpdata'); 
tport=tcpdata.tport;

Zeit=clock;
set(handles.txtZeit,'String',datestr(Zeit,13));

%mark strings to be updated
set(handles.txtReprate,'ForegroundColor','r');
set(handles.txtCrtemp,'ForegroundColor','r');
set(handles.txtDiodeCurrent,'ForegroundColor','r');
set(handles.txtDiodeMaxCurrent,'ForegroundColor','r');
set(handles.txtIRPower,'ForegroundColor','r');
set(handles.txtDiodeTemp,'ForegroundColor','r');
set(handles.txtTowerTemp,'ForegroundColor','r');

% check diode switch status and update laser toggle
fprintf(tport,'?D');
pause(0.5);
DiodeStatus=tport.UserData;
if strcmp(DiodeStatus(1:2),'ON')
    set(handles.toggleLaser,'Value',1)
    set(handles.toggleLaser,'BackgroundColor','g');
    set(handles.toggleLaser,'String','Laser is ON')
elseif strcmp(DiodeStatus(1:3),'OFF')
    set(handles.toggleLaser,'Value',0)
    set(handles.toggleLaser,'BackgroundColor','c');
    set(handles.toggleLaser,'String','Laser is OFF')
end

% check shutter and update shutter toggle
fprintf(tport,'?SHT');
pause(0.5);
ShutterStatus=tport.UserData;
if strcmp(ShutterStatus(1:6),'CLOSED')
    set(handles.toggleShutter,'Value',0)
    set(handles.toggleShutter,'BackgroundColor','c')
    set(handles.toggleShutter,'String','Shutter is CLOSED')
elseif strcmp(ShutterStatus(1:4),'OPEN')
    set(handles.toggleShutter,'Value',1)
    set(handles.toggleShutter,'BackgroundColor','g')
    set(handles.toggleShutter,'String','Shutter is OPEN')
end

% check diode current
fprintf(tport,'?C1'); 
pause(0.5);
DiodeCurrent=tport.UserData;
set(handles.txtDiodeCurrent,'String',DiodeCurrent,'ForegroundColor','k');
%check diode current setting and update power toggle
fprintf(tport,'?CS1'); 
pause(0.5);
DiodeCurrentSet=tport.UserData;
% current depends on power supply
fprintf(tport,'?PSSN'); 
pause(0.5);
PSSN=tport.UserData;
if PSSN(1:6)=='120865'
    if str2double(DiodeCurrentSet)==35.4
        set(handles.togglePower,'Value',1,'String','FULL Power','BackgroundColor','g');
    elseif str2double(DiodeCurrentSet)==14
        set(handles.togglePower,'Value',0,'String','LOW Power','BackgroundColor','c');
    else
        set(handles.togglePower,'Value',0,'String','medium Power','BackgroundColor','c');
    end
elseif PSSN(1:6)=='120881'
    if str2double(DiodeCurrentSet)==36.0
        set(handles.togglePower,'Value',1,'String','FULL Power','BackgroundColor','g');
    elseif str2double(DiodeCurrentSet)==14
        set(handles.togglePower,'Value',0,'String','LOW Power','BackgroundColor','c');
    else
        set(handles.togglePower,'Value',0,'String','medium Power','BackgroundColor','c');
    end
end
    
% check repetition rate
fprintf(tport,'?Q'); 
pause(0.5);
RepRate=tport.UserData;
set(handles.txtReprate,'String',RepRate,'ForegroundColor','k');

% check crystal temperature
fprintf(tport,'?SHG'); 
pause(0.5);
CrystalTmp=tport.UserData;
set(handles.txtCrtemp,'String',CrystalTmp,'ForegroundColor','k');

% check tower temperature
fprintf(tport,'?TT');
pause(0.5);
TowerTemp=tport.UserData;
set(handles.txtTowerTemp,'String',TowerTemp,'ForegroundColor','k');

% check max. allowed diode current
fprintf(tport,'?DCL1');
pause(0.5);
DiodeMaxCurrent=tport.UserData;
set(handles.txtDiodeMaxCurrent,'String',DiodeMaxCurrent,'ForegroundColor','k');
    
% check diode temperature
fprintf(tport,'?T1');
pause(0.5);
DiodeTemp=tport.UserData;
set(handles.txtDiodeTemp,'String',DiodeTemp,'ForegroundColor','k');

% check IR power
fprintf(tport,'?P');
pause(0.5);
IRPower=tport.UserData;
set(handles.txtIRPower,'String',IRPower,'ForegroundColor','k');

set(handles.txtStatus,'String','Idle','ForegroundColor','k');



% --- Executes on button press in togglePower.
function togglePower_Callback(hObject, eventdata, handles)
% hObject    handle to togglePower (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglePower
set(handles.txtStatus,'String','Query Status','ForegroundColor','r');

tcpdata = getappdata(handles.output, 'tcpdata'); 
tport=tcpdata.tport;

% mark strings to be updated
set(handles.txtDiodeCurrent,'ForegroundColor','r');
set(handles.txtIRPower,'ForegroundColor','r');
set(handles.txtCrtemp,'ForegroundColor','r');
set(handles.txtDiodeTemp,'ForegroundColor','r');
set(handles.txtTowerTemp,'ForegroundColor','r');

% current depends on power supply
fprintf(tport,'?PSSN'); 
pause(0.5);
PSSN=tport.UserData;
if PSSN(1:6)=='120865'
    if get(hObject,'Value')
        fprintf(tport,'C1:35.4'); % full power
    else
        fprintf(tport,'C1:14'); % lowest green power 
    end    
elseif PSSN(1:6)=='120881'
    if get(hObject,'Value')
        fprintf(tport,'C1:36.0'); % full power
    else
        fprintf(tport,'C1:14'); % lowest green power 
    end    
end

% check effect
pause(0.5)
fprintf(tport,'?CS1'); 
pause(0.5);
DiodeCurrentSet=tport.UserData;
% current depends on power supply
if (PSSN(1:6)=='120865' & str2double(DiodeCurrentSet)==35.4)...
    | (PSSN(1:6)=='120881' & str2double(DiodeCurrentSet)==36.0)
    set(handles.togglePower,'Value',1)
    set(handles.togglePower,'BackgroundColor','g');
    set(handles.togglePower,'String','FULL Power')
else
    set(handles.togglePower,'Value',0)
    set(handles.togglePower,'BackgroundColor','c');
    set(handles.togglePower,'String','LOW Power')
    if str2double(DiodeCurrentSet)~=14
        set(handles.togglePower,'String','medium Power')
    end
end

% check crystal temp.
fprintf(tport,'?SHG'); 
pause(0.5);
CrystalTmp=tport.UserData;
set(handles.txtCrtemp,'String',CrystalTmp,'ForegroundColor','k');

% check tower temperature
fprintf(tport,'?TT');
pause(0.5);
TowerTemp=tport.UserData;
set(handles.txtTowerTemp,'String',TowerTemp,'ForegroundColor','k');

% check diode temp.
fprintf(tport,'?T1');
pause(0.5);
DiodeTemp=tport.UserData;
set(handles.txtDiodeTemp,'String',DiodeTemp,'ForegroundColor','k');

% check diode current
fprintf(tport,'?C1'); 
pause(0.5);
DiodeCurrent=tport.UserData;
set(handles.txtDiodeCurrent,'String',DiodeCurrent,'ForegroundColor','k');

% check IR power
fprintf(tport,'?P');
pause(0.5);
IRPower=tport.UserData;
set(handles.txtIRPower,'String',IRPower,'ForegroundColor','k');

set(handles.txtStatus,'String','Idle','ForegroundColor','k');
