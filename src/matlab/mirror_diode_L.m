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

% Last Modified by GUIDE v2.5 21-Jul-2008 14:02:06

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

% open tcpip port for communication with Laser,
 picotport=tcpip('10.111.111.28',8100);
 set(picotport,'ReadAsyncMode','continuous');
 set(picotport,'BytesAvailableFcn',{'tcpipdatacallback'});
 set(picotport,'Terminator','CR/LF');
 set(picotport,'BytesAvailableFcnMode','Terminator');
 
 try
 fopen(picotport);
 handles.picotport=picotport;
 data.picotport=handles.picotport;
 set(handles.txtmirrors,'String','communication INITIALIZED','BackgroundColor','g');
 %set(handles.pb_Exit,'backgroundcolor','g');
 
catch  
 % if communication with laser did not work
    fclose(picotport);
    delete(picotport);
    clear('picotport');
    set(handles.txtmirrors,'String','communication FAILED','BackgroundColor','r');
 %   set(handles.pb_Exit,'backgroundcolor','r');
 end


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
set(handles.textpos,'String','0');

% --- Executes on button press in radiover.
function radiover_Callback(hObject, eventdata, handles)
% hObject    handle to radiover (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiover

if get(hObject,'Value')
    set(handles.radiohor,'Value',0);
end
set(handles.textpos,'String','0');

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

steps=uint16(str2double(get(hObject,'String')));
if isnan(steps)
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','white');
    set(hObject,'string',num2str(steps));
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
picotport=handles.picotport;
if isfield(handles,'picotport')
    fclose(handles.picotport);
    delete(handles.picotport);
    set(handles.txtmirrors,'String','communication STOPPED','BackgroundColor','r');
end;


% --- Executes on button press in pushbuttongo.
function pushbuttongo_Callback(hObject, eventdata, handles)
% hObject    handle to pushbuttongo (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

%gandalfdata = getappdata(handles.parenthandle, 'gandalfdata');
%statusData=gandalfdata.statusData;
%col=gandalfdata.col;
%data = getappdata(handles.output, 'diodelaserdata');
%lastrow=data.lastrow;

set(handles.textPos,'BackgroundColor','r');
set(handles.pushgo,'BackgroundColor','r');

%serport=handles.serport;
picotport=handles.picotport;
mirror=get(handles.popupmirror,'Value')-1;
axis=get(handles.radiover,'Value');

steps=str2num(get(handles.editsteps,'String'));
forw=get(handles.radiofor,'Value');
if forw==0 
    steps=-steps;
end

%system(['/gollum/bin/eCmd @gollum s mirrorgoto ',num2str(mirror),' ',num2str(axis),' ',num2str(steps)]);
%disp(['/gollum/bin/eCmd @gollum s mirrorgoto ',num2str(mirror),' ',num2str(axis),' ',num2str(steps)]);

%switch mirror
%    case 0
%        if axis==0
%            mirrorstr='Hi1X';%higher Picomotor
%        else
%            mirrorstr='Hi1Y';
%        end            
%    case 1
%        if axis==0
%            mirrorstr='Lo2X';%lower Picomotor
%        else
%            mirrorstr='Lo2Y';
%        end            
     
%end

%eval(['Mirrorhelp=bitget(uint16(statusData(lastrow,col.Mirror',mirrorstr,'AxisHi)),16);']);
%eval(['currpos=double(statusData(lastrow,col.Mirror',mirrorstr,'AxisHi)).*65536+double(statusData(lastrow,col.Mirror',mirrorstr,'AxisLo));']);
%if Mirrorhelp==1
%    currpos=bitset(floor(currpos),32,0)-2^32/2;
%end
% check if motor is still moving
%while double(statusData(lastrow,col.MirrorMovingFlags))~=0
%    set(handles.textPos,'String',num2str(currpos),'BackgroundColor','r');
%end
set(handles.textPos,'String',num2str(currpos),'BackgroundColor','w');
set(handles.pushgo,'BackgroundColor','w');


% --- Executes on button press in pushbuttonstop.
function pushbuttonstop_Callback(hObject, eventdata, handles)
% hObject    handle to pushbuttonstop (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

%system(['/gollum/bin/eCmd @gollum s mirrorstop']);
%disp(['/gollum/bin/eCmd @gollum s mirrorstop']);

%gandalfdata = getappdata(handles.parenthandle, 'gandalfdatadata');
%statusData=gandalfdata.statusData;
%col=horusdata.col;
%data = getappdata(handles.output, 'mirror_diode_Ldata');
%lastrow=data.lastrow;

%mirror=get(handles.popupmirror,'Value')-1;
%axis=get(handles.radiover,'Value');

%switch mirror
%    case 0
%        if axis==0
%            mirrorstr='Hi1X';%higher picomotor
%        else
%            mirrorstr='Hi1Y';
%        end            
%    case 1
%        if axis==0
%            mirrorstr='Lo2X';%lower picomotor
%        else
%            mirrorstr='Lo2Y';
%        end            
%end

%eval(['Mirrorhelp=bitget(uint16(statusData(lastrow,col.Mirror',mirrorstr,'AxisHi)),16);']);
%eval(['currpos=double(statusData(lastrow,col.Mirror',mirrorstr,'AxisHi)).*65536+double(statusData(lastrow,col.Mirror',mirrorstr,'AxisLo));']);
%if Mirrorhelp==1
%    currpos=bitset(floor(currpos),32,0)-2^32/2;
%end
% check if motor is still moving
%while double(statusData(lastrow,col.MirrorMovingFlags))~=0
%    set(handles.textPos,'String',num2str(currpos),'BackgroundColor','r');
%end
%set(handles.textPos,'String',num2str(currpos),'BackgroundColor','w');
%set(handles.pushgo,'BackgroundColor','w');



function editmirrorcommand_Callback(hObject, eventdata, handles)
% hObject    handle to editmirrorcommand (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editmirrorcommand as text
%        str2double(get(hObject,'String')) returns contents of editmirrorcommand as a double


picotport=handles.picotport;
%ll=handles.ll;
MirrorCmd=get(hObject,'String');
MirrorCmd=char(MirrorCmd);
set(handles.txtmirrors,'String',MirrorCmd);
fprintf(picotport,MirrorCmd);
pause(1);
L_Answer=picotport.UserData;
L_Answer=char(L_Answer);
tport.UserData=[];
set(handles.txtmirrors,'String',L_Answer);

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


