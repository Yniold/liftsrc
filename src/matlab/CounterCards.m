function varargout = CounterCards(varargin)
% GUIDETEMPLATE0 M-file for guidetemplate0.fig
%      GUIDETEMPLATE0, by itself, creates a new GUIDETEMPLATE0 or raises the existing
%      singleton*.
%
%      H = GUIDETEMPLATE0 returns the handle to a new GUIDETEMPLATE0 or the handle to
%      the existing singleton*.
%
%      GUIDETEMPLATE0('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in GUIDETEMPLATE0.M with the given input arguments.
%
%      GUIDETEMPLATE0('Property','Value',...) creates a new GUIDETEMPLATE0 or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before guidetemplate0_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to guidetemplate0_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Copyright 2002-2003 The MathWorks, Inc.

% Edit the above text to modify the response to help guidetemplate0

% Last Modified by GUIDE v2.5 11-Feb-2005 12:25:42

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @CounterCards_OpeningFcn, ...
                   'gui_OutputFcn',  @CounterCards_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);

if nargin && isstr(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before guidetemplate0 is made visible.
function CounterCards_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to guidetemplate0 (see VARARGIN)

% Choose default command line output for guidetemplate0
handles.output = hObject;

% get horus handle
if length(varargin)==2 & varargin{1}=='handle'
    handles.parenthandle=str2double(varargin{2});
end

%setup Timer function
handles.Timer = timer('ExecutionMode','fixedDelay',...
      'Period',0.7,...    
      'BusyMode','drop',...
      'TimerFcn', {@GateRefresh,handles});   

data.Timer=handles.Timer;

% Update handles structure
guidata(hObject, handles);
setappdata(handles.output, 'Gatedata', data);
start(handles.Timer);


function GateRefresh(arg1,arg2,handles)

horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
AvgData=horusdata.AvgData;

% Calculate time as sum of day, hour, min, etc.
statustime=double(statusData(:,1))+ ...
           double(statusData(:,2))./1.0+ ...
           double(statusData(:,3))./24.0+...
           double(statusData(:,4))./1440.0+...
           double(statusData(:,5))./86400.0;

[SortZeit,indexZeit]=sort(statustime);
maxLen=size(statustime,1);
lastrow=indexZeit(maxLen);

set(edMaster,'String',num2str(statusData(lastrow,15)));
switch handles.device
    case 0
        set(handles.toggleGain,'Value',0,'String','Gain is OFF','BackgroundColor','c');
        set(handles.edGain,'String','NA');
        set(handles.edGainWidth,'String','NA');
        set(handles.edCounter,'String',num2str(statusData(lastrow,16)));
    case 1
        if bitget(statusData(lastrow,226),16)
            set(handles.toggleGain,'Value',1,'String','Gain is ON','BackgroundColor','g');
        else
            set(handles.toggleGain,'Value',0,'String','Gain is OFF','BackgroundColor','c');
        end
        set(handles.edGain,'String',num2str(statusData(lastrow,226)));
        set(handles.edGainWidth,'String',num2str(statusData(lastrow,227)));
        set(handles.edCounter,'String',num2str(statusData(lastrow,225)));
    case 2
        if bitget(statusData(lastrow,435),16);
            set(handles.toggleGain,'Value',1,'String','Gain is ON','BackgroundColor','g');
        else
            set(handles.toggleGain,'Value',0,'String','Gain is OFF','BackgroundColor','c');
        end
        set(handles.edGain,'String',num2str(statusData(lastrow,435)));
        set(handles.edGainWidth,'String',num2str(statusData(lastrow,436)));
        set(handles.edCounter,'String',num2str(statusData(lastrow,434)));
end




% --- Outputs from this function are returned to the command line.
function varargout = CounterCards_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in pushbutton1.
function MaskStart_Callback(hObject, eventdata, handles)
% hObject    handle to MaskStart (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of MaskStart as text
%        str2double(get(hObject,'String')) returns contents of MaskStart as a double
MaskStartChannel=uint8(str2double(get(hObject,'String')));
if isnan(MaskStartChannel)| MaskStartChannel<1 | MaskStartChannel>160
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','white');
    set(hObject,'string',num2str(MaskStartChannel));
end


function MaskStop_Callback(hObject, eventdata, handles)
% hObject    handle to MaskStop (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of MaskStop as text
%        str2double(get(hObject,'String')) returns contents of MaskStop as a double
MaskStopChannel=uint8(str2double(get(hObject,'String')));
if isnan(MaskStopChannel)| MaskStopChannel<1 | MaskStopChannel>160
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','white');
    set(hObject,'string',num2str(MaskStopChannel));
end



% --- Executes on button press in ToggleMaskUnmask.
function ToggleMaskUnmask_Callback(hObject, eventdata, handles)
% hObject    handle to ToggleMaskUnmask (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of ToggleMaskUnmask
set(hObject,'BackgroundColor','white');
MaskOption=get(hObject,'Value');
if MaskOption==get(hObject,'Max')
    set(hObject,'String','unmask');
else
    set(hObject,'String','mask');
end

% --- Executes on button press in MaskApply.
function MaskApply_Callback(hObject, eventdata, handles)
% hObject    handle to MaskApply (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
horusdata = getappdata(handles.parenthandle, 'horusdata');
Detdata = getappdata(str2double(horusdata.hADC),'Detdata');

switch handles.device
    case 0
        Mask=Detdata.PMTMask;
    case 1
        Mask=Detdata.MCP1Mask;
    case 2
        Mask=Detdata.MCP2Mask;
end

MaskOption=get(handles.ToggleMaskUnmask,'Value');
if MaskOption==get(handles.ToggleMaskUnmask,'Max')
    MaskOption=1;
else
    MaskOption=0;
end

MaskStartChannel=str2double(get(handles.MaskStart,'String'));
MaskStopChannel=str2double(get(handles.MaskStop,'String'));
if (isnan(MaskStartChannel)| MaskStartChannel<1 | MaskStartChannel>160 ...
    |isnan(MaskStopChannel)| MaskStopChannel<1 | MaskStopChannel>160)
    error('invalid channels');
else
    StartAddr=floor((MaskStartChannel-1)/16)+1;
    StopAddr=floor((MaskStopChannel-1)/16)+1;
    if MaskOption==0
        Mask(MaskStartChannel:MaskStopChannel)=0;
    else
        Mask(MaskStartChannel:MaskStopChannel)=1;
    end
    for i=StartAddr:StopAddr
        wordvalue=bin2dec(num2str(flipdim(Mask(((i-1)*16+1):(i*16)),2)));
        word=[' 0x0',dec2hex(wordvalue)];
        handles.device;
        disp(['/lift/bin/eCmd s setmask ',num2str(i-1+handles.device*10),word]);
        system(['/lift/bin/eCmd s setmask ',num2str(i-1+handles.device*10),word]);
    end
end


function devicePopupmenu_Callback(hObject, eventdata, handles)
set(hObject,'BackgroundColor','white');
val=get(hObject,'Value');
switch val
    case 1
        handles.device=0;
    case 2
        handles.device=1;        
    case 3
        handles.device=2;        
end
% Update handles structure
guidata(hObject, handles);






% --- Executes on button press in Exit.
function Exit_Callback(hObject, eventdata, handles)
% hObject    handle to Exit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
close(CounterCards);



% --- Executes on button press in toggleGain.
function toggleGain_Callback(hObject, eventdata, handles)
% hObject    handle to toggleGain (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleGain



function edGain_Callback(hObject, eventdata, handles)
% hObject    handle to edGain (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edGain as text
%        str2double(get(hObject,'String')) returns contents of edGain as a double


% --- Executes during object creation, after setting all properties.
function edGain_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edGain (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function edMaster_Callback(hObject, eventdata, handles)
% hObject    handle to edMaster (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edMaster as text
%        str2double(get(hObject,'String')) returns contents of edMaster as a double


% --- Executes during object creation, after setting all properties.
function edMaster_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edMaster (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end



function edCounter_Callback(hObject, eventdata, handles)
% hObject    handle to edCounter (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edCounter as text
%        str2double(get(hObject,'String')) returns contents of edCounter as a double


% --- Executes during object creation, after setting all properties.
function edCounter_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edCounter (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end





function edGainWidth_Callback(hObject, eventdata, handles)
% hObject    handle to edGainWidth (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edGainWidth as text
%        str2double(get(hObject,'String')) returns contents of edGainWidth as a double


% --- Executes during object creation, after setting all properties.
function edGainWidth_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edGainWidth (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


