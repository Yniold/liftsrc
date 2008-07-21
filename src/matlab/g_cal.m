function varargout = g_cal(varargin)
% G_CAL M-file for g_cal.fig
%      G_CAL, by itself, creates a new G_CAL or raises the existing
%      singleton*.
%
%      H = G_CAL returns the handle to a new G_CAL or the handle to
%      the existing singleton*.
%
%      G_CAL('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in G_CAL.M with the given input arguments.
%
%      G_CAL('Property','Value',...) creates a new G_CAL or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before g_cal_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to g_cal_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help g_cal

% Last Modified by GUIDE v2.5 18-Jul-2008 11:04:12

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @g_cal_OpeningFcn, ...
                   'gui_OutputFcn',  @g_cal_OutputFcn, ...
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


% --- Executes just before g_cal is made visible.
function g_cal_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to g_cal (see VARARGIN)

% Choose default command line output for g_cal
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes g_cal wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = g_cal_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;
