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

% Last Modified by GUIDE v2.5 14-Dec-2004 16:19:38

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

% open the serial line for communication with Laser
if (computer=='PCWIN') 
   handles.serLaser = serial('COM10', 'BaudRate', 9600); 
else
    handles.serLaser = serial('/dev/ttyS0', 'BaudRate', 9600);
end
fopen(handles.serLaser); 


%setup Timer function for update Laserinfo
handles.LaserTimer = timer('ExecutionMode','fixedDelay',...
          'Period',2,...    
          'BusyMode','drop',...
          'TimerFcn', {@LaserRefresh,handles});   

start(handles.LaserTimer);
% Update handles structure
guidata(hObject, handles);


% UIWAIT makes Laser wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% update Laserdata function
function LaserRefresh(arg1, arg2, GUI_handles)

    ShutterStatus='xxx';
    DiodeStatus='xxx';
    DiodeCurrent=0;
    DiodeMaxCurrent=0;
    DiodePower=0;
    DiodeTemp=0;

    %stop(arg1);
    Zeit=clock;
    set(GUI_handles.txtZeit,'String',datestr(Zeit,13));

    set(GUI_handles.txtStatus,'String','Query Status');

    while (GUI_handles.serLaser.BytesAvailable>0)
        dummy=fgetl(GUI_handles.serLaser);
    end
    fprintf(GUI_handles.serLaser,'?SHT;'); ShutterStatus=fgetl(GUI_handles.serLaser);
    set(GUI_handles.txtLenSerInput,'String',GUI_handles.serLaser.BytesAvailable);
    %fprintf('Number of Bytes at RS232 input %d', GUI_handles.serLaser.BytesAvailable);    
    set(GUI_handles.txtShutterStatus,'String',ShutterStatus);

    fprintf(GUI_handles.serLaser,'?D;'); DiodeStatus=fgetl(GUI_handles.serLaser);
    set(GUI_handles.txtDiodeStatus,'String',DiodeStatus);
    
    fprintf(GUI_handles.serLaser,'?C1;'); DiodeCurrent=fgetl(GUI_handles.serLaser);
    set(GUI_handles.txtDiodeCurrent,'String',DiodeCurrent);
    
    fprintf(GUI_handles.serLaser,'?DCL1;'); DiodeMaxCurrent=fgetl(GUI_handles.serLaser);
    set(GUI_handles.txtDiodeMaxCurrent,'String',DiodeMaxCurrent);
    
    fprintf(GUI_handles.serLaser,'?T1;'); DiodeTemp=fgetl(GUI_handles.serLaser);
    set(GUI_handles.txtDiodeTemp,'String',DiodeTemp);

    fprintf(GUI_handles.serLaser,'?DP1;'); DiodePower=fgetl(GUI_handles.serLaser);
    set(GUI_handles.txtDiodePower,'String',DiodePower);
    
%    fprintf(GUI_handles.serLaser,'?FH;'); LaserHistory=scanstr(GUI_handles.serLaser,';');
%    set(GUI_handles.txtDiodePower,'String',DiodePower);

    
    
    
    set(GUI_handles.txtStatus,'String','Idle');
    
% --- Outputs from this function are returned to the command line.
function varargout = Laser_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in pubLaserOn.
function pubLaserOn_Callback(hObject, eventdata, handles)
% hObject    handle to pubLaserOn (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

fprintf(handles.serLaser,'ON');
LaserAns=fgetl(handles.serLaser);
set(handles.txtCommandAnswer,'String',LaserAns);


% --- Executes on button press in pubLaserOff.
function pubLaserOff_Callback(hObject, eventdata, handles)
% hObject    handle to pubLaserOff (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
fprintf(handles.serLaser,'OFF');
LaserAns=fgetl(handles.serLaser);
set(handles.txtCommandAnswer,'String',LaserAns);



function edtCommand_Callback(hObject, eventdata, handles)
% hObject    handle to edtCommand (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edtCommand as text
%        str2double(get(hObject,'String')) returns contents of edtCommand as a double

LaserCmd=get(hObject,'String')
fprintf(handles.serLaser,LaserCmd);
LaserAns=fgetl(handles.serLaser);
set(handles.txtCommandAnswer,'String',LaserAns);

% --- Executes during object creation, after setting all properties.
function edtCommand_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edtCommand (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end

% --- Executes on button press in pubExit.
function pubExit_Callback(hObject, eventdata, handles)
% hObject    handle to pubExit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

    fclose(handles.serLaser);
    delete(handles.serLaser);
    stop(handles.LaserTimer);
    delete(handles.LaserTimer);
    close(handles.figure1);


% --- Executes on selection change in lstHistory.
function lstHistory_Callback(hObject, eventdata, handles)
% hObject    handle to lstHistory (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns lstHistory contents as cell array
%        contents{get(hObject,'Value')} returns selected item from lstHistory


% --- Executes during object creation, after setting all properties.
function lstHistory_CreateFcn(hObject, eventdata, handles)
% hObject    handle to lstHistory (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: listbox controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


