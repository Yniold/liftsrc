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

% Last Modified by GUIDE v2.5 10-Jan-2005 18:21:42

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

% get horus handle
if length(varargin)==2 & varargin{1}=='handle'
    handles.parenthandle=str2double(varargin{2});
end

%setup Timer function
handles.ActTimer = timer('ExecutionMode','fixedDelay',...
      'Period',0.7,...    
      'BusyMode','drop',...
      'TimerFcn', {@PlotRefresh,handles});   

data.ActTimer=handles.ActTimer;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes ADC wait for user response (see UIRESUME)
% uiwait(handles.figDataGUI);
setappdata(handles.output, 'ADCdata', data);


function PlotRefresh(arg1,arg2,GUI_handles)
data = getappdata(GUI_handles.output, 'ADCdata');

%[s,w] = system('tail -n-10 data/ccStatus.txt > data/status_sub.txt');
%clear status_sub;
%load 'data/status_sub.txt';

%figure(GUI_handles.figDataGUI);
%set(GUI_handles.figDataGUI,'Visible','on');

%statusData=ReadDataAvg('status.bin',50,2500);
horusdata = getappdata(GUI_handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
AvgData=horusdata.AvgData;
%[statusData,AvgData]=ReadDataAvg('/lift/ramdisk/status.bin',50,500);
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

ADCBase1=655;
ADCBase2=688;
TempBase=728;
TempDetAxis=single(statusData(:,TempBase+8))./100-273.15;  
P20=single(statusData(:,ADCBase2+1+1*3))*0.00891-89.55;
P1000=single(statusData(:,ADCBase2+1));%*0.4464-4380.9;
DiodeUV=4.6863E-6*double(statusData(iZeit,ADCBase1+1+2*3)).^2-8.5857E-2*double(statusData(iZeit,ADCBase1+1+2*3))+390.41;
DiodeWZ=(double(statusData(iZeit,ADCBase2+1+5*3))-9790.0)/193.2836;
set(GUI_handles.txtPDyeL,'String',statusData(lastrow,ADCBase1+1));
set(GUI_handles.txtPVent,'String',statusData(lastrow,ADCBase1+1+6*3));
set(GUI_handles.txtPRef,'String',statusData(lastrow,ADCBase1+1+3*3));
set(GUI_handles.txtDiodeUV,'String',statusData(lastrow,ADCBase1+1+2*3));
set(GUI_handles.txtDiodeEta,'String',statusData(lastrow,ADCBase1+1+4*3));
set(GUI_handles.txtDiodeGR,'String',statusData(lastrow,ADCBase1+1+5*3));


set(GUI_handles.txtP1000Det,'String',statusData(lastrow,ADCBase2+1));
set(GUI_handles.txtP20Det,'String',statusData(lastrow,ADCBase2+1+1*3));
set(GUI_handles.txtPNO,'String',statusData(lastrow,ADCBase2+1+2*3));
set(GUI_handles.txtDiodeWht,'String',statusData(lastrow,ADCBase2+1+5*3));
set(GUI_handles.txtVHV,'String',statusData(lastrow,ADCBase2+1+3*3));
set(GUI_handles.txtTDet,'String',TempDetAxis(lastrow));
set(GUI_handles.txtIFila,'String',statusData(lastrow,ADCBase2+1+6*3));
set(GUI_handles.txtOPHIR,'String',statusData(lastrow,ADCBase2+1+7*3));

if P20(lastrow)<3 | P20(lastrow)>6
    set(GUI_handles.txtP20Det,'BackgroundColor','r');
end
if DiodeWZ(lastrow)<2
    set(GUI_handles.txtDiodeWht,'BackgroundColor','r');
end

EtalonBase=643; 
help=int32(statusData(:,EtalonBase)); %+65536.*statusData(:,EtalonBase+1));
EtalonSetPos=(help)+int32(statusData(:,EtalonBase+1)); EtalonSetPos(help>32767)=EtalonSetPos(help>32767)-65535;

help=int32(statusData(:,EtalonBase+2)); %+65536.*statusData(:,EtalonBase+1));
EtalonCurPos=(help)+int32(statusData(:,EtalonBase+3)); EtalonCurPos(help>32767)=EtalonCurPos(help>32767)-65535;

help=int32(statusData(:,EtalonBase+4)); %+65536.*statusData(:,EtalonBase+1));
EtalonEncPos=(help)+int32(statusData(:,EtalonBase+5)); EtalonEncPos(help>32767)=EtalonEncPos(help>32767)-65535;

help=int32(statusData(:,EtalonBase+6)); %+65536.*statusData(:,EtalonBase+1));
EtalonIndPos=(help)+int32(statusData(:,EtalonBase+7)); EtalonIndPos(help>32767)=EtalonIndPos(help>32767)-65535;

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

if get(GUI_handles.chkPDyeL,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ADCBase1+1),'r');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkPVent,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ADCBase1+1+6*3),'g');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkPRef,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ADCBase1+1+3*3),'b');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkDiodeUV,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),DiodeUV(iZeit),'r');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkDiodeEta,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ADCBase1+1+4*3),'g');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkDiodeGR,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ADCBase1+1+5*3),'b');
    hold(GUI_handles.axeADC,'on');
end 

if get(GUI_handles.chkP1000Det,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),P1000(iZeit),'r');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkP20Det,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),P20(iZeit),'g');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkPNO,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ADCBase2+1+2*3),'b');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkDiodeWht,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),DiodeWZ(iZeit),'b');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkVHV,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ADCBase2+1+3*3),'g');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkTDet,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),TempDetAxis(iZeit),'b');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkIFila,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ADCBase2+1+6*3),'r');
    hold(GUI_handles.axeADC,'on');
end 
if get(GUI_handles.chkOPHIR,'Value')
    plot(GUI_handles.axeADC,Zeit(iZeit),statusData(iZeit,ADCBase2+1+7*3),'r');
    hold(GUI_handles.axeADC,'on');
end 

timeStep=double(10.0/86400.0);
if (maxTime-minTime>0.7/1400.0)
    timeStep=1.0/1440.0;
end
timeXTick=[double(minTime):double(timeStep):double(maxTime)];
xlim(GUI_handles.axeADC,[minTime maxTime]);
%datetick(GUI_handles.axeADC,'x',13,'keeplimits');
%set(GUI_handles.axeADC,'XTick',timeXTick);
grid(GUI_handles.axeADC);
%xlim(GUI_handles.axeADC,[minTime maxTime]);
%plot(GUI_handles.axeEtalon,EtalonEncPos(iZeit),EtalonCurPos(iZeit),'.');
plot(GUI_handles.axeEtalon,statusData(iZeit,ADCBase2+1+7*3),statusData(iZeit,ADCBase1+1+5*3),'.');


PMTBase=19;
MCP1Base=228;
MCP2Base=437;
AVGBase=1;
PMTMaskBase=211;
MCP1MaskBase=420;
MCP2MaskBase=629;

% PMT: Hat sich Maske ge?ndert ? Dann neue Maske einlesen.
if ~isfield(data,'PMTMask')| ...
        ~isequal(statusData(lastrow,PMTMaskBase:PMTMaskBase+9),statusData(lastrow-5,PMTMaskBase:PMTMaskBase+9))
    data.PMTMask=ones(1,160);
    for a = 0:9,
        data.PMTMask((a*16+1):(a*16+16))=bitget(statusData(lastrow,PMTMaskBase+a),1:16);
    end
end
    
PMTSumCounts=statusData(:,PMTMaskBase+12);

% MCP1: Hat sich Maske ge?ndert ? Dann neue Maske einlesen.
if ~isfield(data,'MCP1Mask')| ...
        ~isequal(statusData(lastrow,MCP1MaskBase:MCP1MaskBase+9),statusData(lastrow-5,MCP1MaskBase:MCP1MaskBase+9))
    data.MCP1Mask=ones(1,160);
    for a=0:9
        data.MCP1Mask((a*16+1):(a*16+16))=bitget(statusData(lastrow,MCP1MaskBase+a),1:16);
    end
end

MCP1SumCounts=statusData(:,MCP1MaskBase+12);

% MCP2: Hat sich Maske ge?ndert ? Dann neue Maske einlesen.
if ~isfield(data,'MCP2Mask')| ...
        ~isequal(statusData(lastrow,MCP2MaskBase:MCP2MaskBase+9),statusData(lastrow-5,MCP2MaskBase:MCP2MaskBase+9))
    data.MCP2Mask=ones(1,160);
    for a=0:9
        data.MCP2Mask((a*16+1):(a*16+16))=bitget(statusData(lastrow,MCP2MaskBase+a),1:16);
    end
end

MCP2SumCounts=statusData(:,MCP2MaskBase+12);

set(GUI_handles.PMTCounts,'String',statusData(lastrow,PMTBase+204));
set(GUI_handles.MCP1Counts,'String',statusData(lastrow,MCP1Base+204));
set(GUI_handles.MCP2Counts,'String',statusData(lastrow,MCP2Base+204));

set(GUI_handles.PMTPulses,'String',statusData(lastrow,PMTBase+205));
set(GUI_handles.MCP1Pulses,'String',statusData(lastrow,MCP1Base+205));
set(GUI_handles.MCP2Pulses,'String',statusData(lastrow,MCP2Base+205));

%warning('OFF');
OnlineFilter=AvgData(:,AVGBase)>0;
OfflineLeftFilter=AvgData(:,AVGBase+1)>0;
OfflineRightFilter=AvgData(:,AVGBase+2)>0;

UVDiodeOnline(OnlineFilter)=single(statusData(OnlineFilter,ADCBase1+1+2*3))/1000.0;
UVDiodeOfflineLeft(OfflineLeftFilter)=single(statusData(OfflineLeftFilter,ADCBase1+1+2*3))/1000.0;
UVDiodeOfflineRight(OfflineRightFilter)=single(statusData(OfflineRightFilter,ADCBase1+1+2*3))/1000.0;
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


set(GUI_handles.txtPMTOffline,'String',PMTOfflineAvg(lastrow));
set(GUI_handles.txtMCP1Offline,'String',MCP1OfflineAvg(lastrow));
set(GUI_handles.txtMCP2Offline,'String',MCP2OfflineAvg(lastrow));

set(GUI_handles.txtPMTOnline,'String',PMTOnlineAvg(lastrow));
set(GUI_handles.txtMCP1Online,'String',MCP1OnlineAvg(lastrow));
set(GUI_handles.txtMCP2Online,'String',MCP2OnlineAvg(lastrow));


etaOnlinePos=int16(statusData(:,803));
etaCurPos=int16(statusData(:,645));
online=PMTSumCounts>2500 & abs(etaCurPos-etaOnlinePos)<20;
offline1=PMTSumCounts<=2500 & (etaCurPos-etaOnlinePos)==-600;
offline2=PMTSumCounts<=2500 & (etaCurPos-etaOnlinePos)==600;   

hold(GUI_handles.axeRay,'off');
hold(GUI_handles.axeFluo,'off');
hold(GUI_handles.axeCounts,'off');
hold(GUI_handles.axeCountsEtalon,'off');

if get(GUI_handles.chkPMT,'Value')
    %hold(GUI_handles.axeRay,'off');     
    plot(GUI_handles.axeRay,statusData(lastrow,PMTBase+1:PMTBase+160),'r'); 
    hold(GUI_handles.axeRay,'on'); 
    z=find(data.PMTMask==0);
    PMTdata1=double(statusData(lastrow,PMTBase+1:PMTBase+160));
    PMTdata1(z)=NaN;
    plot(GUI_handles.axeRay,PMTdata1); 
    xlim(GUI_handles.axeRay,[1,160]); 
    
    xaxis=[40:160];
    plot(GUI_handles.axeFluo,xaxis,statusData(lastrow,PMTBase+40:PMTBase+160),'r');
    hold(GUI_handles.axeFluo,'on');
    y=data.PMTMask(40:160);
    z=find(y==0);
    PMTdata2=double(statusData(lastrow,PMTBase+40:PMTBase+160));
    PMTdata2(z)=NaN;
    plot(GUI_handles.axeFluo,xaxis,PMTdata2); 
    xlim(GUI_handles.axeFluo,[40,160]); 
    
    WhichPlot=get(GUI_handles.popPMTPlot,'Value');
    switch WhichPlot
        case 1
           plot(GUI_handles.axeCounts,Zeit(iZeit),PMTSumCounts(iZeit)); %statusData(iZeit,PMTBase+204));
           hold(GUI_handles.axeCounts,'on');
        case 2           
           plot(GUI_handles.axeCounts,Zeit(30:end),PMTAvg(30:end),'b');   
           hold(GUI_handles.axeCounts,'on');
    end
    %plot(GUI_handles.axeCounts,Zeit(iZeit),statusData(iZeit,PMTBase+204),'r');
    plot(GUI_handles.axeCountsEtalon,EtalonCurPos(iZeit),PMTSumCounts(iZeit),'.');
    hold(GUI_handles.axeCounts,'on');
    hold(GUI_handles.axeCountsEtalon,'on');

end

if get(GUI_handles.chkMCP1,'Value')
    %hold(GUI_handles.axeRay,'off');     
    plot(GUI_handles.axeRay,statusData(lastrow,MCP1Base+1:MCP1Base+160),'r'); 
    hold(GUI_handles.axeRay,'on'); 
    z=find(data.MCP1Mask==0);
    MCP1data1=double(statusData(lastrow,MCP1Base+1:MCP1Base+160));
    MCP1data1(z)=NaN;
    plot(GUI_handles.axeRay,MCP1data1); 
    xlim(GUI_handles.axeRay,[1,160]); 
    
    %hold(GUI_handles.axeFluo,'off');     
    xaxis=[40:160];
    plot(GUI_handles.axeFluo,xaxis, statusData(lastrow,MCP1Base+40:MCP1Base+160),'r');
    hold(GUI_handles.axeFluo,'on');
    y=data.MCP1Mask(40:160);
    z=find(y==0);
    MCP1data2=double(statusData(lastrow,MCP1Base+40:MCP1Base+160));
    MCP1data2(z)=NaN;
    plot(GUI_handles.axeFluo,xaxis,MCP1data2); 
    xlim(GUI_handles.axeFluo,[40,160]); 

    WhichPlot=get(GUI_handles.popMCP1Plot,'Value');
    switch WhichPlot
        case 1
           plot(GUI_handles.axeCounts,Zeit(iZeit),MCP1SumCounts(iZeit)); %statusData(iZeit,MCP1Base+204));
           hold(GUI_handles.axeCounts,'on');
        case 2           
           plot(GUI_handles.axeCounts,Zeit(30:end),MCP1Avg(30:end),'b');   
           hold(GUI_handles.axeCounts,'on');
    end

    %plot(GUI_handles.axeCounts,Zeit(iZeit),MCP1SumCounts(iZeit),'g');         %statusData(iZeit,MCP1Base+204),'g');
    plot(GUI_handles.axeCountsEtalon,EtalonCurPos(iZeit),MCP1SumCounts(iZeit),'g.'); %statusData(:,MCP2Base+204),'g.');
    hold(GUI_handles.axeCounts,'on');
    hold(GUI_handles.axeCountsEtalon,'on');
end

if get(GUI_handles.chkMCP2,'Value')
    %hold(GUI_handles.axeRay,'off');     
    plot(GUI_handles.axeRay,statusData(lastrow,MCP2Base+1:MCP2Base+160),'r'); 
    hold(GUI_handles.axeRay,'on'); 
    z=find(data.MCP2Mask==0);
    MCP2data1=double(statusData(lastrow,MCP2Base+1:MCP2Base+160));
    MCP2data1(z)=NaN;
    plot(GUI_handles.axeRay,MCP2data1); 
    xlim(GUI_handles.axeRay,[1,160]); 
    
    %hold(GUI_handles.axeFluo,'off');     
    xaxis=[40:160];
    plot(GUI_handles.axeFluo,xaxis,statusData(lastrow,MCP2Base+40:MCP2Base+160),'r');
    hold(GUI_handles.axeFluo,'on');
    y=data.MCP2Mask(40:160);
    z=find(y==0);
    MCP2data2=double(statusData(lastrow,MCP2Base+40:MCP2Base+160));
    MCP2data2(z)=NaN;
    plot(GUI_handles.axeFluo,xaxis,MCP2data2); 
    xlim(GUI_handles.axeFluo,[40,160]); 

    WhichPlot=get(GUI_handles.popMCP2Plot,'Value');
    switch WhichPlot
        case 1
           plot(GUI_handles.axeCounts,Zeit(iZeit),MCP2SumCounts(iZeit)); %statusData(iZeit,MCP2Base+204));
           hold(GUI_handles.axeCounts,'on');
        case 2           
           %plot(GUI_handles.axeCounts,Zeit(30:end),int16(MCP2OnlineAvg(30:end)),'b'); %-int16(MCP2OfflineAvg(30:end)),'b'); 
           plot(GUI_handles.axeCounts,Zeit(30:end),MCP2Avg(30:end),'b');   
           hold(GUI_handles.axeCounts,'on');
           %plot(GUI_handles.axeCounts,Zeit(30:end),MCP2OfflineAvg(30:end),'b:');   
    end

    %plot(GUI_handles.axeCounts,Zeit(iZeit),MCP2SumCounts(iZeit),'g');         %statusData(iZeit,MCP2Base+204),'g');
    plot(GUI_handles.axeCountsEtalon,EtalonCurPos(iZeit),MCP2SumCounts(iZeit),'g.'); %statusData(:,MCP2Base+204),'g.');
    hold(GUI_handles.axeCounts,'on');
    hold(GUI_handles.axeCountsEtalon,'on');
end

     %ylim(GUI_handles.axeRay,[0 50]);
     %xlim(GUI_handles.axeRay,[110 140]);

xlim(GUI_handles.axeCounts,[minTime maxTime]);
%datetick(GUI_handles.axeCounts,'x',13,'keeplimits');
%set(GUI_handles.axeCounts,'XTick',timeXTick);
grid(GUI_handles.axeCounts);
setappdata(GUI_handles.output, 'ADCdata', data);




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




% --- Executes on button press in chkPDyeL.
function chkPDyeL_Callback(hObject, eventdata, handles)
% hObject    handle to chkPDyeL (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkPDyeL
if get(hObject,'Value')
    set(hObject,'Backgroundcolor',[1 0 0]);
else
    set(hObject,'Backgroundcolor',[0.831 0.816 0.784]);    
end

% --- Executes on button press in checkbox30.
function checkbox30_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox30 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox30


% --- Executes on button press in checkbox31.
function checkbox31_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox31 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox31


% --- Executes on button press in checkbox32.
function checkbox32_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox32 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox32


% --- Executes on button press in chkDiodeEta.
function chkDiodeEta_Callback(hObject, eventdata, handles)
% hObject    handle to chkDiodeEta (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkDiodeEta
if get(hObject,'Value')
    set(hObject,'Backgroundcolor',[1 0 0]);
else
    set(hObject,'Backgroundcolor',[0.831 0.816 0.784]);    
end


% --- Executes on button press in chkDiodeGR.
function chkDiodeGR_Callback(hObject, eventdata, handles)
% hObject    handle to chkDiodeGR (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkDiodeGR
if get(hObject,'Value')
    set(hObject,'Backgroundcolor',[1 0 0]);
else
    set(hObject,'Backgroundcolor',[0.831 0.816 0.784]);    
end


% --- Executes on button press in checkbox35.
function checkbox35_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox35 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox35


% --- Executes on button press in checkbox36.
function checkbox36_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox36 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox36


% --- Executes on button press in checkbox37.
function checkbox37_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox37 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox37


% --- Executes on button press in checkbox38.
function checkbox38_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox38 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox38


% --- Executes on button press in checkbox39.
function checkbox39_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox39 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox39


% --- Executes on button press in chkDiodeWht.
function chkDiodeWht_Callback(hObject, eventdata, handles)
% hObject    handle to chkDiodeWht (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkDiodeWht
if get(hObject,'Value')
    set(hObject,'Backgroundcolor',[1 0 0]);
else
    set(hObject,'Backgroundcolor',[0.831 0.816 0.784]);    
end


% --- Executes on button press in checkbox41.
function checkbox41_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox41 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox41


% --- Executes on button press in chkTDet.
function chkTDet_Callback(hObject, eventdata, handles)
% hObject    handle to chkTDet (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkTDet
if get(hObject,'Value')
    set(hObject,'Backgroundcolor',[1 0 0]);
else
    set(hObject,'Backgroundcolor',[0.831 0.816 0.784]);    
end


% --- Executes on button press in checkbox43.
function checkbox43_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox43 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox43


% --- Executes on button press in checkbox44.
function checkbox44_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox44 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox44




% --- Executes on button press in chkPVent.
function chkPVent_Callback(hObject, eventdata, handles)
% hObject    handle to chkPVent (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if get(hObject,'Value')
    set(hObject,'Backgroundcolor',[1 0 0]);
else
    set(hObject,'Backgroundcolor',[0.831 0.816 0.784]);    
end


% --- Executes on button press in chkPRef.
function chkPRef_Callback(hObject, eventdata, handles)
% hObject    handle to chkPRef (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if get(hObject,'Value')
    set(hObject,'Backgroundcolor',[1 0 0]);
else
    set(hObject,'Backgroundcolor',[0.831 0.816 0.784]);    
end


% --- Executes on button press in chkDiodeUV.
function chkDiodeUV_Callback(hObject, eventdata, handles)
% hObject    handle to chkDiodeUV (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if get(hObject,'Value')
    set(hObject,'Backgroundcolor',[1 0 0]);
else
    set(hObject,'Backgroundcolor',[0.831 0.816 0.784]);    
end




% --- Executes on button press in chkP1000Det.
function chkP1000Det_Callback(hObject, eventdata, handles)
% hObject    handle to chkP1000Det (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkP1000Det


% --- Executes on button press in chkP20Det.
function chkP20Det_Callback(hObject, eventdata, handles)
% hObject    handle to chkP20Det (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkP20Det


% --- Executes on button press in chkPNO.
function chkPNO_Callback(hObject, eventdata, handles)
% hObject    handle to chkPNO (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkPNO


% --- Executes on button press in chkVHV.
function chkVHV_Callback(hObject, eventdata, handles)
% hObject    handle to chkVHV (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkVHV


% --- Executes on button press in chkIFila.
function chkIFila_Callback(hObject, eventdata, handles)
% hObject    handle to chkIFila (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkIFila




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


% --- Executes on selection change in popupmenu3.
function popupmenu3_Callback(hObject, eventdata, handles)
% hObject    handle to popupmenu3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns popupmenu3 contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popupmenu3


% --- Executes during object creation, after setting all properties.
function popupmenu3_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmenu3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end




% --- Executes on button press in chkOPHIR.
function chkOPHIR_Callback(hObject, eventdata, handles)
% hObject    handle to chkOPHIR (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkOPHIR
