function varargout = Laser(varargin)
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

% Last Modified by GUIDE v2.5 28-Jan-2005 20:04:16

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
%tcpdata = getappdata(gcbf, 'tcpdata'); 
echotcpip('on',10001);
tcpdata.tport=tcpip('10.111.111.20',10001);
set(tcpdata.tport,'ReadAsyncMode','continuous');
set(tcpdata.tport,'BytesAvailableFcn',{'tcpipdatacallback'});
fopen(tcpdata.tport);
%setup Timer function
%handles.LaserTimer = timer('ExecutionMode','fixedDelay',...
%          'Period',2,...    
%          'BusyMode','drop',...
%          'TimerFcn', {@LaserRefresh,handles});   

%data.LaserTimer=handles.LaserTimer;

% Update handles structure
guidata(hObject, handles);

setappdata(handles.output, 'tcpdata', tcpdata); 

% UIWAIT makes Laser wait for user response (see UIRESUME)
% uiwait(handles.figure1);


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
tcpdata = getappdata(handles.output, 'tcpdata'); 
tport=tcpdata.tport;
if get(hObject,'Value')
    fprintf(tport,'D1'); 
    pause(0.2);
else
    fprintf(tport,'D0');
    pause(0.2);
end
fprintf(tport,'?D'); 
pause(0.2);
DiodeStatus=tport.UserData;
if strcmp(DiodeStatus(1:2),'ON')
    set(hObject,'Value',1)
    set(hObject,'BackgroundColor','r');
    set(hObject,'String','Laser is ON')
elseif strcmp(DiodeStatus(1:3),'OFF')
    set(hObject,'Value',0)
    set(hObject,'BackgroundColor','g');
    set(hObject,'String','Laser is OFF')
end


% --- Executes on button press in toggleShutter.
function toggleShutter_Callback(hObject, eventdata, handles)
% hObject    handle to toggleShutter (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
tcpdata = getappdata(handles.output, 'tcpdata'); 
tport=tcpdata.tport;
if get(hObject,'Value')
    fprintf(tport,'SHT1'); 
    pause(0.2);
else
    fprintf(tport,'SHT0');
    pause(0.2);
end
fprintf(tport,'?SHT'); 
pause(0.2);
ShutterStatus=tport.UserData;
if strcmp(ShutterStatus(1:4),'OPEN') %shutter is open
    set(hObject,'Value',1)
    set(hObject,'BackgroundColor','r')
elseif strcmp(ShutterStatus(1:6),'CLOSED')
    set(hObject,'Value',0)
    set(hObject,'BackgroundColor','g')
end




function edtCommand_Callback(hObject, eventdata, handles)
% hObject    handle to edtCommand (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edtCommand as text
%        str2double(get(hObject,'String')) returns contents of edtCommand as a double
tcpdata = getappdata(handles.output, 'tcpdata'); 
tport=tcpdata.tport;
LaserCmd=get(hObject,'String');
fprintf(tport,LaserCmd);
pause(0.2);
if LaserCmd(1)=='?'
    LaserAns=tport.UserData;
    set(handles.txtCommandAnswer,'String',LaserAns);
else
    fprintf(tport,['?',LaserCmd(isletter(LaserCmd))]);
    pause(0.2);
    LaserAns=tport.UserData;
    set(handles.txtCommandAnswer,'String',LaserAns);
end


% --- Executes on button press in pubExit.
function pubExit_Callback(hObject, eventdata, handles)
% hObject    handle to pubExit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% shut down tcpip connection
tcpdata = getappdata(handles.output, 'tcpdata'); 
tport=tcpdata.tport;
fclose(tcpdata.tport);
delete(tcpdata.tport);
echotcpip('off');
% shut down timer
%stop(handles.LaserTimer);
%delete(handles.LaserTimer);
% close figure
close(handles.figure1);





% --- Executes on button press in Update.
function Update_Callback(hObject, eventdata, handles)
% hObject    handle to Update (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
tcpdata = getappdata(handles.output, 'tcpdata'); 
tport=tcpdata.tport;

Zeit=clock;
set(handles.txtZeit,'String',datestr(Zeit,13));
set(handles.txtStatus,'String','Query Status');

% check diode switch status

fprintf(tport,'?D');
%disp('?D')
pause(0.2);
DiodeStatus=tport.UserData;
if strcmp(DiodeStatus(1:2),'ON')
    set(handles.toggleLaser,'Value',1)
    set(handles.toggleLaser,'BackgroundColor','r');
    set(handles.toggleLaser,'String','Laser is ON')
elseif strcmp(DiodeStatus(1:3),'OFF')
    set(handles.toggleLaser,'Value',0)
    set(handles.toggleLaser,'BackgroundColor','g');
    set(handles.toggleLaser,'String','Laser is OFF')
end

%check shutter
fprintf(tport,'?SHT');
%disp('?SHT')
pause(0.2);
ShutterStatus=tport.UserData;
if strcmp(ShutterStatus(1:6),'CLOSED') %shutter is open
    set(handles.toggleShutter,'Value',0)
    set(handles.toggleShutter,'BackgroundColor','g')
elseif strcmp(ShutterStatus(1:4),'OPEN')
    set(handles.toggleShutter,'Value',1)
    set(handles.toggleShutter,'BackgroundColor','r')
end

fprintf(tport,'?Q'); 
%disp('?Q')
pause(0.2);
RepRate=tport.UserData;
set(handles.txtReprate,'String',RepRate);

fprintf(tport,'?SHG'); 
%disp('?SHG')
pause(0.2);
CrystalTmp=tport.UserData;
set(handles.txtCrtemp,'String',CrystalTmp);

fprintf(tport,'?C'); 
%disp('?C')
pause(0.2);
DiodeCurrent=tport.UserData;
set(handles.txtDiodeCurrent,'String',DiodeCurrent);
    
fprintf(tport,'?DCL');
%disp('?DCL')
pause(0.2);
DiodeMaxCurrent=tport.UserData;
set(handles.txtDiodeMaxCurrent,'String',DiodeMaxCurrent);
    
fprintf(tport,'?T');
%disp('?T')
pause(0.2);
DiodeTemp=tport.UserData;
set(handles.txtDiodeTemp,'String',DiodeTemp);

fprintf(tport,'?DP');
%disp('?DP')
pause(0.2);
DiodePower=tport.UserData;
set(handles.txtDiodePower,'String',DiodePower);

set(handles.txtStatus,'String','Idle');

