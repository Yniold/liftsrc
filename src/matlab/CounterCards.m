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

% Last Modified by GUIDE v2.5 14-Feb-2005 16:17:12

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

% initialize device to be the PMT
set(handles.device,'Value',1);
% setup Timer function
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
data = getappdata(handles.output, 'Gatedata');

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

set(handles.txtMaster,'String',num2str(statusData(lastrow,15)));
if isfield(handles,'device')
    switch get(handles.device,'Value')
        case 1
            set(handles.toggleGain,'Value',0,'String','Gain is OFF','BackgroundColor','c');
            set(handles.txtGain,'String','NA');
            set(handles.txtGainWidth,'String','NA');
            set(handles.txtCounter,'String',num2str(statusData(lastrow,16)));
        case 2
            if bitget(statusData(lastrow,226),16)
                set(handles.toggleGain,'Value',1,'String','Gain is ON','BackgroundColor','g');
            else
                set(handles.toggleGain,'Value',0,'String','Gain is OFF','BackgroundColor','c');
            end
            set(handles.txtGain,'String',num2str(bitset(statusData(lastrow,226),16,0)));
            set(handles.txtGainWidth,'String',num2str(statusData(lastrow,227)));
            set(handles.txtCounter,'String',num2str(statusData(lastrow,225)));
        case 3
            if bitget(statusData(lastrow,435),16);
                set(handles.toggleGain,'Value',1,'String','Gain is ON','BackgroundColor','g');
            else
                set(handles.toggleGain,'Value',0,'String','Gain is OFF','BackgroundColor','c');
            end
            set(handles.txtGain,'String',num2str(bitset(statusData(lastrow,435),16,0)));
            set(handles.txtGainWidth,'String',num2str(statusData(lastrow,436)));
            set(handles.txtCounter,'String',num2str(statusData(lastrow,434)));
    end
else % if no device was chosen then show values for PMT
    set(handles.toggleGain,'Value',0,'String','Gain is OFF','BackgroundColor','c');
    set(handles.txtGain,'String','NA');
    set(handles.txtGainWidth,'String','NA');
    set(handles.txtCounter,'String',num2str(statusData(lastrow,16)));
end

data.lastrow=lastrow;
setappdata(handles.output, 'Gatedata', data);



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
if MaskStartChannel<1 | MaskStartChannel>160
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
if MaskStopChannel<1 | MaskStopChannel>160
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

switch get(handles.device,'Value')
    case 1
        Mask=Detdata.PMTMask;
    case 2
        Mask=Detdata.MCP1Mask;
    case 3
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
        system(['/lift/bin/eCmd s setmask ',num2str(i-1+handles.device*10),word]);
    end
end


function device_Callback(hObject, eventdata, handles)
set(hObject,'BackgroundColor','white');
guidata(hObject, handles);






% --- Executes on button press in Exit.
function Exit_Callback(hObject, eventdata, handles)
% hObject    handle to Exit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
stop(handles.Timer);
delete(handles.Timer);
close(handles.CounterCards);



% --- Executes on button press in toggleGain.
function toggleGain_Callback(hObject, eventdata, handles)
% hObject    handle to toggleGain (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleGain
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
data = getappdata(handles.output, 'Gatedata');
lastrow=data.lastrow;

if get(hObject,'Value')
    set(hObject,'String','Gain is ON','BackgroundColor','g');
    switch get(handles.device,'Value')
        case 1
            set(hObject,'Value',0,'String','Gain is OFF','BackgroundColor','c');
        case 2
            word=bitset(statusData(lastrow,226),16);
            system(['/lift/bin/eCmd w 0xa318 ',num2str(word)]);
        case 3
            word=bitset(statusData(lastrow,435),16);
            system(['/lift/bin/eCmd w 0xa31c ',num2str(word)]);
    end
else
    set(hObject,'String','Gain is OFF','BackgroundColor','c');
    switch get(handles.device,'Value')
        case 1
        case 2
            word=bitset(statusData(lastrow,226),16,0);
            system(['/lift/bin/eCmd w 0xa318 ',num2str(word)]);
        case 3
            word=bitset(statusData(lastrow,435),16,0);
            system(['/lift/bin/eCmd w 0xa31c ',num2str(word)]);
    end
end    
    
    

function edGain_Callback(hObject, eventdata, handles)
% hObject    handle to edGain (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edGain as text
%        str2double(get(hObject,'String')) returns contents of edGain as a double
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
data = getappdata(handles.output, 'Gatedata');
lastrow=data.lastrow;

gaindelay=uint16(str2double(get(hObject,'String')));
if gaindelay<1 
    disp('value must be >0')
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','w');
    switch get(handles.device,'Value')
        case 1
        case 2
            gainstatus=bitget(statusData(lastrow,226),16);
            word=bitset(gaindelay,16,gainstatus);
            system(['/lift/bin/eCmd w 0xa318 ',num2str(word)]);            
        case 3
            gainstatus=bitget(statusData(lastrow,435),16);
            word=bitset(gaindelay,16,gainstatus);
            system(['/lift/bin/eCmd w 0xa31c ',num2str(word)]);            
    end
end


function edMaster_Callback(hObject, eventdata, handles)
% hObject    handle to edMaster (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edMaster as text
%        str2double(get(hObject,'String')) returns contents of edMaster as a double
masterdelay=uint16(str2double(get(hObject,'String')));
if masterdelay<1 
    disp('value must be >0')
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','w');
    system(['/lift/bin/eCmd w 0xa316 ',num2str(masterdelay)]);            
end



function edCounter_Callback(hObject, eventdata, handles)
% hObject    handle to edCounter (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edCounter as text
%        str2double(get(hObject,'String')) returns contents of edCounter as a double
counterdelay=uint16(str2double(get(hObject,'String')));
if counterdelay<1 
    disp('value must be >0')
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','w');
    switch get(handles.device,'Value')
        case 1
            system(['/lift/bin/eCmd w 0xa310 ',num2str(counterdelay)]);            
        case 2
            system(['/lift/bin/eCmd w 0xa312 ',num2str(counterdelay)]);            
        case 3
            system(['/lift/bin/eCmd w 0xa314 ',num2str(counterdelay)]);            
    end
end



function edGainWidth_Callback(hObject, eventdata, handles)
% hObject    handle to edGainWidth (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edGainWidth as text
%        str2double(get(hObject,'String')) returns contents of edGainWidth as a double
gainwidth=uint16(str2double(get(hObject,'String')));
if gainwidth<1 
    disp('value must be >0')
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','w');
    switch get(handles.device,'Value')
        case 1
        case 2
            system(['/lift/bin/eCmd w 0xa31a ',num2str(gainwidth)]);            
        case 3
            system(['/lift/bin/eCmd w 0xa31e ',num2str(gainwidth)]);            
    end
end




