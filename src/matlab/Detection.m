function varargout = Detection(varargin)
% DETECTION M-file for Detection.fig
%      DETECTION, by itself, creates a new DETECTION or raises the existing
%      singleton*.
%
%      H = DETECTION returns the handle to a new DETECTION or the handle to
%      the existing singleton*.
%
%      DETECTION('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in DETECTION.M with the given input arguments.
%
%      DETECTION('Property','Value',...) creates a new DETECTION or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before Detection_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to Detection_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Copyright 2002-2003 The MathWorks, Inc.

% Edit the above text to modify the response to help Detection

% Last Modified by GUIDE v2.5 04-Feb-2005 18:34:32

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @Detection_OpeningFcn, ...
                   'gui_OutputFcn',  @Detection_OutputFcn, ...
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


% --- Executes just before Detection is made visible.
function Detection_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to Detection (see VARARGIN)

% Choose default command line output for Detection
handles.output = hObject;

% get horus handle
if length(varargin)==2 & varargin{1}=='handle'
    handles.parenthandle=str2double(varargin{2});
end

%setup Timer function
handles.Timer = timer('ExecutionMode','fixedDelay',...
      'Period',0.7,...    
      'BusyMode','drop',...
      'TimerFcn', {@DetRefresh,handles});   

data.Timer=handles.Timer;

% Update handles structure
guidata(hObject, handles);
setappdata(handles.output, 'Detdata', data);
start(handles.Timer);


function DetRefresh(arg1,arg2,handles)

data = getappdata(handles.output, 'Detdata');

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
PlotWidth=maxLen;
stopPlot=maxLen;
startPlot=1;
iZeit=indexZeit(startPlot:stopPlot);
minTime=statustime(iZeit(1));
maxTime=statustime(iZeit(size(iZeit,1)));

% display system time
set(handles.txtTimer,'String',strcat(datestr(statustime(lastrow),13),'.',num2str(statusData(lastrow,6)/100)));

% calculate parameters from ADC counts
ADCBase0=656;
ADCBase1=689;
TempBase=729;

TempDetAxis=single(statusData(:,TempBase+7))./100-273.15;  
P20=single(statusData(:,ADCBase1+1*3))*0.00891-89.55;
P1000=single(statusData(:,ADCBase1));%*0.4464-4380.9;
DiodeWZout=(double(statusData(iZeit,ADCBase1+5*3))-9790.0)/193.2836;

% display ADC counts
set(handles.txtWZin,'String','NA'); %statusData(lastrow,ADCBase1+5*3));
set(handles.txtWZout,'String',statusData(lastrow,ADCBase1+5*3));
set(handles.txtP1000,'String',statusData(lastrow,ADCBase1));
set(handles.txtP20,'String',statusData(lastrow,ADCBase1+1*3));
set(handles.txtPNO,'String',statusData(lastrow,ADCBase1+2*3));
set(handles.txtVHV,'String',statusData(lastrow,ADCBase1+3*3));
set(handles.txtTDet,'String',TempDetAxis(lastrow));

% warn for ADC signals out of allowed range for measurements
if P20(lastrow)<3 | P20(lastrow)>6
    set(handles.txtP20,'BackgroundColor','r');
end
if DiodeWZout(lastrow)<2
    set(handles.txtWZout,'BackgroundColor','r');
end

% plot checked parameters vs. time
hold(handles.axes1,'off'); 
%if get(handles.chkWZin,'Value')
%    plot(handles.axes1,statustime(iZeit),DiodeWZin,'r');
    %hold(handles.axes1,'on');
%end 
if get(handles.chkWZout,'Value')
    plot(handles.axes1,statustime(iZeit),DiodeWZout,'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkP1000,'Value')
    plot(handles.axes1,statustime(iZeit),P1000(iZeit),'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkP20,'Value')
    plot(handles.axes1,statustime(iZeit),P20(iZeit),'b');
    hold(handles.axes1,'on');
end 
if get(handles.chkPNO,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,ADCBase1+2*3),'b');
    hold(handles.axes1,'on');
end 
if get(handles.chkVHV,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,ADCBase1+3*3),'b');
    hold(handles.axes1,'on');
end 
if get(handles.chkTDet,'Value')
    plot(handles.axes1,statustime(iZeit),TempDetAxis(iZeit),'b');
    hold(handles.axes1,'on');
end 
xlim(handles.axes1,[minTime maxTime]);
grid(handles.axes1);

%plot PMT and MCP signals

PMTBase=19;
MCP1Base=228;
MCP2Base=437;
AVGBase=1;
PMTMaskBase=211;
MCP1MaskBase=420;
MCP2MaskBase=629;

% PMT: Has the counter mask changed ? Then read in new mask.
if ~isfield(data,'PMTMask')| ...
        ~isequal(statusData(lastrow,PMTMaskBase:PMTMaskBase+9),statusData(lastrow-5,PMTMaskBase:PMTMaskBase+9))
    data.PMTMask=ones(1,160);
    for a = 0:9,
        data.PMTMask((a*16+1):(a*16+16))=bitget(statusData(lastrow,PMTMaskBase+a),1:16);
    end
end
    
PMTSumCounts=statusData(:,PMTMaskBase+12);

% MCP1: Has the counter mask changed ? Then read in new mask.
if ~isfield(data,'MCP1Mask')| ...
        ~isequal(statusData(lastrow,MCP1MaskBase:MCP1MaskBase+9),statusData(lastrow-5,MCP1MaskBase:MCP1MaskBase+9))
    data.MCP1Mask=ones(1,160);
    for a=0:9
        data.MCP1Mask((a*16+1):(a*16+16))=bitget(statusData(lastrow,MCP1MaskBase+a),1:16);
    end
end

MCP1SumCounts=statusData(:,MCP1MaskBase+12);

% MCP2: Has the counter mask changed ? Then read in new mask.
if ~isfield(data,'MCP2Mask')| ...
        ~isequal(statusData(lastrow,MCP2MaskBase:MCP2MaskBase+9),statusData(lastrow-5,MCP2MaskBase:MCP2MaskBase+9))
    data.MCP2Mask=ones(1,160);
    for a=0:9
        data.MCP2Mask((a*16+1):(a*16+16))=bitget(statusData(lastrow,MCP2MaskBase+a),1:16);
    end
end

MCP2SumCounts=statusData(:,MCP2MaskBase+12);

% display counts and pulses
set(handles.txtPMTCounts,'String',statusData(lastrow,PMTBase+204));
set(handles.txtMCP1Counts,'String',statusData(lastrow,MCP1Base+204));
set(handles.txtMCP2Counts,'String',statusData(lastrow,MCP2Base+204));

set(handles.txtPMTPulses,'String',statusData(lastrow,PMTBase+205));
set(handles.txtMCP1Pulses,'String',statusData(lastrow,MCP1Base+205));
set(handles.txtMCP2Pulses,'String',statusData(lastrow,MCP2Base+205));

% calculate running averages for online and both offlines
OnlineFilter=AvgData(:,AVGBase)>0;
OfflineLeftFilter=AvgData(:,AVGBase+1)>0;
OfflineRightFilter=AvgData(:,AVGBase+2)>0;

PMTOnlineAvg(OnlineFilter)=AvgData(OnlineFilter,AVGBase);  
PMTOnlineAvg(~OnlineFilter)=NaN;
PMTOfflineLeftAvg(OfflineLeftFilter)=AvgData(OfflineLeftFilter,AVGBase+1);
PMTOfflineLeftAvg(~OfflineLeftFilter)=NaN;
PMTOfflineRightAvg(OfflineRightFilter)=AvgData(OfflineRightFilter,AVGBase+2);
PMTOfflineRightAvg(~OfflineRightFilter)=NaN;

PMTOfflineAvg(1:size(statusData,1))=NaN;
PMTOfflineAvg(OfflineRightFilter & statusData(:,805)==1)=PMTOfflineRightAvg(OfflineRightFilter & statusData(:,805)==1);
PMTOfflineAvg(OfflineLeftFilter & statusData(:,805)==2)=PMTOfflineLeftAvg(OfflineLeftFilter & statusData(:,805)==2);

PMTAvg(statusData(:,805)==3)=PMTOnlineAvg(statusData(:,805)==3);
PMTAvg(statusData(:,805)==2)=PMTOfflineLeftAvg(statusData(:,805)==2);
PMTAvg(statusData(:,805)==1)=PMTOfflineRightAvg(statusData(:,805)==1);
PMTAvg(statusData(:,805)==0)=NaN;

MCP1OnlineAvg(OnlineFilter)=AvgData(OnlineFilter,AVGBase+3);
MCP1OnlineAvg(~OnlineFilter)=NaN;
MCP1OfflineLeftAvg(OfflineLeftFilter)=AvgData(OfflineLeftFilter,AVGBase+4);
MCP1OfflineLeftAvg(~OfflineLeftFilter)=NaN;
MCP1OfflineRightAvg(OfflineRightFilter)=AvgData(OfflineRightFilter,AVGBase+5);
MCP1OfflineRightAvg(~OfflineRightFilter)=NaN;

MCP1OfflineAvg(1:size(statusData,1))=NaN;
MCP1OfflineAvg(OfflineRightFilter & statusData(:,805)==1)=MCP1OfflineRightAvg(OfflineRightFilter & statusData(:,805)==1);
MCP1OfflineAvg(OfflineLeftFilter & statusData(:,805)==2)=MCP1OfflineLeftAvg(OfflineLeftFilter & statusData(:,805)==2);

MCP1Avg(statusData(:,805)==3)=MCP1OnlineAvg(statusData(:,805)==3);
MCP1Avg(statusData(:,805)==2)=MCP1OfflineLeftAvg(statusData(:,805)==2);
MCP1Avg(statusData(:,805)==1)=MCP1OfflineRightAvg(statusData(:,805)==1);
MCP1Avg(statusData(:,805)==0)=NaN;

MCP2OnlineAvg(OnlineFilter)=AvgData(OnlineFilter,AVGBase+6);
MCP2OnlineAvg(~OnlineFilter)=NaN;
MCP2OfflineLeftAvg(OfflineLeftFilter)=AvgData(OfflineLeftFilter,AVGBase+7);
MCP2OfflineLeftAvg(~OfflineLeftFilter)=NaN;
MCP2OfflineRightAvg(OfflineRightFilter)=AvgData(OfflineRightFilter,AVGBase+8);
MCP2OfflineRightAvg(~OfflineRightFilter)=NaN;

MCP2OfflineAvg(1:size(statusData,1))=NaN;
MCP2OfflineAvg(OfflineRightFilter & statusData(:,805)==1)=MCP2OfflineRightAvg(OfflineRightFilter & statusData(:,805)==1);
MCP2OfflineAvg(OfflineLeftFilter & statusData(:,805)==2)=MCP2OfflineLeftAvg(OfflineLeftFilter & statusData(:,805)==2);

MCP2Avg(statusData(:,805)==3)=MCP2OnlineAvg(statusData(:,805)==3);
MCP2Avg(statusData(:,805)==2)=MCP2OfflineLeftAvg(statusData(:,805)==2);
MCP2Avg(statusData(:,805)==1)=MCP2OfflineRightAvg(statusData(:,805)==1);
MCP2Avg(statusData(:,805)==0)=NaN;

% display offline and online averages
set(handles.txtPMTOffline,'String',PMTOfflineAvg(lastrow));
set(handles.txtMCP1Offline,'String',MCP1OfflineAvg(lastrow));
set(handles.txtMCP2Offline,'String',MCP2OfflineAvg(lastrow));

set(handles.txtPMTOnline,'String',PMTOnlineAvg(lastrow));
set(handles.txtMCP1Online,'String',MCP1OnlineAvg(lastrow));
set(handles.txtMCP2Online,'String',MCP2OnlineAvg(lastrow));

% make plots 
hold(handles.axeRay,'off');
hold(handles.axeFluo,'off');
hold(handles.axeCounts,'off');

if get(handles.chkPMT,'Value')
    plot(handles.axeRay,statusData(lastrow,PMTBase+1:PMTBase+160),'r'); 
    hold(handles.axeRay,'on'); 
    z=find(data.PMTMask==0);
    PMTdata1=double(statusData(lastrow,PMTBase+1:PMTBase+160));
    PMTdata1(z)=NaN;
    plot(handles.axeRay,PMTdata1); 
    xlim(handles.axeRay,[1,160]); 
    
    xaxis=[40:160];
    plot(handles.axeFluo,xaxis,statusData(lastrow,PMTBase+40:PMTBase+160),'r');
    hold(handles.axeFluo,'on');
    y=data.PMTMask(40:160);
    z=find(y==0);
    PMTdata2=double(statusData(lastrow,PMTBase+40:PMTBase+160));
    PMTdata2(z)=NaN;
    plot(handles.axeFluo,xaxis,PMTdata2); 
    xlim(handles.axeFluo,[40,160]); 
    
    WhichPlot=get(handles.popPMTPlot,'Value');
    switch WhichPlot
        case 1
           plot(handles.axeCounts,statustime(iZeit),PMTSumCounts(iZeit)); %statusData(iZeit,PMTBase+204));
           hold(handles.axeCounts,'on');
        case 2           
           plot(handles.axeCounts,statustime(30:end),PMTAvg(30:end),'b');   
           hold(handles.axeCounts,'on');
    end
    hold(handles.axeCounts,'on');
end

if get(handles.chkMCP1,'Value')
    plot(handles.axeRay,statusData(lastrow,MCP1Base+1:MCP1Base+160),'r'); 
    hold(handles.axeRay,'on'); 
    z=find(data.MCP1Mask==0);
    MCP1data1=double(statusData(lastrow,MCP1Base+1:MCP1Base+160));
    MCP1data1(z)=NaN;
    plot(handles.axeRay,MCP1data1); 
    xlim(handles.axeRay,[1,160]); 
    
    xaxis=[40:160];
    plot(handles.axeFluo,xaxis, statusData(lastrow,MCP1Base+40:MCP1Base+160),'r');
    hold(handles.axeFluo,'on');
    y=data.MCP1Mask(40:160);
    z=find(y==0);
    MCP1data2=double(statusData(lastrow,MCP1Base+40:MCP1Base+160));
    MCP1data2(z)=NaN;
    plot(handles.axeFluo,xaxis,MCP1data2); 
    xlim(handles.axeFluo,[40,160]); 

    WhichPlot=get(handles.popMCP1Plot,'Value');
    switch WhichPlot
        case 1
           plot(handles.axeCounts,statustime(iZeit),MCP1SumCounts(iZeit)); %statusData(iZeit,MCP1Base+204));
           hold(handles.axeCounts,'on');
        case 2           
           plot(handles.axeCounts,statustime(30:end),MCP1Avg(30:end),'b');   
           hold(handles.axeCounts,'on');
    end
    hold(handles.axeCounts,'on');
end

if get(handles.chkMCP2,'Value')
    plot(handles.axeRay,statusData(lastrow,MCP2Base+1:MCP2Base+160),'r'); 
    hold(handles.axeRay,'on'); 
    z=find(data.MCP2Mask==0);
    MCP2data1=double(statusData(lastrow,MCP2Base+1:MCP2Base+160));
    MCP2data1(z)=NaN;
    plot(handles.axeRay,MCP2data1); 
    xlim(handles.axeRay,[1,160]); 
    
    xaxis=[40:160];
    plot(handles.axeFluo,xaxis,statusData(lastrow,MCP2Base+40:MCP2Base+160),'r');
    hold(handles.axeFluo,'on');
    y=data.MCP2Mask(40:160);
    z=find(y==0);
    MCP2data2=double(statusData(lastrow,MCP2Base+40:MCP2Base+160));
    MCP2data2(z)=NaN;
    plot(handles.axeFluo,xaxis,MCP2data2); 
    xlim(handles.axeFluo,[40,160]); 

    WhichPlot=get(handles.popMCP2Plot,'Value');
    switch WhichPlot
        case 1
           plot(handles.axeCounts,statustime(iZeit),MCP2SumCounts(iZeit)); %statusData(iZeit,MCP2Base+204));
           hold(handles.axeCounts,'on');
        case 2           
           plot(handles.axeCounts,statustime(30:end),MCP2Avg(30:end),'b');   
           hold(handles.axeCounts,'on');
    end
    hold(handles.axeCounts,'on');
end

xlim(handles.axeCounts,[minTime maxTime]);
grid(handles.axeCounts);

% check HV
if single(statusData(lastrow,725))==0
    set(handles.togHV,'Value',0)
    set(handles.togHV,'BackgroundColor','b','String','HV OFF');
else
    set(handles.togHV,'Value',1)
    set(handles.togHV,'BackgroundColor','r','String','HV ON');
end

% check Blower
if single(statusData(lastrow,727))==0
    set(handles.togBlower,'Value',0)
    set(handles.togBlower,'BackgroundColor','b','String','Blower OFF');
else
    set(handles.togBlower,'Value',1)
    set(handles.togBlower,'BackgroundColor','r','String','Blower ON');
end

% check Butterfly
if single(statusData(lastrow,726))==0
    set(handles.togButterfly,'Value',1)
    set(handles.togButterfly,'BackgroundColor','r','String','Butterfly OPEN');
else
    set(handles.togButterfly,'Value',0)
    set(handles.togButterfly,'BackgroundColor','b','String','Butterfly CLOSED');
end

data.lastrow=lastrow;
setappdata(handles.output, 'Detdata', data);

% --- Outputs from this function are returned to the command line.
function varargout = Detection_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in chkPMT.
function chkPMT_Callback(hObject, eventdata, handles)
% hObject    handle to chkPMT (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkPMT


% --- Executes on button press in chkMCP2.
function chkMCP2_Callback(hObject, eventdata, handles)
% hObject    handle to chkMCP2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkMCP2


% --- Executes on button press in chkMCP1.
function chkMCP1_Callback(hObject, eventdata, handles)
% hObject    handle to chkMCP1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkMCP1


% --- Executes on button press in pshExit.
function pshExit_Callback(hObject, eventdata, handles)
% hObject    handle to pshExit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of pshExit
stop(handles.Timer);
delete(handles.Timer);
close(handles.figure1);


% --- Executes on selection change in popPMTPlot.
function popPMTPlot_Callback(hObject, eventdata, handles)
% hObject    handle to popPMTPlot (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns popPMTPlot contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popPMTPlot


% --- Executes during object creation, after setting all properties.
function popPMTPlot_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popPMTPlot (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on selection change in popMCP1Plot.
function popMCP1Plot_Callback(hObject, eventdata, handles)
% hObject    handle to popMCP1Plot (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns popMCP1Plot contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popMCP1Plot


% --- Executes during object creation, after setting all properties.
function popMCP1Plot_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popMCP1Plot (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on selection change in popMCP2Plot.
function popMCP2Plot_Callback(hObject, eventdata, handles)
% hObject    handle to popMCP2Plot (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns popMCP2Plot contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popMCP2Plot


% --- Executes during object creation, after setting all properties.
function popMCP2Plot_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popMCP2Plot (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on button press in chkWZout.
function chkWZout_Callback(hObject, eventdata, handles)
% hObject    handle to chkWZout (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkWZout


% --- Executes on button press in chkWZin.
function chkWZin_Callback(hObject, eventdata, handles)
% hObject    handle to chkWZin (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkWZin


% --- Executes on button press in chkPNO.
function chkPNO_Callback(hObject, eventdata, handles)
% hObject    handle to chkPNO (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkPNO


% --- Executes on button press in chkP20.
function chkP20_Callback(hObject, eventdata, handles)
% hObject    handle to chkP20 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkP20


% --- Executes on button press in chkP1000.
function chkP1000_Callback(hObject, eventdata, handles)
% hObject    handle to chkP1000 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkP1000


% --- Executes on button press in togglebutton3.
function togBlower_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton3
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
data = getappdata(handles.output, 'Dyelaserdata');
lastrow=data.lastrow;

if get(hObject,'Value')
%    if single(statusData(:,ADCBase1))<? % switch on Blower only if cell pressure P1000 is low enough
        system('/lift/bin/eCmd w 0xa464 1800');
        set(hObject,'BackgroundColor','r','String','Blower ON');
%    end
else
    system('/lift/bin/eCmd w 0xa464 0');
    set(hObject,'BackgroundColor','b','String','Blower OFF');
end



% --- Executes on button press in togHV.
function togHV_Callback(hObject, eventdata, handles)
% hObject    handle to togHV (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togHV
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
data = getappdata(handles.output, 'Dyelaserdata');
lastrow=data.lastrow;

if get(hObject,'Value')
%    if single(statusData(lastrow,ADCBase1+1*3))<? % switch on HV only if cell pressure P20 is low
        system(['/lift/bin/eCmd w 0xa460 ', num2str(uint16(13*140))]); % 13V needed for HV
        set(hObject,'BackgroundColor','r','String','HV ON');
%    end
else
    system('/lift/bin/eCmd w 0xa460 0');
    set(hObject,'BackgroundColor','b','String','HV OFF');
end


% --- Executes on button press in togglebutton5.
function togButterfly_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton5 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton5
if get(hObject,'Value')
    system(['/lift/bin/eCmd w 0xa462 0']);
    set(hObject,'BackgroundColor','r','String','Butterfly OPEN');
else
    system('/lift/bin/eCmd w 0xa462 1800'); 
    set(hObject,'BackgroundColor','b','String','Butterfly CLOSED');
end



% --- Executes on button press in chkTDet.
function chkTDet_Callback(hObject, eventdata, handles)
% hObject    handle to chkTDet (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkTDet


% --- Executes on button press in chkVHV.
function chkVHV_Callback(hObject, eventdata, handles)
% hObject    handle to chkVHV (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkVHV


