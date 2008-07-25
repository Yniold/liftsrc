function varargout = mirror_diode_L(varargin)
% MIRROR_DIODE_L M-file for mirror_diode_L.fig
%      MIRROR_DIODE_L, by itself, creates a new MIRROR_DIODE_L or raises the existing
%      singleton*.
%
%      H = MIRROR_DIODE_L returns the handle to a new MIRROR_DIODE_L or the handle to
%      the existing singleton*.
%
%      MIRROR_DIODE_L('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in MIRROR_DIODE_L.M with the given input arguments.
%
%      MIRROR_DIODE_L('Property','Value',...) creates a new MIRROR_DIODE_L or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before mirror_diode_L_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to mirror_diode_L_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help mirror_diode_L

% Last Modified by GUIDE v2.5 24-Jul-2008 10:31:32

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @mirror_diode_L_OpeningFcn, ...
                   'gui_OutputFcn',  @mirror_diode_L_OutputFcn, ...
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


% --- Executes just before mirror_diode_L is made visible.
function mirror_diode_L_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to mirror_diode_L (see VARARGIN)

% Choose default command line output for mirror_diode_L
handles.output = hObject;

% choose Text size
set(double(get(handles.figure1,'Children')),'FontSize',8)

% get horus handle
if length(varargin)==2 & varargin{1}=='handle'
    handles.parenthandle=str2double(varargin{2});
end

% Update handles structure
guidata(hObject, handles);

serport=serial('COM1','BaudRate',19200,'Terminator','CR');
set(serport,'BytesAvailableFcn',{'serialdatacallback'});

try fopen(serport);
    handles.serport=serport;
    data.serport=handles.serport;
    set(handles.txtmirrors,'String','OPENED','BackgroundColor','g');
catch 
    fclose(serport);
    delete(serport);
    %rmfield(handles,'serport');
    set(handles.txtmirrors,'String','FAILED','BackgroundColor','r');
end;

% UIWAIT makes mirror_diode_L wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = mirror_diode_L_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on selection change in menu_mirrorselect.
function menu_mirrorselect_Callback(hObject, eventdata, handles)
% hObject    handle to menu_mirrorselect (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns menu_mirrorselect contents as cell array
%        contents{get(hObject,'Value')} returns selected item from menu_mirrorselect
driver=get(hObject,'string');
handles.driver=driver;

%if strcmp(driver,'high')
%    handles.driver=0;
%else
%    handles.driver=1;
%end

% --- Executes during object creation, after setting all properties.
function menu_mirrorselect_CreateFcn(hObject, eventdata, handles)
% hObject    handle to menu_mirrorselect (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
set(hObject,'Value',0);

end


% --- Executes on button press in radiohor.
function radiohor_Callback(hObject, eventdata, handles)
% hObject    handle to radiohor (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiohor

if get(hObject,'Value')
    set(handles.radiover,'Value',0);
end
%set(handles.textpos,'String','0');

% --- Executes on button press in radiover.
function radiover_Callback(hObject, eventdata, handles)
% hObject    handle to radiover (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiover

if get(hObject,'Value')
    set(handles.radiohor,'Value',0);
end
%set(handles.textpos,'String','0');

% --- Executes on button press in radiofor.
function radiofor_Callback(hObject, eventdata, handles)
% hObject    handle to radiofor (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiofor

if get(hObject,'Value')
    set(handles.radiorev,'Value',0);
end


% --- Executes on button press in radiorev.
function radiorev_Callback(hObject, eventdata, handles)
% hObject    handle to radiorev (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiorev

if get(hObject,'Value')
    set(handles.radiofor,'Value',0);
end

function txtsteps_Callback(hObject, eventdata, handles)
% hObject    handle to txtsteps (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of txtsteps as text
%        str2double(get(hObject,'String')) returns contents of txtsteps as a double

steps=get(hObject,'String');

if isnan(steps)
    set(hObject,'BackgroundColor','red');
    set(handles.txtmirrors,'String','Invalid Input','BackgroundColor','r');
else
    steps=uint16(str2double(steps));
    set(hObject,'BackgroundColor','white');
    set(hObject,'string',num2str(steps));
    handles.steps=steps;
end


% --- Executes during object creation, after setting all properties.
function txtsteps_CreateFcn(hObject, eventdata, handles)
% hObject    handle to txtsteps (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end




% --- Executes on button press in pushbuttonExit.
function pushbuttonExit_Callback(hObject, eventdata, handles)
% hObject    handle to pushbuttonExit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

%stop(handles.ActTimer);
%delete(handles.ActTimer);
%if isvalid(handles.serport)
%    fclose(handles.serport);
%    delete(handles.serport);
%end;

%picotport=handles.picotport;
handles.serport=data.serport;
serport=handles.serport;
%if isfield(handles,'serport')
    fclose(serport);
    delete(serport);
    set(handles.txtmirrors,'String','communication STOPPED','BackgroundColor','r');
%end;


% --- Executes on button press in pushbuttongo.
function pushbuttongo_Callback(hObject, eventdata, handles)
% hObject    handle to pushbuttongo (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

handles.serport=data.serport;
serport=handles.serport;

mirror=get(handles.menu_mirrorselect,'Value');
%axis=get(handles.radiohor,'Value');

steps=str2num(get(handles.editsteps,'String'));
forw=get(handles.radiofor,'Value');

if forw==0 
    steps=-steps;
end

if handles.radiover==0
    MirrorCmd=['chl a',mirror,'=1'];
    MirrorCmd=char(MirrorCmd);
    fprintf(serport,MirrorCmd);
else
    MirrorCmd=['chl a',mirror,'=2'];
    MirrorCmd=char(MirrorCmd);
    fprintf(serport,MirrorCmd);
end

%set(handles.textPos,'String',num2str(currpos),'BackgroundColor','w');
set(handles.pushgo,'BackgroundColor','w');

MotorCmd=['REL a',mirror,' ',steps,' g'];
MotorCmd=char(MotorCmd);
fprintf(serport,MotorCmd);


% --- Executes on button press in pushbuttonstop.
function pushbuttonstop_Callback(hObject, eventdata, handles)
% hObject    handle to pushbuttonstop (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

handles.serport=data.serport;
serport=handles.serport;
Input='hal';
Input=char(Input);
set(handles.txtmirror,'String',Input);
fprintf(serport,Input);
pause(1)

 
function editmirrorcommand_Callback(hObject, eventdata, handles)
% hObject    handle to editmirrorcommand (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editmirrorcommand as text
%        str2double(get(hObject,'String')) returns contents of editmirrorcommand as a double

handles.serport=data.serport;
serport=handles.serport;
MirrorCmd=get(hObject,'String');
MirrorCmd=char(MirrorCmd);
set(handles.txtmirror,'String',MirrorCmd);
fprintf(serport,MirrorCmd);
pause(1);
Mirror_Answer=serport.UserData;
Mirror_Answer=char(Mirror_Answer);
tport.UserData=[];
set(handles.txtmirror,'String',Mirror_Answer);

% --- Executes during object creation, after setting all properties.
function editmirrorcommand_CreateFcn(hObject, eventdata, handles)
% hObject    handle to editmirrorcommand (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


