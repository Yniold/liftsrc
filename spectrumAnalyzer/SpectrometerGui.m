function varargout = SpectrometerGui(varargin)
% SPECTROMETERGUI M-file for SpectrometerGui.fig
%      SPECTROMETERGUI, by itself, creates a new SPECTROMETERGUI or raises the existing
%      singleton*.
%
%      H = SPECTROMETERGUI returns the handle to a new SPECTROMETERGUI or the handle to
%      the existing singleton*.
%
%      SPECTROMETERGUI('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in SPECTROMETERGUI.M with the given input arguments.
%
%      SPECTROMETERGUI('Property','Value',...) creates a new SPECTROMETERGUI or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before SpectrometerGui_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to SpectrometerGui_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help SpectrometerGui

% Last Modified by GUIDE v2.5 18-Feb-2007 22:38:21

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @SpectrometerGui_OpeningFcn, ...
                   'gui_OutputFcn',  @SpectrometerGui_OutputFcn, ...
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

% --- Executes just before SpectrometerGui is made visible.
function SpectrometerGui_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to SpectrometerGui (see VARARGIN)

% Choose default command line output for SpectrometerGui
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes SpectrometerGui wait for user response (see UIRESUME)
% uiwait(handles.figure1);
% kill any existing open socket %
OldSockets = instrfind;
delete(OldSockets);

% User Data for Callback%
MyCallbackData.Counter = 0;
MyCallbackData.StartNewPlot = 1;
MyCallbackData.MaxPackets = 0;
MyCallbackData.OldMaxPackets = 0;
MyCallbackData.PacketsSeen = zeros(2,1);
MyCallbackData.ThePlotBuffer = [];
MyCallbackData.handles = handles;

% create a new UDP socket %
MySocket = udp('127.0.0.1');

set(MySocket,'UserData',MyCallbackData);

MySocket.LocalPort = 4711;
MySocket.InputBufferSize = 8192;
MySocket.ByteOrder = 'littleEndian';
MySocket.DatagramTerminateMode = 'on';
MySocket.DatagramReceivedFcn={'SpectrumDatagramCallback'};
fopen(MySocket);

% --- Outputs from this function are returned to the command line.
function varargout = SpectrometerGui_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;
