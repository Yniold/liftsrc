function varargout = mirror_trials(varargin)
% MIRROR_TRIALS M-file for mirror_trials.fig
%      MIRROR_TRIALS, by itself, creates a new MIRROR_TRIALS or raises the existing
%      singleton*.
%
%      H = MIRROR_TRIALS returns the handle to a new MIRROR_TRIALS or the handle to
%      the existing singleton*.
%
%      MIRROR_TRIALS('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in MIRROR_TRIALS.M with the given input arguments.
%
%      MIRROR_TRIALS('Property','Value',...) creates a new MIRROR_TRIALS or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before mirror_trials_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to mirror_trials_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help mirror_trials

% Last Modified by GUIDE v2.5 23-Jul-2008 08:48:51

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @mirror_trials_OpeningFcn, ...
                   'gui_OutputFcn',  @mirror_trials_OutputFcn, ...
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


% --- Executes just before mirror_trials is made visible.
function mirror_trials_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to mirror_trials (see VARARGIN)

% Choose default command line output for mirror_trials
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);
% choose Text size
set(double(get(handles.figure1,'Children')),'FontSize',8)

% get NO2LIF handle
if length(varargin)==2 & varargin{1}==handle
    handles.parenthandle=str2double(varargin{2});
end

% UIWAIT makes mirror_trials wait for user response (see UIRESUME)
% uiwait(handles.figure1);
% open tcpip port for communication with Laser,

 picotport=tcpip('10.111.111.28',8100);
 set(picotport,'ReadAsyncMode','continuous');
 set(picotport,'BytesAvailableFcn',{'tcpipdatacallback'});
 set(picotport,'Terminator','CR');
 set(picotport,'BytesAvailableFcnMode','Terminator');
 
 try
 fopen(picotport);
 handles.picotport=picotport;
 data.picotport=handles.picotport;
 set(handles.txtpicotport,'String','communication INITIALIZED','BackgroundColor','g');
  
catch  
 % if communication with laser did not work
    fclose(picotport);
    delete(picotport);
    clear('picotport');
    set(handles.txtpicotport,'String','communication FAILED','BackgroundColor','r');
end


% --- Outputs from this function are returned to the command line.
function varargout = mirror_trials_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;



function editmirror_Callback(hObject, eventdata, handles)
% hObject    handle to editmirror (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editmirror as text
%        str2double(get(hObject,'String')) returns contents of editmirror as a double

picotport=handles.picotport;
MirrorCmd=get(hObject,'String');
MirrorCmd=char(MirrorCmd);
set(handles.txtmirror,'String',MirrorCmd);
fprintf(picotport,MirrorCmd);
    pause(1);
M_Answer=picotport.UserData;
M_Answer=char(L_Answer);
picotport.UserData=[];
set(handles.txtmirror,'String',M_Answer);

% --- Executes during object creation, after setting all properties.
function editmirror_CreateFcn(hObject, eventdata, handles)
% hObject    handle to editmirror (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


