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

% Last Modified by GUIDE v2.5 12-Jul-2007 19:08:58

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

data.Timer=handles.Timer;  
data.Counter=0;
data.CounterOnl=0;
data.CounterOffl=0;
data.sumDiodeWZ1inonl=0;
data.sumDiodeWZ1outonl=0;
data.sumDiodeWZ2inonl=0;
data.sumDiodeWZ2outonl=0;
data.sumDiodeWZ1inoffl=0;
data.sumDiodeWZ1outoffl=0;
data.sumDiodeWZ2inoffl=0;
data.sumDiodeWZ2outoffl=0;
data.sumctsMCP1onl=0;
data.sumctsMCP1offl=0;
data.sumctsMCP2onl=0;
data.sumctsMCP2offl=0;
data.sumH2O=0;
data.averaging=0;

% Update handles structure
guidata(hObject, handles);
% UIWAIT makes Calibration wait for user response (see UIRESUME)
% uiwait(handles.figure1);
setappdata(handles.output, 'Caldata', data);
start(handles.Timer);

function CalRefresh(arg1,arg2,handles)

data = getappdata(handles.output, 'Caldata');

horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
AvgData=horusdata.AvgData;
col=horusdata.col;
fcts2val=horusdata.fcts2val;

calib=ReadCalibData('/lift/ramdisk/status.cal');

% Calculate time as sum of day, hour, min, etc.
statustime=double(statusData(:,2))./1.0+ ...
           double(statusData(:,3))./24.0+...
           double(statusData(:,4))./1440.0+...
           double(statusData(:,5))./86400.0+...
           double(statusData(:,6))./86400000.0;

[SortZeit,indexZeit]=sort(statustime);
maxLen=size(statustime,1);
lastrow=indexZeit(maxLen);

PlotWidth=maxLen;
stopPlot=maxLen;
startPlot=1;
iZeit=indexZeit(startPlot:stopPlot);
minTime=statustime(iZeit(1));
maxTime=statustime(iZeit(size(iZeit,1)));

% calculate parameters from ADC counts
x=double(statusData(:,col.DiodeUV)); eval(['DiodeUV=',fcts2val.DiodeUV,';']);
x=double(statusData(:,col.DiodeWZ1out)); eval(['DiodeWZ1out=',fcts2val.DiodeWZ1out,';']);
x=double(statusData(:,col.DiodeWZ2out)); eval(['DiodeWZ2out=',fcts2val.DiodeWZ2out,';']);
x=double(statusData(:,col.DiodeWZ1in)); eval(['DiodeWZ1in=',fcts2val.DiodeWZ1in,';']);
x=double(statusData(:,col.DiodeWZ2in)); eval(['DiodeWZ2in=',fcts2val.DiodeWZ2in,';']);

ctsMCP1=double(statusData(:,col.ccCounts1));
ctsMCP2=double(statusData(:,col.ccCounts2));
OnOffFlag=statusData(:,col.RAvgOnOffFlag);

H2O=double(calib.LicorCalib.H2OB);
Pamb=double(calib.LicorCalib.AmbientPressure)./10;
TLicor=double(calib.LicorCalib.LicorTemperature)/100-273.15;
TH2O=double(calib.PIDRegulator.ActualValueH2O)/100-273.15;

x=double(calib.MFCCardCalib.ChannelData0.Flow); eval(['Flow0=',fcts2val.CalFlow0,';']);
x=double(calib.MFCCardCalib.ChannelData1.Flow); eval(['Flow1=',fcts2val.CalFlow1,';']);
x=double(calib.MFCCardCalib.ChannelData2.Flow); eval(['Flow2=',fcts2val.CalFlow2,';']);
x=double(calib.MFCCardCalib.ChannelData3.Flow); eval(['Flow3=',fcts2val.CalFlow3,';']);

Humid=Flow0./(Flow0+Flow1).*100;

set(handles.textDUV,'String',[num2str(DiodeUV(lastrow),3),' mW']);
set(handles.textH2O,'String',[num2str(H2O(lastrow),3),' ppm']);
set(handles.textPamb,'String',[num2str(Pamb(lastrow),4),' mbar']);
set(handles.textTLicor,'String',[num2str(TLicor(lastrow),4),' C']);
set(handles.textTH2O,'String',[num2str(TH2O(lastrow),3),' C']);
set(handles.textFlowCal,'String',[num2str(Flow0(lastrow)+Flow1(lastrow),5),' sccm']);
set(handles.textFlowLicor,'String',[num2str((Flow2(lastrow)+Flow3(lastrow))/2,5),' sccm']);
set(handles.textHumid,'String',[num2str(Humid(lastrow),3),' %']);

% warn with red background if values are off limits
if Flow2~=300 | Flow3~=300
    set(handles.textFlowLicor,'BackgroundColor','r');
else
    set(handles.textFlowLicor,'BackgroundColor',[0.7,0.7,0.7]);
end

hold(handles.axes1,'off'); 

if get(handles.chkDUV,'Value')
    plot(handles.axes1,statustime(iZeit),DiodeUV(iZeit),'b');
    hold(handles.axes1,'on');
end 

if get(handles.chkH2O,'Value')
    plot(handles.axes1,statustime(iZeit),H2O(iZeit),'b');
    hold(handles.axes1,'on');
end 

if get(handles.chkPamb,'Value')
    plot(handles.axes1,statustime(iZeit),Pamb(iZeit),'b');
    hold(handles.axes1,'on');
end 

if get(handles.chkTLicor,'Value')
    plot(handles.axes1,statustime(iZeit),TLicor(iZeit),'b');
    hold(handles.axes1,'on');
end 

if get(handles.chkTH2O,'Value')
    plot(handles.axes1,statustime(iZeit),TH2O(iZeit),'r');
    hold(handles.axes1,'on');
end 

if get(handles.chkFlowCal,'Value')
    plot(handles.axes1,statustime(iZeit),Flow0(iZeit)+Flow1(iZeit),'r');
    hold(handles.axes1,'on');
end 

if get(handles.chkFlowLicor,'Value')
    plot(handles.axes1,statustime(iZeit),(Flow2(iZeit)+Flow3(iZeit))/2,'r');
    hold(handles.axes1,'on');
end 

if get(handles.chkHumid,'Value')
    plot(handles.axes1,statustime(iZeit),Humid(iZeit),'r');
    hold(handles.axes1,'on');
end 

xlim(handles.axes1,[minTime maxTime]);
grid(handles.axes1);


% calculate running averages
if data.averaging==1
for i=1:5
    data.Counter=data.Counter+1;
    data.sumH2O=data.sumH2O+H2O(lastrow-5+i);
    if OnOffFlag(lastrow-5+i)==3
        data.sumDiodeWZ1inonl=data.sumDiodeWZ1inonl+DiodeWZ1in(lastrow-5+i);
        data.sumDiodeWZ1outonl=data.sumDiodeWZ1outonl+DiodeWZ1out(lastrow-5+i);
        data.sumDiodeWZ2inonl=data.sumDiodeWZ2inonl+DiodeWZ2in(lastrow-5+i);
        data.sumDiodeWZ2outonl=data.sumDiodeWZ2outonl+DiodeWZ2out(lastrow-5+i);
        data.sumctsMCP1onl=data.sumctsMCP1onl+ctsMCP1(lastrow-5+i);
        data.sumctsMCP2onl=data.sumctsMCP2onl+ctsMCP2(lastrow-5+i);
        data.CounterOnl=data.CounterOnl+1;
    elseif OnOffFlag(lastrow-5+i)==1 | OnOffFlag(lastrow-5+i)==2
        data.sumDiodeWZ1inoffl=data.sumDiodeWZ1inoffl+DiodeWZ1in(lastrow-5+i);
        data.sumDiodeWZ1outoffl=data.sumDiodeWZ1outoffl+DiodeWZ1out(lastrow-5+i);
        data.sumDiodeWZ2inoffl=data.sumDiodeWZ2inoffl+DiodeWZ2in(lastrow-5+i);
        data.sumDiodeWZ2outoffl=data.sumDiodeWZ2outoffl+DiodeWZ2out(lastrow-5+i);
        data.sumctsMCP1offl=data.sumctsMCP1offl+ctsMCP1(lastrow-5+i);
        data.sumctsMCP2offl=data.sumctsMCP2offl+ctsMCP2(lastrow-5+i);
        data.CounterOffl=data.CounterOffl+1;
    end
end
if data.CounterOnl>0
    avgDiodeWZ1inonl=data.sumDiodeWZ1inonl/data.CounterOnl;
    avgDiodeWZ1outonl=data.sumDiodeWZ1outonl/data.CounterOnl;
    avgDiodeWZ2inonl=data.sumDiodeWZ2inonl/data.CounterOnl;
    avgDiodeWZ2outonl=data.sumDiodeWZ2outonl/data.CounterOnl;
    avgMCP1onl=data.sumctsMCP1onl/data.CounterOnl;
    avgMCP2onl=data.sumctsMCP2onl/data.CounterOnl;
else
    avgDiodeWZ1inonl=NaN;
    avgDiodeWZ1outonl=NaN;
    avgDiodeWZ2inonl=NaN;
    avgDiodeWZ2outonl=NaN;
    avgMCP1onl=NaN;
    avgMCP2onl=NaN;
end
if data.CounterOffl>0
    avgDiodeWZ1inoffl=data.sumDiodeWZ1inoffl/data.CounterOffl;
    avgDiodeWZ1outoffl=data.sumDiodeWZ1outoffl/data.CounterOffl;
    avgDiodeWZ2inoffl=data.sumDiodeWZ2inoffl/data.CounterOffl;
    avgDiodeWZ2outoffl=data.sumDiodeWZ2outoffl/data.CounterOffl;
    avgMCP1offl=data.sumctsMCP1offl/data.CounterOffl;
    avgMCP2offl=data.sumctsMCP2offl/data.CounterOffl;
else
    avgDiodeWZ1inoffl=NaN;
    avgDiodeWZ1outoffl=NaN;
    avgDiodeWZ2inoffl=NaN;
    avgDiodeWZ2outoffl=NaN;
    avgMCP1offl=NaN;
    avgMCP2offl=NaN;
end
if data.Counter>0
    avgH2O=data.sumH2O/data.Counter;
end
    
% display averages
set(handles.txtCounts,'String',[num2str(data.Counter)]);
set(handles.txtH2O,'String',[num2str(avgH2O,3),' ppm']);
set(handles.txtWZ1inonl,'String',[num2str(avgDiodeWZ1inonl,3),' mW']);
set(handles.txtWZ1outonl,'String',[num2str(avgDiodeWZ1outonl,3),' mW']);
set(handles.txtWZ2inonl,'String',[num2str(avgDiodeWZ2inonl,3),' mW']);
set(handles.txtWZ2outonl,'String',[num2str(avgDiodeWZ2outonl,3),' mW']);
set(handles.txtWZ1inoffl,'String',[num2str(avgDiodeWZ1inoffl,3),' mW']);
set(handles.txtWZ1outoffl,'String',[num2str(avgDiodeWZ1outoffl,3),' mW']);
set(handles.txtWZ2inoffl,'String',[num2str(avgDiodeWZ2inoffl,3),' mW']);
set(handles.txtWZ2outoffl,'String',[num2str(avgDiodeWZ2outoffl,3),' mW']);
set(handles.txtMCP1onl,'String',[num2str(avgMCP1onl,3)]);
set(handles.txtMCP1offl,'String',[num2str(avgMCP1offl,3)]);
set(handles.txtMCP2onl,'String',[num2str(avgMCP2onl,3)]);
set(handles.txtMCP2offl,'String',[num2str(avgMCP2offl,3)]);
set(handles.txtMCP2offl,'String',[num2str(avgMCP2offl,3)]);

end
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
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;

% Calculate time as sum of day, hour, min, etc.
statustime=double(statusData(:,2))./1.0+ ...
           double(statusData(:,3))./24.0+...
           double(statusData(:,4))./1440.0+...
           double(statusData(:,5))./86400.0;

[SortZeit,indexZeit]=sort(statustime);
maxLen=size(statustime,1);
lastrow=indexZeit(maxLen);

if get(hObject,'Value')
    set(hObject,'String','Stop')
    data.Counter=0;
    data.CounterOnl=0;
    data.CounterOffl=0;
    data.sumDiodeWZ1inonl=0;
    data.sumDiodeWZ1outonl=0;
    data.sumDiodeWZ2inonl=0;
    data.sumDiodeWZ2outonl=0;
    data.sumDiodeWZ1inoffl=0;
    data.sumDiodeWZ1outoffl=0;
    data.sumDiodeWZ2inoffl=0;
    data.sumDiodeWZ2outoffl=0;
    data.sumctsMCP1onl=0;
    data.sumctsMCP1offl=0;
    data.sumctsMCP2onl=0;
    data.sumctsMCP2offl=0;
    data.sumH2O=0;
    data.averaging=1;
    set(handles.txtStartTime,'String',strcat(datestr(statustime(lastrow),13)));
else
    set(hObject,'String','Start')
    % display system time
    data.averaging=0;
    set(handles.txtStopTime,'String',strcat(datestr(statustime(lastrow),13)));
end

setappdata(handles.output, 'Caldata', data);


% --- Executes on button press in chkH2O.
function chkH2O_Callback(hObject, eventdata, handles)
% hObject    handle to chkH2O (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkH2O


% --- Executes on button press in chkDUV.
function chkDUV_Callback(hObject, eventdata, handles)
% hObject    handle to chkDUV (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkDUV


% --- Executes on button press in chkFlowCal.
function chkFlowCal_Callback(hObject, eventdata, handles)
% hObject    handle to chkFlowCal (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkFlowCal


% --- Executes on button press in chkFlowLicor.
function chkFlowLicor_Callback(hObject, eventdata, handles)
% hObject    handle to chkFlowLicor (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkFlowLicor


% --- Executes on button press in chkTLicor.
function chkTLicor_Callback(hObject, eventdata, handles)
% hObject    handle to chkTLicor (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkTLicor


% --- Executes on button press in chkPamb.
function chkPamb_Callback(hObject, eventdata, handles)
% hObject    handle to chkPamb (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkPamb


% --- Executes on button press in chkTH2O.
function chkTH2O_Callback(hObject, eventdata, handles)
% hObject    handle to chkTH2O (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkTH2O


% --- Executes on button press in pushSetFlowLicor.
function pushSetFlowLicor_Callback(hObject, eventdata, handles)
% hObject    handle to pushSetFlowLicor (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
x=str2double(get(handles.editFlowLicor,'String'));
if x==0
    SetFlow2=0;
    SetFlow3=0;
else
    eval(['SetFlow2=',fcts2val.CalFlow2Set,';']);
    eval(['SetFlow3=',fcts2val.CalFlow3Set,';']);
end

if ~isnan(Flow)
    system(['/lift/bin/eCmd @armCalib s calibflow 2 +', num2str(SetFlow2)]);   
    system(['/lift/bin/eCmd @armCalib s calibflow 3 +', num2str(SetFlow3)]);   
end



function editFlowLicor_Callback(hObject, eventdata, handles)
% hObject    handle to editFlowLicor (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editFlowLicor as text
%        str2double(get(hObject,'String')) returns contents of editFlowLicor as a double
Flow=uint16(str2double(get(hObject,'String')));
if Flow>500 Flow=500; end
if isnan(Flow)
    set(hObject,'BackgroundColor','red');
else
    set(hObject,'BackgroundColor','white');
end


% --- Executes during object creation, after setting all properties.
function editFlowLicor_CreateFcn(hObject, eventdata, handles)
% hObject    handle to editFlowLicor (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in pushSetFlowCal.
function pushSetFlowCal_Callback(hObject, eventdata, handles)
% hObject    handle to pushSetFlowCal (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
Flow=str2double(get(handles.editFlowCal,'String'));
Humid=str2double(get(handles.editHumid,'String'))/100;
FlowOn=get(handles.tglAir,'Value');
Flow0=Humid*Flow;
Flow1=Flow-Flow0;
if Flow0==0
    SetFlow0=0;
else
    x=Flow0; eval(['SetFlow0=',fcts2val.CalFlow0Set,';']);
end
if Flow1==0
    SetFlow1=0;
else
    x=Flow1; eval(['SetFlow1=',fcts2val.CalFlow1Set,';']);
end
if ~isnan(Flow0) & ~isnan(Flow1) & FlowOn==1 
    system(['/lift/bin/eCmd @armCalib s calibflow 0 +', num2str(SetFlow0)]);   
    system(['/lift/bin/eCmd @armCalib s calibflow 1 +', num2str(SetFlow1)]);   
end



function editFlowCal_Callback(hObject, eventdata, handles)
% hObject    handle to editFlowCal (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editFlowCal as text
%        str2double(get(hObject,'String')) returns contents of editFlowCal as a double
Flow=uint16(str2double(get(hObject,'String')));
if Flow>50000 Flow=50000; end
if isnan(Flow)
    set(hObject,'BackgroundColor','red');
else
    set(hObject,'BackgroundColor','white');
end


% --- Executes during object creation, after setting all properties.
function editFlowCal_CreateFcn(hObject, eventdata, handles)
% hObject    handle to editFlowCal (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in pushSetTH2O.
function pushSetTH2O_Callback(hObject, eventdata, handles)
% hObject    handle to pushSetTH2O (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
TH2O=(str2double(get(handles.editTH2O,'String'))+273.15)*100;
if ~isnan(TH2O)
    system(['/lift/bin/eCmd @armCalib s calibwatertemp ', num2str(TH2O)]);   
end




function editTH2O_Callback(hObject, eventdata, handles)
% hObject    handle to editTH2O (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editTH2O as text
%        str2double(get(hObject,'String')) returns contents of editTH2O as a double
TH2O=uint16(str2double(get(hObject,'String')));
if TH2O>40 TH2O=40; end
if isnan(TH2O)
    set(hObject,'BackgroundColor','red');
else
    set(hObject,'BackgroundColor','white');
end


% --- Executes during object creation, after setting all properties.
function editTH2O_CreateFcn(hObject, eventdata, handles)
% hObject    handle to editTH2O (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in chkHumid.
function chkHumid_Callback(hObject, eventdata, handles)
% hObject    handle to chkHumid (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkHumid


% --- Executes on button press in pushSethumid.
function pushSethumid_Callback(hObject, eventdata, handles)
% hObject    handle to pushSethumid (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)




% --- Executes on button press in tglAir.
function tglAir_Callback(hObject, eventdata, handles)
% hObject    handle to tglAir (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if get(hObject,'Value')
    Flow=str2double(get(handles.editFlowCal,'String'));
    Humid=str2double(get(handles.editHumid,'String'))/100;
    FlowOn=get(handles.tglAir,'Value');
    Flow0=Humid*Flow;
    Flow1=Flow-Flow0;
    if Flow0==0
        SetFlow0=0;
    else
        x=Flow0; eval(['SetFlow0=',fcts2val.CalFlow0Set,';']);
    end
    if Flow1==0
        SetFlow1=0;
    else
        x=Flow1; eval(['SetFlow1=',fcts2val.CalFlow1Set,';']);
    end
    if ~isnan(Flow0) & ~isnan(Flow1)
        system(['/lift/bin/eCmd @armCalib s calibflow 0 +', num2str(SetFlow0)]);   
        system(['/lift/bin/eCmd @armCalib s calibflow 1 +', num2str(SetFlow1)]);   
        set(hObject,'BackgroundColor','g');
    else
        set(hObject,'BackgroundColor','r');
    end
else
    system(['/lift/bin/eCmd @armCalib s calibflow 0 +0']);   
    system(['/lift/bin/eCmd @armCalib s calibflow 1 +0']);   
    set(hObject,'BackgroundColor','c');
end


function editHumid_Callback(hObject, eventdata, handles)
% hObject    handle to editHumid (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editHumid as text
%        str2double(get(hObject,'String')) returns contents of editHumid as a double
x=uint16(str2double(get(hObject,'String')));
if x>100 x=100; end
if isnan(x)
    set(hObject,'BackgroundColor','red');
else
    set(hObject,'BackgroundColor','white');
end


% --- Executes during object creation, after setting all properties.
function editHumid_CreateFcn(hObject, eventdata, handles)
% hObject    handle to editHumid (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in pushSetHumid.
function pushSetHumid_Callback(hObject, eventdata, handles)
% hObject    handle to pushSetHumid (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
Flow=str2double(get(handles.editFlowCal,'String'));
Humid=str2double(get(handles.editHumid,'String'))/100;
FlowOn=get(handles.tglAir,'Value');
Flow0=Humid*Flow;
Flow1=Flow-Flow0;
if Flow0==0
    SetFlow0=0;
else
    x=Flow0; eval(['SetFlow0=',fcts2val.CalFlow0Set,';']);
end
if Flow1==0
    SetFlow1=0;
else
    x=Flow1; eval(['SetFlow1=',fcts2val.CalFlow1Set,';']);
end
if ~isnan(Flow0) & ~isnan(Flow1) & FlowOn==1 
    system(['/lift/bin/eCmd @armCalib s calibflow 0 +', num2str(SetFlow0)]);   
    system(['/lift/bin/eCmd @armCalib s calibflow 1 +', num2str(SetFlow1)]);   
end


