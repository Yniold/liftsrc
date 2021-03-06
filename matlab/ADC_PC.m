function varargout = ADC(varargin)
% ADC M-file for ADC.fig
%      ADC, by itself, creates a new ADC or raises the existing
%      singleton*.
%
%      H = ADC returns the handle to a new ADC or the handle to
%      the existing singleton*.
%
%      ADC('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in ADC.M with the given input arguments.
%
%      ADC('Property','Value',...) creates a new ADC or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before ADC_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to ADC_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help ADC

% Last Modified by GUIDE v2.5 14-Jun-2004 20:43:38

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @ADC_OpeningFcn, ...
                   'gui_OutputFcn',  @ADC_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin & isstr(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT

% --- Executes just before ADC is made visible.
function ADC_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to ADC (see VARARGIN)

% Choose default command line output for ADC
handles.output = hObject;

    c=@PlotRefresh;
    %setup Timer function
    handles.ActTimer = timer('ExecutionMode','fixedDelay',...
          'Period',0.5,...    
          'BusyMode','drop',...
          'TimerFcn', {@PlotRefresh,handles});   


% Update handles structure
guidata(hObject, handles);

% UIWAIT makes ADC wait for user response (see UIRESUME)
% uiwait(handles.figDataGUI);


function PlotRefresh(arg1,arg2,GUI_handles)


%[s,w] = system('tail -n-10 data/ccStatus.txt > data/status_sub.txt');
%clear status_sub;
%load 'data/status_sub.txt';

%figure(GUI_handles.figDataGUI);
%set(GUI_handles.figDataGUI,'Visible','on');

statusData=ReadData('/Lift/data/status.bin');
statustime=double(statusData(:,2))./1.0+ ...
           double(statusData(:,3))./24.0+...
           double(statusData(:,4))./1440.0+...
           double(statusData(:,5))./86400.0;

%lastrow=size(statusData,1);
       
ZeitTage=double(statusData(:,2))/1.0+double(statusData(:,3))/24.0+...
    double(statusData(:,4))/1440.0+...
    double(statusData(:,5))/86400.0+...
    double(statusData(:,6))/86400000.0;
Stunden=double(statusData(:,3))+...
    double(statusData(:,4))/60.0+...
    double(statusData(:,5))/3600.0+...
    double(statusData(:,6))/3600000.0;
Minuten=double(statusData(:,4))+...
    double(statusData(:,5))/60.0+...
    double(statusData(:,6))/60000.0;
Zeit=Stunden/24.0;
%[maxTime,lastrow]=max(ZeitTage)
[SortZeit,indexZeit]=sort(ZeitTage);
maxLen=size(ZeitTage,1);
lastrow=indexZeit(maxLen);

PlotWidth=max(int16((1-get(GUI_handles.sldTimeWidth,'Value'))*maxLen),2);
set(GUI_handles.txtTimeWidth,'String',PlotWidth);

stopPlot=max(maxLen-int16((1-get(GUI_handles.sldTimeStartPos,'Value'))*maxLen),5);
set(GUI_handles.txtTimeStartPos,'String',stopPlot);

startPlot=max(stopPlot-PlotWidth,1);
startPlot=min(startPlot,995);
iZeit=indexZeit(startPlot:stopPlot);
minTime=Zeit(iZeit(1));
maxTime=Zeit(iZeit(size(iZeit,1)));

%[startPlot stopPlot]
%[minTime maxTime]
%ZeitTage(lastrow)
set(GUI_handles.txtTimer,'String',strcat(datestr(Stunden(lastrow)/24.0,13),'_',num2str(statusData(lastrow,6))));
%set(GUI_handles.txtTimer findall(GUI_figure,'Tag','txtTimer'),'String',statusData(lastrow,6));

ccADCBase=7;
set(GUI_handles.ADCWert1,'String',statusData(lastrow,ccADCBase+0));
set(GUI_handles.ADCWert2,'String',statusData(lastrow,ccADCBase+1));
set(GUI_handles.ADCWert3,'String',statusData(lastrow,ccADCBase+2));
set(GUI_handles.ADCWert4,'String',statusData(lastrow,ccADCBase+3));
set(GUI_handles.ADCWert5,'String',statusData(lastrow,ccADCBase+4));
set(GUI_handles.ADCWert6,'String',statusData(lastrow,ccADCBase+5));
set(GUI_handles.ADCWert7,'String',statusData(lastrow,ccADCBase+6));
set(GUI_handles.ADCWert8,'String',statusData(lastrow,ccADCBase+7));

EtalonBase=642; 
help=int32(statusData(:,EtalonBase)); %+65536.*statusData(:,EtalonBase+1));
EtalonSetPos=(help); EtalonSetPos(help>32767)=EtalonSetPos(help>32767)-65535;

help=int32(statusData(:,EtalonBase+2)); %+65536.*statusData(:,EtalonBase+1));
EtalonCurPos=(help); EtalonCurPos(help>32767)=EtalonCurPos(help>32767)-65535;

help=int32(statusData(:,EtalonBase+4)); %+65536.*statusData(:,EtalonBase+1));
EtalonEncPos=(help); EtalonEncPos(help>32767)=EtalonEncPos(help>32767)-65535;

help=int32(statusData(:,EtalonBase+6)); %+65536.*statusData(:,EtalonBase+1));
EtalonIndPos=(help); EtalonIndPos(help>32767)=EtalonIndPos(help>32767)-65535;

EtalonSpeed=statusData(:,EtalonBase+10);
EtalonStatus=statusData(:,EtalonBase+11);

%EtalonCurPos=statusData(:,EtalonBase+2); %+65536.*statusData(:,EtalonBase+3);
%EtalonEncPos=int8(statusData(:,EtalonBase+4)); %+65536.*statusData(:,EtalonBase+5))
%EtalonIndPos=statusData(:,EtalonBase+6); %+65536.*statusData(:,EtalonBase+7);

set(GUI_handles.radLeftEnd,'Value',bitand(EtalonStatus(lastrow),1));
set(GUI_handles.radRightEnd,'Value',bitand(EtalonStatus(lastrow),2));

set(GUI_handles.EtalonSpd,'String',EtalonSpeed(lastrow));
set(GUI_handles.EtalonEncPos,'String',EtalonEncPos(lastrow));
set(GUI_handles.EtalonCurPos,'String',EtalonCurPos(lastrow));
set(GUI_handles.EtalonSetPos,'String',EtalonSetPos(lastrow));
set(GUI_handles.EtalonIndPos,'String',EtalonIndPos(lastrow));
set(GUI_handles.txtEtalonStatus,'String',EtalonStatus(lastrow));

hold(GUI_handles.axeADC,'off'); 

if get(GUI_handles.chkADC1,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ccADCBase+0));
    hold(GUI_handles.axeADC,'on');
end 

if get(GUI_handles.chkADC2,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ccADCBase+1));
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkADC3,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ccADCBase+2),'r');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkADC4,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ccADCBase+3),'g');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkADC5,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ccADCBase+4),'b');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkADC6,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ccADCBase+5),'m');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkADC7,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ccADCBase+6),'c');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkADC8,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ccADCBase+8),'y');
    hold(GUI_handles.axeADC,'on');
end 

if get(GUI_handles.chkEtalonSpd,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),EtalonStatus(iZeit),'k');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkEtalonEncPos,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),EtalonEncPos(iZeit),'k');
    hold(GUI_handles.axeADC,'on');
end 

if get(GUI_handles.chkEtalonCurPos,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),EtalonCurPos(iZeit),'b');
    hold(GUI_handles.axeADC,'on');
end 

if get(GUI_handles.chkEtalonSetPos,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),EtalonSetPos(iZeit),'g');
    hold(GUI_handles.axeADC,'on');
end 

timeStep=double(10.0/86400.0);
if (maxTime-minTime>0.7/1400.0)
    timeStep=1.0/1440.0;
end
timeXTick=[double(minTime):double(timeStep):double(maxTime)];
xlim(GUI_handles.axeADC,[minTime maxTime]);
datetick(GUI_handles.axeADC,'x',13,'keeplimits');
set(GUI_handles.axeADC,'XTick',timeXTick);
grid(GUI_handles.axeADC);
%xlim(GUI_handles.axeADC,[minTime maxTime]);
plot(GUI_handles.axeEtalon,EtalonEncPos(iZeit),EtalonCurPos(iZeit),'.');


PMTBase=18;
MCP1Base=227;
MCP2Base=436;
set(GUI_handles.PMTCounts,'String',statusData(lastrow,PMTBase+204));
set(GUI_handles.MCP1Counts,'String',statusData(lastrow,MCP1Base+204));
set(GUI_handles.MCP2Counts,'String',statusData(lastrow,MCP2Base+204));

set(GUI_handles.PMTPulses,'String',statusData(lastrow,PMTBase+205));
set(GUI_handles.MCP1Pulses,'String',statusData(lastrow,MCP1Base+205));
set(GUI_handles.MCP2Pulses,'String',statusData(lastrow,MCP2Base+205));

hold(GUI_handles.axeRay,'off');
hold(GUI_handles.axeFluo,'off');
hold(GUI_handles.axeCounts,'off');
hold(GUI_handles.axeCountsEtalon,'off');

if get(GUI_handles.chkPMT,'Value')
    plot(GUI_handles.axeRay,statusData(lastrow,PMTBase+1:PMTBase+160));
    plot(GUI_handles.axeFluo,statusData(lastrow,PMTBase+30:PMTBase+160));
    plot(GUI_handles.axeCounts,Zeit(iZeit),statusData(iZeit,PMTBase+204));
    plot(GUI_handles.axeCountsEtalon,EtalonEncPos,statusData(:,PMTBase+204),'.');
    hold(GUI_handles.axeRay,'on');
    hold(GUI_handles.axeFluo,'on');
    hold(GUI_handles.axeCounts,'on');
    hold(GUI_handles.axeCountsEtalon,'on');

end

if get(GUI_handles.chkMCP1,'Value')
    plot(GUI_handles.axeRay,statusData(lastrow,MCP1Base+1:MCP1Base+160),'r');
    plot(GUI_handles.axeFluo,statusData(lastrow,MCP1Base+30:MCP1Base+160),'r');
    plot(GUI_handles.axeCounts,Zeit(iZeit),statusData(iZeit,MCP1Base+204),'r');
    plot(GUI_handles.axeCountsEtalon,EtalonEncPos,statusData(:,MCP1Base+204),'r.');
    hold(GUI_handles.axeRay,'on');
    hold(GUI_handles.axeFluo,'on');
    hold(GUI_handles.axeCounts,'on');
    hold(GUI_handles.axeCountsEtalon,'on');
end

if get(GUI_handles.chkMCP2,'Value')
    plot(GUI_handles.axeRay,statusData(lastrow,MCP2Base+1:MCP2Base+160),'g');
    plot(GUI_handles.axeFluo,statusData(lastrow,MCP2Base+30:MCP2Base+160),'g');
    plot(GUI_handles.axeCounts,Zeit(iZeit),statusData(iZeit,MCP2Base+204),'g');
    plot(GUI_handles.axeCountsEtalon,EtalonEncPos,statusData(:,MCP2Base+204),'g.');
    hold(GUI_handles.axeRay,'on');
    hold(GUI_handles.axeFluo,'on');
    hold(GUI_handles.axeCounts,'on');
    hold(GUI_handles.axeCountsEtalon,'on');
end

xlim(GUI_handles.axeCounts,[minTime maxTime]);
datetick(GUI_handles.axeCounts,'x',13,'keeplimits');
set(GUI_handles.axeCounts,'XTick',timeXTick);
grid(GUI_handles.axeCounts);




% --- Outputs from this function are returned to the command line.
function varargout = ADC_OutputFcn(hObject, eventdata, handles)
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;

% --- Executes on button press in togglebutton1.
function togglebutton1_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton1

ToggleState=get(hObject,'Value');
if ToggleState==0
    stop(handles.ActTimer);
    set(hObject,'String','Wait'); 
else

    start(handles.ActTimer);
    set(hObject,'String','Plotting...');

end


% --- Executes on button press in chkEtalonSpd.
function chkEtalonSpd_Callback(hObject, eventdata, handles)
% hObject    handle to chkEtalonSpd (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkEtalonSpd


% --- Executes on button press in chkEtalonEncPos.
function chkEtalonEncPos_Callback(hObject, eventdata, handles)
% hObject    handle to chkEtalonEncPos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkEtalonEncPos




% --- Executes on button press in tglExit.
function tglExit_Callback(hObject, eventdata, handles)
% hObject    handle to tglExit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of tglExit
stop(handles.ActTimer);
delete(handles.ActTimer);
close(handles.figDataGUI);



% --- Executes on button press in chkADC1.
function chkADC1_Callback(hObject, eventdata, handles)
% hObject    handle to chkADC1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkADC1



% --- Executes on button press in chkADC2.
function chkADC2_Callback(hObject, eventdata, handles)
% hObject    handle to chkADC2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkADC2


% --- Executes on button press in chkADC3.
function chkADC3_Callback(hObject, eventdata, handles)
% hObject    handle to chkADC3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkADC3




% --- Executes on button press in chkADC4.
function chkADC4_Callback(hObject, eventdata, handles)
% hObject    handle to chkADC4 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkADC4


% --- Executes on button press in chkADC5.
function chkADC5_Callback(hObject, eventdata, handles)
% hObject    handle to chkADC5 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkADC5


% --- Executes on button press in chkADC6.
function chkADC6_Callback(hObject, eventdata, handles)
% hObject    handle to chkADC6 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkADC6


% --- Executes on button press in chkADC7.
function chkADC7_Callback(hObject, eventdata, handles)
% hObject    handle to chkADC7 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkADC7


% --- Executes on button press in chkADC8.
function chkADC8_Callback(hObject, eventdata, handles)
% hObject    handle to chkADC8 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkADC8


% --- Executes on button press in chkPMT.
function chkPMT_Callback(hObject, eventdata, handles)
% hObject    handle to chkPMT (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkPMT


% --- Executes on button press in chkMCP1.
function chkMCP1_Callback(hObject, eventdata, handles)
% hObject    handle to chkMCP1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkMCP1

% --- Executes on button press in chkMCP2.
function chkMCP2_Callback(hObject, eventdata, handles)
% hObject    handle to chkMCP2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkMCP2




% --- Executes on button press in chkEtalonCurPos.
function chkEtalonCurPos_Callback(hObject, eventdata, handles)
% hObject    handle to chkEtalonCurPos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkEtalonCurPos


% --- Executes on button press in chkEtalonSetPos.
function chkEtalonSetPos_Callback(hObject, eventdata, handles)
% hObject    handle to chkEtalonSetPos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkEtalonSetPos




% --- Executes on button press in chkEtalonIndPos.
function chkEtalonIndPos_Callback(hObject, eventdata, handles)
% hObject    handle to chkEtalonIndPos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkEtalonIndPos




% --- Executes on slider movement.
function sldTimeWidth_Callback(hObject, eventdata, handles)
% hObject    handle to sldTimeWidth (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider


% --- Executes during object creation, after setting all properties.
function sldTimeWidth_CreateFcn(hObject, eventdata, handles)
% hObject    handle to sldTimeWidth (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background, change
%       'usewhitebg' to 0 to use default.  See ISPC and COMPUTER.
usewhitebg = 1;
if usewhitebg
    set(hObject,'BackgroundColor',[.9 .9 .9]);
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end




% --- Executes on slider movement.
function sldTimeStartPos_Callback(hObject, eventdata, handles)
% hObject    handle to sldTimeStartPos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider


% --- Executes during object creation, after setting all properties.
function sldTimeStartPos_CreateFcn(hObject, eventdata, handles)
% hObject    handle to sldTimeStartPos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background, change
%       'usewhitebg' to 0 to use default.  See ISPC and COMPUTER.
usewhitebg = 1;
if usewhitebg
    set(hObject,'BackgroundColor',[.9 .9 .9]);
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end




% --- Executes on button press in radRightEnd.
function radRightEnd_Callback(hObject, eventdata, handles)
% hObject    handle to radRightEnd (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radRightEnd


% --- Executes on button press in radLeftEnd.
function radLeftEnd_Callback(hObject, eventdata, handles)
% hObject    handle to radLeftEnd (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radLeftEnd


