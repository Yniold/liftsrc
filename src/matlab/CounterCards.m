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

% Last Modified by GUIDE v2.5 12-Jan-2005 14:47:40

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

% UIWAIT makes guidetemplate0 wait for user response (see UIRESUME)
% uiwait(handles.CounterCards);
if length(varargin)==2 & varargin{1}=='handle'
    handles.parenthandle=str2double(varargin{2});
end

% Update handles structure
guidata(hObject, handles);

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
ADCdata = getappdata(str2double(horusdata.hADC),'ADCdata');

switch handles.device
    case 0
        Mask=ADCdata.PMTMask;
    case 1
        Mask=ADCdata.MCP1Mask;
    case 2
        Mask=ADCdata.MCP2Mask;
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




