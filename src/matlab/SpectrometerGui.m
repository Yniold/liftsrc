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

% Last Modified by GUIDE v2.5 09-Nov-2007 16:06:51

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

% get horus handle
if length(varargin)==2 & varargin{1}=='handle'
    handles.parenthandle=str2double(varargin{2});
end

%setup Timer function
handles.Timer = timer('ExecutionMode','fixedDelay',...
      'Period',0.7,...    
      'BusyMode','drop',...
      'TimerFcn', {@SpecRefresh,handles});   
  
% Update handles structure
guidata(hObject, handles);

start(handles.Timer);


function SpecRefresh(arg1,arg2,handles)

specdata=ReadSpec('/lift/ramdisk/status.spc');

% Calculate time as sum of day, hour, min, etc.
spectime=double(specdata(:,2))./1.0+ ...
           double(specdata(:,3))./24.0+...
           double(specdata(:,4))./1440.0+...
           double(specdata(:,5))./86400.0+...
           double(specdata(:,6))./86400000.0;
       
[SortZeit,indexZeit]=sort(spectime);
maxLen=size(spectime,1);
lastrow=indexZeit(maxLen);

%display time       
disptime=spectime(lastrow)-double(specdata(lastrow,6))/86400000.0;
set(handles.txtTime,'String',strcat(datestr(disptime,13),'.',num2str(specdata(lastrow,6)/100)));
%display etalon action
etaction=double(specdata(lastrow,16));
switch etaction
    case {0,1}
        set(handles.txtEtalonAction,'String','Online','BackgroundColor','c');
    case 3
        set(handles.txtEtalonAction,'String','Offline Left','BackgroundColor','g');
    case 4
        set(handles.txtEtalonAction,'String','Offline Right','BackgroundColor','r');
    otherwise
        set(handles.txtEtalonAction,'String','ERROR','BackgroundColor','m');
end

xdata=double(specdata(lastrow,17:3856))./50;
ydata=double(specdata(lastrow,3857:7696));
ydata(ydata==0)=NaN;
[peak,xpeak]=max(ydata);
if get(handles.tglscale,'Value')
    xlim1=double(specdata(lastrow,12))/50;
    xlim2=double(specdata(lastrow,13))/50;
else
    xlim1=xdata(xpeak)-1;
    xlim2=xdata(xpeak)+1;
end
plot(handles.TheSpectrum,xdata,ydata);
xlimits=[xlim1, xlim2];
set(handles.TheSpectrum,'xlim',xlimits);



% --- Executes on button press in pshExit.
function pshExit_Callback(hObject, eventdata, handles)
% hObject    handle to pshExit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
stop(handles.Timer);
delete(handles.Timer);
close(handles.figure1);




% --- Outputs from this function are returned to the command line.
function varargout = SpectrometerGui_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;



% --- Executes on button press in tglscale.
function tglscale_Callback(hObject, eventdata, handles)
% hObject    handle to tglscale (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if get(hObject,'Value')
    set(hObject,'String','FULL SCALE')
else
    set(hObject,'String','full scale')
end


