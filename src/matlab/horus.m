function varargout = horus(varargin)
% HORUS M-file for horus.fig
%      HORUS, by itself, creates a new HORUS or raises the existing
%      singleton*.
%
%      H = HORUS returns the handle to a new HORUS or the handle to
%      the existing singleton*.
%
%      HORUS('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in HORUS.M with the given input arguments.
%
%      HORUS('Property','Value',...) creates a new HORUS or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before horus_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to horus_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Copyright 2002-2003 The MathWorks, Inc.

% Edit the above text to modify the response to help horus

% Last Modified by GUIDE v2.5 24-Jan-2005 19:11:29

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @horus_OpeningFcn, ...
                   'gui_OutputFcn',  @horus_OutputFcn, ...
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


% --- Executes just before horus is made visible.
function horus_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to horus (see VARARGIN)

% Choose default command line output for horus
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes horus wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = horus_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in ADC.
function ADC_Callback(hObject, eventdata, handles)
% hObject    handle to ADC (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
data = getappdata(gcbf, 'horusdata');
handleADC=ADC('handle',num2str(gcbf,16));
data.hADC=num2str(handleADC,16);
setappdata(gcbf, 'horusdata', data); 


% --- Executes on button press in CounterCards.
function CounterCards_Callback(hObject, eventdata, handles)
% hObject    handle to CounterCards (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
data = getappdata(gcbf, 'horusdata');
handleCounterCards=CounterCards('handle',num2str(gcbf,16));
data.hCounterCards=num2str(handleCounterCards,16);
setappdata(gcbf, 'horusdata', data); 



% --- Executes on button press in etalon.
function Etalon_Callback(hObject, eventdata, handles)
% hObject    handle to etalon (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
data = getappdata(gcbf, 'horusdata');
handleEtalon=etalon('handle',num2str(gcbf,16));
data.hEtalon=num2str(handleEtalon,16);
setappdata(gcbf, 'horusdata', data); 




% --- Executes on button press in Exit.
function Exit_Callback(hObject, eventdata, handles)
% hObject    handle to Exit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
data = getappdata(gcbf, 'horusdata');

if isfield(data,'hADC')
    hADC=str2double(data.hADC);
    if ishandle(hADC), 
        ADCdata = getappdata(hADC, 'ADCdata');
        if isfield(ADCdata,'ActTimer')
            stop(ADCdata.ActTimer);
            delete(ADCdata.ActTimer);
        end
        close(hADC); 
    end
end

if isfield(data,'hCounterCards')
    hCounterCards=str2double(data.hCounterCards);
    if ishandle(hCounterCards), close(hCounterCards); end
end

if isfield(data,'hEtalon')
    hEtalon=str2double(data.hEtalon);
    if ishandle(hEtalon), close(hEtalon); end
end

close(horus);


