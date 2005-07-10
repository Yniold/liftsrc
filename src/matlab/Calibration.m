function varargout = Calibration(varargin)
%CALIBRATION M-file for Calibration.fig
%      CALIBRATION, by itself, creates a new CALIBRATION or raises the existing
%      singleton*.
%
%      H = CALIBRATION returns the handle to a new CALIBRATION or the handle to
%      the existing singleton*.
%
%      CALIBRATION('Property','Value',...) creates a new CALIBRATION using the
%      given property value pairs. Unrecognized properties are passed via
%      varargin to Calibration_OpeningFcn.  This calling syntax produces a
%      warning when there is an existing singleton*.
%
%      CALIBRATION('CALLBACK') and CALIBRATION('CALLBACK',hObject,...) call the
%      local function named CALLBACK in CALIBRATION.M with the given input
%      arguments.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help Calibration

% Last Modified by GUIDE v2.5 10-Jul-2005 20:54:39

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @Calibration_OpeningFcn, ...
                   'gui_OutputFcn',  @Calibration_OutputFcn, ...
                   'gui_LayoutFcn',  [], ...
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


% --- Executes just before Calibration is made visible.
function Calibration_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   unrecognized PropertyName/PropertyValue pairs from the
%            command line (see VARARGIN)

% Choose default command line output for Calibration
handles.output = hObject;

% get horus handle
if length(varargin)==2 & varargin{1}=='handle'
    handles.parenthandle=str2double(varargin{2});
end

%setup Timer function
handles.Timer = timer('ExecutionMode','fixedDelay',...
      'Period',1.0,...    
      'BusyMode','drop',...
      'TimerFcn', {@CalRefresh,handles});   

data.Counter=0;
data.CounterOnl=0;
data.CounterOffl=0;
data.sumDiodeWZ1in=0;
data.sumDiodeWZ1out=0;
data.sumDiodeWZ2in=0;
data.sumDiodeWZ2out=0;
data.sumctsMCP1onl=0;
data.sumctsMCP1offl=0;
data.sumctsMCP2onl=0;
data.sumctsMCP2offl=0;

% Update handles structure
guidata(hObject, handles);
setappdata(handles.output, 'Caldata', data);



% UIWAIT makes Calibration wait for user response (see UIRESUME)
% uiwait(handles.figure1);


function CalRefresh(arg1,arg2,handles)

data = getappdata(handles.output, 'Caldata');

horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
AvgData=horusdata.AvgData;
col=horusdata.col;
fcts2val=horusdata.fcts2val;

% Calculate time as sum of day, hour, min, etc.
statustime=double(statusData(:,2))./1.0+ ...
           double(statusData(:,3))./24.0+...
           double(statusData(:,4))./1440.0+...
           double(statusData(:,5))./86400.0;

[SortZeit,indexZeit]=sort(statustime);
maxLen=size(statustime,1);
lastrow=indexZeit(maxLen);
data.Counter=data.Counter+1;

% display system time
set(handles.txtTimer,'String',strcat(datestr(statustime(lastrow),13),'.',num2str(statusData(lastrow,6)/100)));

% calculate parameters from ADC counts
x=double(statusData(lastrow-4:lastrow,col.DiodeWZ1out)); eval(['DiodeWZ1out=',fcts2val.DiodeWZ1out,';']);
x=double(statusData(lastrow-4:lastrow,col.DiodeWZ2out)); eval(['DiodeWZ2out=',fcts2val.DiodeWZ2out,';']);
x=double(statusData(lastrow-4:lastrow,col.DiodeWZ1in)); eval(['DiodeWZ1in=',fcts2val.DiodeWZ1in,';']);
x=double(statusData(lastrow-4:lastrow,col.DiodeWZ2in)); eval(['DiodeWZ2in=',fcts2val.DiodeWZ2in,';']);

ctsMCP1=double(statusData(lastrow-4:lastrow,col.ccCounts1));
ctsMCP2=double(statusData(lastrow-4:lastrow,col.ccCounts2));
OnOffFlag=statusData(lastrow-4:lastrow,col.RAvgOnOffFlag);

% calculate running averages
for i=1:5
    data.sumDiodeWZ1in=data.sumDiodeWZ1in+DiodeWZ1in(i);
    data.sumDiodeWZ1out=data.sumDiodeWZ1out+DiodeWZ1out(i);
    data.sumDiodeWZ2in=data.sumDiodeWZ2in+DiodeWZ2in(i);
    data.sumDiodeWZ2out=data.sumDiodeWZ2out+DiodeWZ2out(i);
    if OnOffFlag(i)==3
        data.sumctsMCP1onl=data.sumctsMCP1onl+ctsMCP1(i);
        data.sumctsMCP2onl=data.sumctsMCP2onl+ctsMCP2(i);
        data.CounterOnl=data.CounterOnl+1;
    elseif OnOffFlag(i)==1 | OnOffFlag(i)==2
        data.sumctsMCP1offl=data.sumctsMCP1offl+ctsMCP1;
        data.sumctsMCP2offl=data.sumctsMCP2offl+ctsMCP2;
        data.CounterOffl=data.CounterOffl+1;
    end
end
avgDiodeWZ1in=data.sumDiodeWZ1in/data.Counter;
avgDiodeWZ1out=data.sumDiodeWZ1out/data.Counter;
avgDiodeWZ2in=data.sumDiodeWZ2in/data.Counter;
avgDiodeWZ2out=data.sumDiodeWZ2out/data.Counter;
if data.CounterOnl>0
    avgMCP1onl=data.sumctsMCP1onl/data.CounterOnl;
    avgMCP2onl=data.sumctsMCP2onl/data.CounterOnl;
else
    avgMCP1onl=NaN;
    avgMCP2onl=NaN;
end
if data.CounterOffl>0
    avgMCP1offl=data.sumctsMCP1offl/data.CounterOffl;
    avgMCP2offl=data.sumctsMCP2offl/data.CounterOffl;
else
    avgMCP1offl=NaN;
    avgMCP2offl=NaN;
end

    
% display averages
set(handles.txtCounts,'String',[num2str(data.Counter)']);
set(handles.txtWZ1in,'String',[num2str(avgDiodeWZ1in,3),' mW']);
set(handles.txtWZ1out,'String',[num2str(avgDiodeWZ1out,3),' mW']);
set(handles.txtWZ2in,'String',[num2str(avgDiodeWZ2in,3),' mW']);
set(handles.txtWZ2out,'String',[num2str(avgDiodeWZ2out,3),' mW']);
set(handles.txtMCP1onl,'String',[num2str(avgMCP1onl,3)]);
set(handles.txtMCP1offl,'String',[num2str(avgMCP1offl,3)]);
set(handles.txtMCP2onl,'String',[num2str(avgMCP2onl,3)]);
set(handles.txtMCP2offl,'String',[num2str(avgMCP2offl,3)]);

setappdata(handles.output, 'Caldata', data);




% --- Outputs from this function are returned to the command line.
function varargout = Calibration_OutputFcn(hObject, eventdata, handles)
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
    
% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in pshExit.
function pshExit_Callback(hObject, eventdata, handles)
% hObject    handle to pshExit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
data = getappdata(handles.output, 'Caldata');

stop(handles.Timer);
delete(handles.Timer);
close(handles.figure1);


% --- Executes on button press in tglStart.
function tglStart_Callback(hObject, eventdata, handles)
% hObject    handle to tglStart (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
data = getappdata(handles.output, 'Caldata');
if get(hObject,'Value')
    set(hObject,'String','Stop')
    data.Counter=0;
    data.CounterOnl=0;
    data.CounterOffl=0;
    data.sumDiodeWZ1in=0;
    data.sumDiodeWZ1out=0;
    data.sumDiodeWZ2in=0;
    data.sumDiodeWZ2out=0;
    data.sumctsMCP1onl=0;
    data.sumctsMCP1offl=0;
    data.sumctsMCP2onl=0;
    data.sumctsMCP2offl=0;
    setappdata(handles.output, 'Caldata', data);

    start(handles.Timer);
else
    set(hObject,'String','Start')
    stop(handles.Timer);
end

