function varargout = Dyelaser(varargin)
% DYELASER M-file for Dyelaser.fig
%      DYELASER, by itself, creates a new DYELASER or raises the existing
%      singleton*.
%
%      H = DYELASER returns the handle to a new DYELASER or the handle to
%      the existing singleton*.
%
%      DYELASER('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in DYELASER.M with the given input arguments.
%
%      DYELASER('Property','Value',...) creates a new DYELASER or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before Dyelaser_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to Dyelaser_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Copyright 2002-2003 The MathWorks, Inc.

% Edit the above text to modify the response to help Dyelaser

% Last Modified by GUIDE v2.5 26-Jan-2005 13:56:04

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @Dyelaser_OpeningFcn, ...
                   'gui_OutputFcn',  @Dyelaser_OutputFcn, ...
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


% --- Executes just before Dyelaser is made visible.
function Dyelaser_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to Dyelaser (see VARARGIN)

% Choose default command line output for Dyelaser
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes Dyelaser wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = Dyelaser_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in checkDiodeGr.
function checkDiodeGr_Callback(hObject, eventdata, handles)
% hObject    handle to checkDiodeGr (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkDiodeGr


% --- Executes on button press in checkDiodeUV.
function checkDiodeUV_Callback(hObject, eventdata, handles)
% hObject    handle to checkDiodeUV (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkDiodeUV


% --- Executes on button press in checkDiodeEt.
function checkDiodeEt_Callback(hObject, eventdata, handles)
% hObject    handle to checkDiodeEt (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkDiodeEt


% --- Executes on button press in checkPDyelaser.
function checkPDyelaser_Callback(hObject, eventdata, handles)
% hObject    handle to checkPDyelaser (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkPDyelaser


% --- Executes on button press in checkPVent.
function checkPVent_Callback(hObject, eventdata, handles)
% hObject    handle to checkPVent (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkPVent


% --- Executes on button press in toggleDyelaser.
function toggleDyelaser_Callback(hObject, eventdata, handles)
% hObject    handle to toggleDyelaser (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleDyelaser


% --- Executes on button press in toggleVacuum.
function toggleVacuum_Callback(hObject, eventdata, handles)
% hObject    handle to toggleVacuum (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleVacuum


% --- Executes on button press in toggleN2.
function toggleN2_Callback(hObject, eventdata, handles)
% hObject    handle to toggleN2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleN2


% --- Executes on button press in toggleAmbient.
function toggleAmbient_Callback(hObject, eventdata, handles)
% hObject    handle to toggleAmbient (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleAmbient


% --- Executes on button press in Exit.
function Exit_Callback(hObject, eventdata, handles)
% hObject    handle to Exit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


