function varargout = etalon(varargin)
% ETALON M-file for etalon.fig
%      ETALON, by itself, creates a new ETALON or raises the existing
%      singleton*.
%
%      H = ETALON returns the handle to a new ETALON or the handle to
%      the existing singleton*.
%
%      ETALON('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in ETALON.M with the given input arguments.
%
%      ETALON('Property','Value',...) creates a new ETALON or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before etalon_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to etalon_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Copyright 2002-2003 The MathWorks, Inc.

% Edit the above text to modify the response to help etalon

% Last Modified by GUIDE v2.5 18-Jan-2005 16:40:06

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @etalon_OpeningFcn, ...
                   'gui_OutputFcn',  @etalon_OutputFcn, ...
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


% --- Executes just before etalon is made visible.
function etalon_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to etalon (see VARARGIN)

% Choose default command line output for etalon
handles.output = hObject;

if length(varargin)==2 & varargin{1}=='handle'
    handles.parenthandle=str2double(varargin{2});
end

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes etalon wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = etalon_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;



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
