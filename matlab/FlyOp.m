
function varargout = FlyOp(varargin)
% DETECTION M-file for FlyOp.fig
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
%      applied to the GUI before FlyOp_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to FlyOp_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Copyright 2002-2003 The MathWorks, Inc.

% Edit the above text to modify the response to help FlyOp

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @FlyOp_OpeningFcn, ...
                   'gui_OutputFcn',  @FlyOp_OutputFcn, ...
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


% --- Executes just before FlyOp is made visible.
function FlyOp_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to FlyOp (see VARARGIN)

% Choose default command line output for FlyOp
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

% open communication with picomotors
handles.serport=serial('/dev/ttyS0','BaudRate',19200,'Terminator','CR');
set(handles.serport,'BytesAvailableFcn',{'serialdatacallback'});
try fopen(handles.serport);
catch 
    delete(handles.serport);
    rmfield(handles,'serport');
end;

data.toggleDyelaser=handles.toggleDyelaser;
data.toggleVacuum=handles.toggleVacuumDye;
data.toggleN2=handles.toggleN2Dye;

% Update handles structure
guidata(hObject, handles);
setappdata(handles.output, 'Detdata', data);
start(handles.Timer);


function DetRefresh(arg1,arg2,handles)

data = getappdata(handles.output, 'Detdata');

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
PlotWidth=maxLen;
stopPlot=maxLen;
startPlot=1;
iZeit=indexZeit(startPlot:stopPlot);
minTime=statustime(iZeit(1));
maxTime=statustime(iZeit(size(iZeit,1)));
xlim1=str2double(get(handles.editxlim1,'String'));
xlim2=str2double(get(handles.editxlim2,'String'));
limTime1=maxTime-xlim1./86400.0;
limTime2=maxTime-xlim2./86400.0;
if limTime2==limTime1
    limTime2=limTime1+1/86400.0;
end

% display system time
set(handles.txtTimer,'String',strcat(datestr(statustime(lastrow),13),'.',num2str(statusData(lastrow,6)/100)));

% calculate parameters from ADC counts
x=double(statusData(:,col.DiodeUV)); eval(['DiodeUV=',fcts2val.DiodeUV,';']);
x=double(statusData(:,col.TDet)); eval(['TDet=',fcts2val.TDet,';']);
x=double(statusData(:,col.P20)); eval(['P20=',fcts2val.P20,';']);
%x=double(statusData(:,col.P1000)); eval(['P1000=',fcts2val.P1000,';']);
x=double(statusData(:,col.DiodeWZ1out)); eval(['DiodeWZ1out=',fcts2val.DiodeWZ1out,';']);
x=double(statusData(:,col.DiodeWZ2out)); eval(['DiodeWZ2out=',fcts2val.DiodeWZ2out,';']);
x=double(statusData(:,col.DiodeWZ1in)); eval(['DiodeWZ1in=',fcts2val.DiodeWZ1in,';']);
x=double(statusData(:,col.DiodeWZ2in)); eval(['DiodeWZ2in=',fcts2val.DiodeWZ2in,';']);
x=double(statusData(:,col.PNO)); eval(['PNO=',fcts2val.PNO,';']);
x=double(statusData(:,col.MFCFlow)); eval(['MFCFlow=',fcts2val.MFCFlow,';']);
x=double(statusData(:,col.DiodeGr)); eval(['DiodeGr=',fcts2val.DiodeGr,';']);
x=double(statusData(:,col.PVent)); eval(['PVent=',fcts2val.PVent,';']);
x=double(statusData(:,col.PRef)); eval(['PRef=',fcts2val.PRef,';']);
x=double(statusData(:,col.PDyelaser)); eval(['PDyelaser=',fcts2val.PDyelaser,';']);
if ~isnan(col.TempDyelaser)
    x=double(statusData(:,col.TempDyelaser)); eval(['TDyelaser=',fcts2val.TempDyelaser,';']);
else
    TDyelaser=statustime; TDyelaser(:)=NaN;
end
if ~isnan(col.TempLaserPlate)
    x=double(statusData(:,col.TempLaserPlate)); eval(['TLaserpl=',fcts2val.TempLaserPlate,';']);
else
    TLaserpl=statustime; TLaserpl(:)=NaN;
end
if ~isnan(col.TempPrallpl)
    x=double(statusData(:,col.TempPrallpl)); eval(['TempPrallpl=',fcts2val.TempPrallpl,';']);
else
    TempPrallpl=statustime; TempPrallpl(:)=NaN;
end
%x=double(statusData(:,col.TLamp)); eval(['TLamp=',fcts2val.TLamp,';']);
%x=double(statusData(:,col.TPhoto2)); eval(['TPhoto2=',fcts2val.TPhoto2,';']);

set(handles.txtDiodeUV,'String',[num2str(DiodeUV(lastrow),3),' mW']);
set(handles.txtWZ1in,'String',[num2str(DiodeWZ1in(lastrow),3),' mW']);
set(handles.txtWZ1out,'String',[num2str(DiodeWZ1out(lastrow),3),' mW']);
set(handles.txtWZ2in,'String',[num2str(DiodeWZ2in(lastrow),3),' mW']);
set(handles.txtWZ2out,'String',[num2str(DiodeWZ2out(lastrow),3),' mW']);
set(handles.txtP1000,'String',statusData(lastrow,col.P1000));
set(handles.txtP20,'String',[num2str(P20(lastrow),3),' mbar']);
set(handles.txtPNO,'String',[num2str(PNO(lastrow),4),' mbar']);
set(handles.txtVHV,'String',statusData(lastrow,col.VHV));
set(handles.txtTDet,'String',[num2str(TDet(lastrow),3),' C']);
set(handles.txtTPrall,'String',[num2str(TempPrallpl(lastrow),3),' C']);
%set(handles.txtTLamp,'String',[num2str(TLamp(lastrow),3),' C']);
%set(handles.txtTPhoto2,'String',[num2str(TPhoto2(lastrow),3),' C']);
set(handles.txtPabs,'String',statusData(lastrow,col.PitotAbs));
set(handles.txtPdiff,'String',statusData(lastrow,col.PitotDiff));
set(handles.txtLamp1,'String',statusData(lastrow,col.PhototubeLamp1));
set(handles.txtLamp2,'String',statusData(lastrow,col.PhototubeLamp2));
set(handles.txtMFC,'String',[num2str(MFCFlow(lastrow),3),' sccm']);
set(handles.txtDiodeGr,'String',[num2str(DiodeGr(lastrow),2),' W']);
set(handles.txtPDyelaser,'String',[num2str(PDyelaser(lastrow),4),' mbar']);
set(handles.txtPVent,'String',[num2str(PVent(lastrow),4),' mbar']);
set(handles.txtIFilament,'String',statusData(lastrow,col.IFilament));
set(handles.txtPRef,'String',[num2str(PRef(lastrow),4),' mbar']);
set(handles.txtTDyelaser,'String',[num2str(TDyelaser(lastrow),3),' C']);
set(handles.txtLaserpl,'String',[num2str(TLaserpl(lastrow),3),' C']);

% warn for ADC signals out of allowed range for measurements
if P20(lastrow)<1 | P20(lastrow)>5
    set(handles.txtP20,'BackgroundColor','r');
else 
    set(handles.txtP20,'BackgroundColor',[0.7 0.7 0.7]);
end
if DiodeWZ1in(lastrow)<3
    set(handles.txtWZ1in,'BackgroundColor','r');
else 
    set(handles.txtWZ1in,'BackgroundColor',[0.7 0.7 0.7]);
end
if DiodeWZ1out(lastrow)<0.75*DiodeWZ1in
    set(handles.txtWZ1out,'BackgroundColor','r');
else 
    set(handles.txtWZ1out,'BackgroundColor',[0.7 0.7 0.7]);
end
if DiodeWZ2in(lastrow)<0.4
    set(handles.txtWZ2in,'BackgroundColor','r');
else 
    set(handles.txtWZ2in,'BackgroundColor',[0.7 0.7 0.7]);
end
if DiodeWZ2out(lastrow)<0.6*DiodeWZ2in
    set(handles.txtWZ2out,'BackgroundColor','r');
else 
    set(handles.txtWZ2out,'BackgroundColor',[0.7 0.7 0.7]);
end
if MFCFlow(lastrow)<5.5 | MFCFlow(lastrow)>6.5
    set(handles.txtMFC,'BackgroundColor','r');
else 
    set(handles.txtMFC,'BackgroundColor',[0.7 0.7 0.7]);
end
if statusData(lastrow,col.VHV)<12400
    set(handles.txtVHV,'BackgroundColor','r');
else 
    set(handles.txtVHV,'BackgroundColor',[0.7 0.7 0.7]);
end
if statusData(lastrow,col.PhototubeLamp1)>10010;
    set(handles.txtLamp1,'BackgroundColor','r');
else 
    set(handles.txtLamp1,'BackgroundColor',[0.7 0.7 0.7]);
end
if statusData(lastrow,col.PhototubeLamp2)>10010;
    set(handles.txtLamp2,'BackgroundColor','r');
else 
    set(handles.txtLamp2,'BackgroundColor',[0.7 0.7 0.7]);
end
if statusData(lastrow,col.PRef)>10500
    set(handles.txtPRef,'BackgroundColor','r');
else
    set(handles.txtPRef,'BackgroundColor',[0.7,0.7,0.7]);
end

if TDyelaser(lastrow)>51 | TDyelaser(lastrow)<49
    set(handles.txtTDyelaser,'BackgroundColor','r');
else
    set(handles.txtTDyelaser,'BackgroundColor',[0.7,0.7,0.7]);
end
if TLaserpl(lastrow)>38 | TLaserpl(lastrow)<42
    set(handles.txtTDyelaser,'BackgroundColor','r');
else
    set(handles.txtTDyelaser,'BackgroundColor',[0.7,0.7,0.7]);
end

if statusData(lastrow,col.IFilament)<10100
    set(handles.txtIFilament,'BackgroundColor','r');
else
    set(handles.txtIFilament,'BackgroundColor',[0.7,0.7,0.7]);
end

Etalonhelp=int32(statusData(:,col.etaSetPosLow));
EtalonSetPos=(Etalonhelp)+int32(statusData(:,col.etaSetPosHigh));
EtalonSetPos(Etalonhelp>32767)=EtalonSetPos(Etalonhelp>32767)-65535;

Etalonhelp=int32(statusData(:,col.etaCurPosLow));
EtalonCurPos=(Etalonhelp)+int32(statusData(:,col.etaCurPosHigh)); 
EtalonCurPos(Etalonhelp>32767)=EtalonCurPos(Etalonhelp>32767)-65535;

%Etalonhelp=int32(statusData(:,col.etaEncoderPosLow)); 
%EtalonEncPos=(Etalonhelp)+int32(statusData(:,col.etaEncoderPosHigh)); 
%EtalonEncPos(Etalonhelp>32767)=EtalonEncPos(Etalonhelp>32767)-65535;

Etalonhelp=int32(statusData(:,col.etaOnlinePosLow)); 
OnlinePos=(Etalonhelp)+int32(statusData(:,col.etaOnlinePosHigh)); 
OnlinePos(Etalonhelp>32767)=OnlinePos(Etalonhelp>32767)-65535;

EtalonSpeed=statusData(:,col.etaSetSpd);
EtalonStatus=statusData(:,col.etaStatus);

set(handles.txtEtCurPos,'String',EtalonCurPos(lastrow));
set(handles.txtEtSetPos,'String',EtalonSetPos(lastrow));
set(handles.txtonline,'String',OnlinePos(lastrow));

if bitget(EtalonStatus(lastrow),9)
    set(handles.txtLimitSwitch,'String','left','BackgroundColor','r');
elseif bitget(EtalonStatus(lastrow),10)
    set(handles.txtLimitSwitch,'String','right','BackgroundColor','r');
else
    set(handles.txtLimitSwitch,'String','none','BackgroundColor','c');
end


% plot checked parameters vs. time
hold(handles.axes1,'off'); 
if get(handles.chkDiodeUV,'Value')
    plot(handles.axes1,statustime(iZeit),DiodeUV(iZeit),'b');
    hold(handles.axes1,'on');
end 
if get(handles.chkWZ1in,'Value')
    plot(handles.axes1,statustime(iZeit),DiodeWZ1in(iZeit),'b');
    hold(handles.axes1,'on');
end 
if get(handles.chkWZ1out,'Value')
    plot(handles.axes1,statustime(iZeit),DiodeWZ1out(iZeit),'b');
    hold(handles.axes1,'on');
end 
if get(handles.chkWZ2in,'Value')
    plot(handles.axes1,statustime(iZeit),DiodeWZ2in(iZeit),'g');
    hold(handles.axes1,'on');
end 
if get(handles.chkWZ2out,'Value')
    plot(handles.axes1,statustime(iZeit),DiodeWZ2out(iZeit),'g');
    hold(handles.axes1,'on');
end 
if get(handles.chkP1000,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.P1000),'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkP20,'Value')
    plot(handles.axes1,statustime(iZeit),P20(iZeit),'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkPNO,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.PNO),'g');
    hold(handles.axes1,'on');
end 
if get(handles.chkVHV,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.VHV),'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkTDet,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.TDet),'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkMFC,'Value')
    plot(handles.axes1,statustime(iZeit),MFCFlow(iZeit),'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkPabs,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.PitotAbs),'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkPdiff,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.PitotDiff),'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkTPrall,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.TempPrallpl),'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkTLamp,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.TempPenray),'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkTPhoto2,'Value')
%    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.TPhoto2),'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkLamp1,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.PhototubeLamp1),'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkLamp2,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.PhototubeLamp2),'r');
    hold(handles.axes1,'on');
end 
if get(handles.checkDiodeGr,'Value')
    plot(handles.axes1,statustime(iZeit),DiodeGr,'r');
    hold(handles.axes1,'on');
end 
if get(handles.checkPDyelaser,'Value')
    plot(handles.axes1,statustime(iZeit),PDyelaser,'r');
    hold(handles.axes1,'on');
end 
if get(handles.checkPVent,'Value')
    plot(handles.axes1,statustime(iZeit),PVent,'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkEtCurPos,'Value')
    plot(handles.axes1,statustime(iZeit),EtalonCurPos(iZeit));
    hold(handles.axes1,'on');
end 
if get(handles.chkEtSetPos,'Value')
    plot(handles.axes1,statustime(iZeit),EtalonSetPos(iZeit));
    hold(handles.axes1,'on');
end 
if get(handles.chkTDyelaser,'Value')
    plot(handles.axes1,statustime(iZeit),TDyelaser(iZeit));
    hold(handles.axes1,'on');
end 
if get(handles.checkTLaserpl,'Value')
    plot(handles.axes1,statustime(iZeit),TLaserpl(iZeit));
    hold(handles.axes1,'on');
end 
xlim(handles.axes1,[limTime1 limTime2]);
grid(handles.axes1);

%plot PMT and MCP signals

PMTBase=col.ccData0;
MCP1Base=col.ccData1;
MCP2Base=col.ccData2;
PMTMaskBase=col.ccMask0;
MCP1MaskBase=col.ccMask1;
MCP2MaskBase=col.ccMask2;

% PMT: Has the counter mask changed ? Then read in new mask.
if ~isfield(data,'PMTMask')| ...
        ~isequal(statusData(lastrow,PMTMaskBase:PMTMaskBase+9),statusData(lastrow-5,PMTMaskBase:PMTMaskBase+9))
    data.PMTMask=ones(1,160);
    for a = 0:9,
        data.PMTMask((a*16+1):(a*16+16))=bitget(statusData(lastrow,PMTMaskBase+a),1:16);
    end
end
    
PMTSumCounts=statusData(:,col.ccCounts0);

% MCP1: Has the counter mask changed ? Then read in new mask.
if ~isfield(data,'MCP1Mask')| ...
        ~isequal(statusData(lastrow,MCP1MaskBase:MCP1MaskBase+9),statusData(lastrow-5,MCP1MaskBase:MCP1MaskBase+9))
    data.MCP1Mask=ones(1,160);
    for a=0:9
        data.MCP1Mask((a*16+1):(a*16+16))=bitget(statusData(lastrow,MCP1MaskBase+a),1:16);
    end
end

MCP1SumCounts=statusData(:,col.ccCounts1);

% MCP2: Has the counter mask changed ? Then read in new mask.
if ~isfield(data,'MCP2Mask')| ...
        ~isequal(statusData(lastrow,MCP2MaskBase:MCP2MaskBase+9),statusData(lastrow-5,MCP2MaskBase:MCP2MaskBase+9))
    data.MCP2Mask=ones(1,160);
    for a=0:9
        data.MCP2Mask((a*16+1):(a*16+16))=bitget(statusData(lastrow,MCP2MaskBase+a),1:16);
    end
end

MCP2SumCounts=statusData(:,col.ccCounts2);

% display counts and pulses
set(handles.txtPMTCounts,'String',statusData(lastrow,col.ccCounts0));
set(handles.txtMCP1Counts,'String',statusData(lastrow,col.ccCounts1));
set(handles.txtMCP2Counts,'String',statusData(lastrow,col.ccCounts2));

set(handles.txtPMTPulses,'String',statusData(lastrow,col.ccPulses0));
set(handles.txtMCP1Pulses,'String',statusData(lastrow,col.ccPulses1));
set(handles.txtMCP2Pulses,'String',statusData(lastrow,col.ccPulses2));

% calculate running averages for online and both offlines
PMTOnlineAvg=AvgData(:,1);  
PMTOfflineLeftAvg=AvgData(:,2);
PMTOfflineRightAvg=AvgData(:,3);

PMTOfflineAvg(1:size(statusData,1))=NaN;
PMTOfflineAvg=PMTOfflineAvg';
PMTOfflineAvg(statusData(:,col.RAvgOnOffFlag)==1)=PMTOfflineRightAvg(statusData(:,col.RAvgOnOffFlag)==1);
PMTOfflineAvg(statusData(:,col.RAvgOnOffFlag)==2)=PMTOfflineLeftAvg(statusData(:,col.RAvgOnOffFlag)==2);

PMTAvg(statusData(:,col.RAvgOnOffFlag)==3)=PMTOnlineAvg(statusData(:,col.RAvgOnOffFlag)==3);
PMTAvg(statusData(:,col.RAvgOnOffFlag)==2)=PMTOfflineLeftAvg(statusData(:,col.RAvgOnOffFlag)==2);
PMTAvg(statusData(:,col.RAvgOnOffFlag)==1)=PMTOfflineRightAvg(statusData(:,col.RAvgOnOffFlag)==1);
PMTAvg(statusData(:,col.RAvgOnOffFlag)==0)=NaN;

MCP1OnlineAvg=AvgData(:,4);
MCP1OfflineLeftAvg=AvgData(:,5);
MCP1OfflineRightAvg=AvgData(:,6);

MCP1OfflineAvg(1:size(statusData,1))=NaN;
MCP1OfflineAvg=MCP1OfflineAvg';
MCP1OfflineAvg(statusData(:,col.RAvgOnOffFlag)==1)=MCP1OfflineRightAvg(statusData(:,col.RAvgOnOffFlag)==1);
MCP1OfflineAvg(statusData(:,col.RAvgOnOffFlag)==2)=MCP1OfflineLeftAvg(statusData(:,col.RAvgOnOffFlag)==2);

MCP1Avg(statusData(:,col.RAvgOnOffFlag)==3)=MCP1OnlineAvg(statusData(:,col.RAvgOnOffFlag)==3);
MCP1Avg(statusData(:,col.RAvgOnOffFlag)==2)=MCP1OfflineLeftAvg(statusData(:,col.RAvgOnOffFlag)==2);
MCP1Avg(statusData(:,col.RAvgOnOffFlag)==1)=MCP1OfflineRightAvg(statusData(:,col.RAvgOnOffFlag)==1);
MCP1Avg(statusData(:,col.RAvgOnOffFlag)==0)=NaN;

MCP2OnlineAvg=AvgData(:,7);
MCP2OfflineLeftAvg=AvgData(:,8);
MCP2OfflineRightAvg=AvgData(:,9);

MCP2OfflineAvg(1:size(statusData,1))=NaN;
MCP2OfflineAvg=MCP2OfflineAvg';
MCP2OfflineAvg(statusData(:,col.RAvgOnOffFlag)==1)=MCP2OfflineRightAvg(statusData(:,col.RAvgOnOffFlag)==1);
MCP2OfflineAvg(statusData(:,col.RAvgOnOffFlag)==2)=MCP2OfflineLeftAvg(statusData(:,col.RAvgOnOffFlag)==2);

MCP2Avg(statusData(:,col.RAvgOnOffFlag)==3)=MCP2OnlineAvg(statusData(:,col.RAvgOnOffFlag)==3);
MCP2Avg(statusData(:,col.RAvgOnOffFlag)==2)=MCP2OfflineLeftAvg(statusData(:,col.RAvgOnOffFlag)==2);
MCP2Avg(statusData(:,col.RAvgOnOffFlag)==1)=MCP2OfflineRightAvg(statusData(:,col.RAvgOnOffFlag)==1);
MCP2Avg(statusData(:,col.RAvgOnOffFlag)==0)=NaN;

% display offline and online averages
set(handles.txtPMTOffline,'String',PMTOfflineAvg(lastrow));
set(handles.txtMCP1Offline,'String',MCP1OfflineAvg(lastrow));
set(handles.txtMCP2Offline,'String',MCP2OfflineAvg(lastrow));

set(handles.txtPMTOnline,'String',PMTOnlineAvg(lastrow));
set(handles.txtMCP1Online,'String',MCP1OnlineAvg(lastrow));
set(handles.txtMCP2Online,'String',MCP2OnlineAvg(lastrow));

% warn if PMTOnline is too low for valid online Signal
if PMTOnlineAvg(lastrow)<450
    set(handles.txtPMTOnline,'BackgroundColor','r');
else
    set(handles.txtPMTOnline,'BackgroundColor',[0.7 0.7 0.7]);
end

%calculate OH and HO2 mixing ratios
if statusData(lastrow,col.ValidSlaveDataFlag)
    radlife=1.45e6;			% Radiative lifetime (Hz) from D. Heard data
    % THESE PARAMETERS NEED TO BE CONSIDERED WHEN RUNNING THE INSTRUMENT IN A
    % DIFFERENT SETUP
    gate1=136e-9;			   % Approximate gate setting for rising edge (sec)
    gate2=596e-9;		   	% ...for falling edge (sec)
    Tcal=299;			      % Cell Temperature during lab calibration (K)
    Pcal=3.7;
    PowCal=10;		%OHUVPower during lab calibration (mW)
    PowCalb=1.75;		%HO2UVPower during lab calibration (mW)
    wmrcal=8E-3;	         % Calibration reference water concentration

    k_qcal=getq(Tcal,wmrcal);
    GAMMAcal= k_qcal*Pcal + radlife;  
    densCal=(6.022E+23/22400)*273/Tcal*Pcal/1013;

    bc=boltzcorr(Tcal,TDet(lastrow)+273);
    k_q=getq(TDet(lastrow)+273,str2double(get(handles.editH2O,'String')));
    GAMMA = k_q*P20(lastrow) + radlife;

    quen = (1/GAMMA).*((exp(-gate1*GAMMA)-exp(-gate2*GAMMA)));
    quencal = (1/GAMMAcal).*((exp(-gate1*GAMMAcal)-exp(-gate2*GAMMAcal)));

    Dens=6.023E23/22400*273./(TDet(lastrow)+273)*P20(lastrow)/1013; %Converting to density

    COH=quen.*bc.*(str2double(get(handles.editC,'String'))/quencal/densCal)*Dens;
    COH=COH.*(DiodeWZ1in(lastrow)+DiodeWZ1out(lastrow))/2;

    CHO2b=quen.*bc.*(str2double(get(handles.editC,'String'))/quencal/densCal)*Dens;
    CHO2b=CHO2b.*(DiodeWZ2in(lastrow)+DiodeWZ2out(lastrow))/2;
    if rank(COH)~=0
        XOH = (MCP1OnlineAvg-MCP1OfflineAvg).*5./COH';
    else
        XOH = MCP1OnlineAvg; XOH(:)=NaN;
    end
    if rank(CHO2b)~=0
        XHOx = (MCP2OnlineAvg-MCP2OfflineAvg).*5./CHO2b';
    else
        XHOx = MCP1OnlineAvg; XHOx(:)=NaN;
    end
else
    XOH = MCP1OnlineAvg; XOH(:)=NaN;
    XHOx = MCP1OnlineAvg; XHOx(:)=NaN;
end    

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
           xlim1=str2double(get(handles.editxlim1,'String'));
           xlim2=str2double(get(handles.editxlim2,'String'));
           limTime1=maxTime-xlim1./86400.0;
           limTime2=maxTime-xlim2./86400.0;
           xlim(handles.axes1,[limTime1 limTime2]);

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
        case 3           
           plot(handles.axeCounts,statustime(30:end),XOH(30:end),'b');   
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
        case 3           
           plot(handles.axeCounts,statustime(30:end),XHOx(30:end),'b');   
           hold(handles.axeCounts,'on');
    end
    hold(handles.axeCounts,'on');
end

xlim(handles.axeCounts,[limTime1 limTime2]);
grid(handles.axeCounts);

% check HV
if bitget(statusData(lastrow,col.Valve2armAxis),8)==0
    %    set(handles.togHV,'Value',0)
    set(handles.togHV,'BackgroundColor','c','String','HV OFF');
else
    if bitget(statusData(lastrow,col.ccGateDelay1),16)==0 ...
            | bitget(statusData(lastrow,col.ccGateDelay2),16)==0
        set(handles.togHV,'BackgroundColor','y','String','HV ON');
    else
    %    set(handles.togHV,'Value',1)
        set(handles.togHV,'BackgroundColor','g','String','HV ON');
    end
end

% check Blower
if bitget(statusData(lastrow,col.Valve2armAxis),9)==0 | ...
        bitget(statusData(lastrow,col.Valve2armAxis),1)==0
    if bitget(statusData(lastrow,col.Valve2armAxis),10)
        set(handles.togBlower,'BackgroundColor','r','String','Pump ON');
    else
    %    set(handles.togBlower,'Value',0)
        set(handles.togBlower,'BackgroundColor','c','String','Blower OFF');
    end
else
%    set(handles.togBlower,'Value',1)
    set(handles.togBlower,'BackgroundColor','g','String','Blower ON');
end



% check Lamp
if bitget(statusData(lastrow,col.Valve2armAxis),11)
    set(handles.tglLamp,'BackgroundColor','g','String','Lamp ON');
else
    set(handles.tglLamp,'BackgroundColor','c','String','Lamp OFF');
end

% check Pitot Zeroing Valve
if bitget(statusData(lastrow,col.Valve1armAxis),12)
    set(handles.tglPitot,'BackgroundColor','g','String','Pitot 0 ON');
else
    set(handles.tglPitot,'BackgroundColor','c','String','Pitot 0 OFF');
end

% check Heaters
% Heater Lamp
if bitget(statusData(lastrow,col.Valve2armAxis),3)
    set(handles.tglHeatLamp,'BackgroundColor','g');
else
    set(handles.tglHeatLamp,'BackgroundColor','c');
end
% Heater Prallplatte
if bitget(statusData(lastrow,col.Valve2armAxis),4)
    set(handles.tglHeatPrall,'BackgroundColor','g');
else
    set(handles.tglHeatPrall,'BackgroundColor','c');
end
% Heater Phototube 2
if bitget(statusData(lastrow,col.Valve2armAxis),5)
    set(handles.tglHeatPhoto2,'BackgroundColor','g');
else
    set(handles.tglHeatPhoto2,'BackgroundColor','c');
end

% check solenoids
if bitget(statusData(lastrow,col.Valve1armAxis),4)==0
    set(handles.toggleC3F6,'BackgroundColor','c');
else 
    set(handles.toggleC3F6,'BackgroundColor','g');
end
if bitget(statusData(lastrow,col.Valve1armAxis),3)==0
    set(handles.toggleN2,'BackgroundColor','c');
else 
    set(handles.toggleN2,'BackgroundColor','g');
end
if bitget(statusData(lastrow,col.Valve1armAxis),2)==0
    set(handles.toggleHO2Inj,'BackgroundColor','c');
else 
    set(handles.toggleHO2Inj,'BackgroundColor','g');
end
if bitget(statusData(lastrow,col.Valve1armAxis),1)==0
    set(handles.toggleOHInj,'BackgroundColor','c');
else 
    set(handles.toggleOHInj,'BackgroundColor','g');
end
if bitget(statusData(lastrow,col.Valve1armAxis),7)==0
    set(handles.toggleNO1,'BackgroundColor','c');
else 
    set(handles.toggleNO1,'BackgroundColor','g');
end
if bitget(statusData(lastrow,col.Valve1armAxis),6)==0
    set(handles.toggleNO2,'BackgroundColor','c');
else 
    set(handles.toggleNO2,'BackgroundColor','g');
end
if bitget(statusData(lastrow,col.Valve1armAxis),5)==0
    set(handles.toggleNOPurge,'BackgroundColor','c');
else 
    set(handles.toggleNOPurge,'BackgroundColor','g');
end
if bitget(statusData(lastrow,col.Valve1armAxis),14)==0
    set(handles.tglN2O,'BackgroundColor','c');
else 
    set(handles.tglN2O,'BackgroundColor','g');
end
if bitget(statusData(lastrow,col.Valve1armAxis),13)==0
    set(handles.tglVac,'BackgroundColor','c');
else 
    set(handles.tglVac,'BackgroundColor','g');
end
if bitget(statusData(lastrow,col.Valve2armAxis),12)==0
    set(handles.tglKuv,'BackgroundColor','c');
else 
    set(handles.tglKuv,'BackgroundColor','g');
end

% check filament status (e.g. if it was swiched off by horus)
if bitget(statusData(lastrow,col.ValveLift),14)==0;
    set(handles.toggleFilament,'Value',0,'string','Filament is OFF');
    set(handles.toggleFilament,'BackgroundColor','c');
else 
    set(handles.toggleFilament,'Value',1,'string','Filament is ON');
    set(handles.toggleFilament,'BackgroundColor','g');
end

% check shutter status
if bitget(statusData(lastrow,col.ValveLift),13)==0;
    set(handles.toggleShutter,'Value',0,'string','Shutter is OPEN');
    set(handles.toggleShutter,'BackgroundColor','g');
else 
    set(handles.toggleShutter,'Value',1,'string','Shutter is CLOSED');
    set(handles.toggleShutter,'BackgroundColor','c');
end

% check Dyelaser solenoids
if bitget(statusData(lastrow,col.ValveLift),11)==0
    set(handles.toggleVacuumDye,'BackgroundColor','c');
else 
    set(handles.toggleVacuumDye,'BackgroundColor','g');
end
if bitget(statusData(lastrow,col.ValveLift),8)==0
    set(handles.toggleDyelaser,'BackgroundColor','c');
else 
    set(handles.toggleDyelaser,'BackgroundColor','g');
end
if bitget(statusData(lastrow,col.ValveLift),9)==0
    set(handles.toggleN2Dye,'BackgroundColor','c');
else 
    set(handles.toggleN2Dye,'BackgroundColor','g');
end
if bitget(statusData(lastrow,col.ValveLift),10)==0
    set(handles.toggleAmbientDye,'BackgroundColor','c');
else 
    set(handles.toggleAmbientDye,'BackgroundColor','g');
end

data.lastrow=lastrow;
data.OnlinePos=OnlinePos;
data.CurPos=EtalonCurPos;
setappdata(handles.output, 'Detdata', data);

% --- Outputs from this function are returned to the command line.
function varargout = FlyOp_OutputFcn(hObject, eventdata, handles) 
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
if isvalid(handles.serport)
    fclose(handles.serport);
    delete(handles.serport);
end;
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


% --- Executes on button press in chkWZ1out.
function chkWZ1out_Callback(hObject, eventdata, handles)
% hObject    handle to chkWZ1out (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkWZ1out


% --- Executes on button press in chkWZ1in.
function chkWZ1in_Callback(hObject, eventdata, handles)
% hObject    handle to chkWZ1in (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkWZ1in


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
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;
col=horusdata.col;

if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        if isequal(get(hObject,'BackgroundColor'),[0 1 1])
            set(hObject,'BackgroundColor','r','String','switching Blower ON');
            Valveword=bitset(statusData(lastrow,col.Valve2armAxis),10);  % switch pump on
            system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
            system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
            set(hObject,'BackgroundColor','r','String','Pump ON');
            pause(5);
            while str2double(statusData(lastrow,col.P1000))>11000 % switch on Blower only when cell pressure P1000 is low enough
                pause(1);
            end
            Valveword=bitset(statusData(lastrow,col.Valve2armAxis),10);  % make sure pump is not switched off
            Valveword=bitset(Valveword,9); % switch on blower
            Valveword=bitset(Valveword,1); % ramp blower up 
            system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
            system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
            set(hObject,'BackgroundColor','g','String','Blower ON');
        end
    else
        if isequal(get(hObject,'BackgroundColor'),[0 1 0]) | isequal(get(hObject,'BackgroundColor'),[1 0 0])
            set(hObject,'BackgroundColor','r','String','switching Blower OFF');
            Valveword=bitset(statusData(lastrow,col.Valve2armAxis),1,0); % ramp blower down
            system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
            system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
            set(hObject,'BackgroundColor','r','String','Pump ON');
            pause(5);
            Valveword=bitset(statusData(lastrow,col.Valve2armAxis),1,0); % make sure ramp down switch is set
            Valveword=bitset(Valveword,9,0); % switch off blower
            pause(10);
            Valveword=bitset(statusData(lastrow,col.Valve2armAxis),1,0); % make sure ramp down switch is set
            Valveword=bitset(Valveword,9,0); % make sure blower is off
            Valveword=bitset(Valveword,10,0); % switch off pump
            system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
            system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
            set(hObject,'BackgroundColor','c','String','Blower OFF');
        end
    end
end


% --- Executes on button press in togHV.
function togHV_Callback(hObject, eventdata, handles)
% switches HV and Gain

% hObject    handle to togHV (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togHV
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;
col=horusdata.col;

if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        if isequal(get(hObject,'BackgroundColor'),[0 1 1]) | isequal(get(hObject,'BackgroundColor'),[1 1 0])
            set(hObject,'BackgroundColor','g','String','HV ON');
            Valveword=bitset(statusData(lastrow,col.Valve2armAxis),8);  % switch HV on
            % switch gain on for MCP1
            word1=bitset(statusData(lastrow,col.ccGateDelay1),16);
            % switch gain on for MCP2
            word2=bitset(statusData(lastrow,col.ccGateDelay2),16);
            system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch HV
            system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
            system(['/lift/bin/eCmd @armAxis w 0xa318 ',num2str(word1)]);
            system(['/lift/bin/eCmd @armAxis w 0xa31c ',num2str(word2)]);
        end
    else
        if isequal(get(hObject,'BackgroundColor'),[0 1 0])  | isequal(get(hObject,'BackgroundColor'),[1 1 0])
            set(hObject,'BackgroundColor','c','String','HV OFF');
            Valveword=bitset(statusData(lastrow,col.Valve2armAxis),8,0);  % switch HV off
            % switch gain off for MCP1
            word1=bitset(statusData(lastrow,col.ccGateDelay1),16,0);
            % switch gain off for MCP2
            word2=bitset(statusData(lastrow,col.ccGateDelay2),16,0);
            system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch HV
            system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
            system(['/lift/bin/eCmd @armAxis w 0xa318 ',num2str(word1)]);
            system(['/lift/bin/eCmd @armAxis w 0xa31c ',num2str(word2)]);
        end
    end 
end




% --- Executes on button press in togglebutton5.
function tglLamp_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton5 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton5
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;
col=horusdata.col;
if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),11);
        set(hObject,'BackgroundColor','g','String','Lamp ON');
    else
        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),11,0);
        set(hObject,'BackgroundColor','c','String','Lamp OFF');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
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



% --- Executes on button press in toggleOHInj.
function toggleOHInj_Callback(hObject, eventdata, handles)
% hObject    handle to toggleOHInj (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleOHInj
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),1);
        set(hObject,'BackgroundColor','g');
    else
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),1,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to keep Pitot Zero open
end


% --- Executes on button press in toggleHO2Inj.
function toggleHO2Inj_Callback(hObject, eventdata, handles)
% hObject    handle to toggleHO2Inj (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleHO2Inj
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),2);
        set(hObject,'BackgroundColor','g');
    else
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),2,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to keep Pitot Zero open
end


% --- Executes on button press in toggleN2.
function toggleN2_Callback(hObject, eventdata, handles)
% hObject    handle to toggleN2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleN2
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),3);
        set(hObject,'BackgroundColor','g');
    else
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),3,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to keep Pitot Zero open
end


% --- Executes on button press in toggleC3F6.
function toggleC3F6_Callback(hObject, eventdata, handles)
% hObject    handle to toggleC3F6 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleC3F6
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),4);
        set(hObject,'BackgroundColor','g');
    else
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),4,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to keep Pitot Zero open
end


% --- Executes on button press in toggleNO1.
function toggleNO1_Callback(hObject, eventdata, handles)
% hObject    handle to toggleNO1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleNO1
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),7);
        set(hObject,'BackgroundColor','g');
    else
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),7,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to keep Pitot Zero open
end

% --- Executes on button press in toggleNO2.
function toggleNO2_Callback(hObject, eventdata, handles)
% hObject    handle to toggleNO2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleNO2
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),6);
        set(hObject,'BackgroundColor','g');
    else
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),6,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to keep Pitot Zero open
end


% --- Executes on button press in toggleNOPurge.
function toggleNOPurge_Callback(hObject, eventdata, handles)
% hObject    handle to toggleNOPurge (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleNOPurge
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),5);
        set(hObject,'BackgroundColor','g');
    else
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),5,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to keep Pitot Zero open
end



function editxlim1_Callback(hObject, eventdata, handles)
% hObject    handle to editxlimit1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editxlimit1 as text
%        str2double(get(hObject,'String')) returns contents of editxlimit1 as a double
xlim1=uint16(str2double(get(hObject,'String')));
if (xlim1>600) set(hObject,'String','600');
else set(hObject,'String',num2str(xlim1));
end



function editxlim2_Callback(hObject, eventdata, handles)
% hObject    handle to editxlim2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editxlim2 as text
%        str2double(get(hObject,'String')) returns contents of editxlim2 as a double
xlim2=uint16(str2double(get(hObject,'String')));
if xlim2>600 set(hObject,'String','600');
else set(hObject,'String',num2str(xlim2));
end


% --- Executes on button press in chkWZ2out.
function chkWZ2out_Callback(hObject, eventdata, handles)
% hObject    handle to chkWZ2out (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkWZ2out


% --- Executes on button press in chkWZ2in.
function chkWZ2in_Callback(hObject, eventdata, handles)
% hObject    handle to chkWZ2in (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkWZ2in



function editH2O_Callback(hObject, eventdata, handles)
% hObject    handle to editH2O (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editH2O as text
%        str2double(get(hObject,'String')) returns contents of editH2O as a double


function editC_Callback(hObject, eventdata, handles)
% hObject    handle to editC (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editC as text
%        str2double(get(hObject,'String')) returns contents of editC as a double




function editMFC_Callback(hObject, eventdata, handles)
% hObject    handle to editMFC (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editMFC as text
%        str2double(get(hObject,'String')) returns contents of editMFC as a double
MaxFlow=20;
setMFC=str2double(get(hObject,'String'));
if setMFC<0 | isnan(setMFC) setMFC=0; end
if setMFC>MaxFlow setMFC=MaxFlow; end
set(hObject,'String',num2str(setMFC));

Value=setMFC/MaxFlow*255;
system(['/lift/bin/eCmd @armAxis w 0xa404 0xFF']); % initialise
system(['/lift/bin/eCmd @armAxis w 0xa440 ', num2str(uint16(Value))]);



% --- Executes on button press in chkMFC.
function chkMFC_Callback(hObject, eventdata, handles)
% hObject    handle to chkMFC (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkMFC


% --- Executes on button press in chkDiodeUV.
function chkDiodeUV_Callback(hObject, eventdata, handles)
% hObject    handle to chkDiodeUV (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkDiodeUV




% --- Executes on button press in chkTPrall.
function chkTPrall_Callback(hObject, eventdata, handles)
% hObject    handle to chkTPrall (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkTPrall


% --- Executes on button press in chkPabs.
function chkPabs_Callback(hObject, eventdata, handles)
% hObject    handle to chkPabs (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkPabs


% --- Executes on button press in tglN2O.
function tglN2O_Callback(hObject, eventdata, handles)
% hObject    handle to tglN2O (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of tglN2O
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),14);
        set(hObject,'BackgroundColor','g');
    else
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),14,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to keep Pitot Zero open
end

% --- Executes on button press in tglVac.
function tglVac_Callback(hObject, eventdata, handles)
% hObject    handle to tglVac (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of tglVac
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),13);
        set(hObject,'BackgroundColor','g');
    else
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),13,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to keep Pitot Zero open
end


% --- Executes on button press in tglKuv.
function tglKuv_Callback(hObject, eventdata, handles)
% hObject    handle to tglKuv (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of tglKuv
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),12);
        set(hObject,'BackgroundColor','g');
    else
        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),12,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to other valves working
end


% --- Executes on button press in tglKuvN2.
%function tglKuvN2_Callback(hObject, eventdata, handles)
% hObject    handle to tglKuv (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of tglKuv
%if statusData(lastrow,col.ValidSlaveDataFlag)
%    if get(hObject,'Value')
%        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),13);
%        set(hObject,'BackgroundColor','g');
%    else
%        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),13,0);
%        set(hObject,'BackgroundColor','c');
%    end
%    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
%    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
%    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18 needed to keep other valves working%
%end


% --- Executes on button press in chkLamp1.
function chkLamp1_Callback(hObject, eventdata, handles)
% hObject    handle to chkLamp1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkLamp1


% --- Executes on button press in chkLamp2.
function chkLamp2_Callback(hObject, eventdata, handles)
% hObject    handle to chkLamp2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkLamp2




% --- Executes on button press in tglPitot.
function tglPitot_Callback(hObject, eventdata, handles)
% hObject    handle to tglPitot (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Hint: get(hObject,'Value') returns toggle state of tglPitot
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;
col=horusdata.col;
if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),12);
        set(hObject,'BackgroundColor','g','String','Pitot 0 ON');
    else
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),12,0);
        set(hObject,'BackgroundColor','c','String','Pitot 0 OFF');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch
    system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to hold solenoids
end

% --- Executes on button press in tglHeatLamp.
function tglHeatLamp_Callback(hObject, eventdata, handles)
% hObject    handle to tglHeatLamp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Hint: get(hObject,'Value') returns toggle state of tglHeatLamp
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;
col=horusdata.col;
if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),3);
        set(hObject,'BackgroundColor','g');
    else
        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),3,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
end

% --- Executes on button press in chkTLamp.
function chkTLamp_Callback(hObject, eventdata, handles)
% hObject    handle to chkTLamp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkTLamp


% --- Executes on button press in chkTPhoto2.
function chkTPhoto2_Callback(hObject, eventdata, handles)
% hObject    handle to chkTPhoto2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkTPhoto2


% --- Executes on button press in tglHeatPrall.
function tglHeatPrall_Callback(hObject, eventdata, handles)
% hObject    handle to tglHeatPrall (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of tglHeatPrall
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;
col=horusdata.col;
if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),4);
        set(hObject,'BackgroundColor','g');
    else
        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),4,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
end


% --- Executes on button press in tglHeatPhoto2.
function tglHeatPhoto2_Callback(hObject, eventdata, handles)
% hObject    handle to tglHeatPhoto2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Hint: get(hObject,'Value') returns toggle state of tglHeatPhoto2
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;
col=horusdata.col;
if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),5);
        set(hObject,'BackgroundColor','g');
    else
        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),5,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
end




% --- Executes on button press in checkDiodeGr.
function checkDiodeGr_Callback(hObject, eventdata, handles)
% hObject    handle to checkDiodeGr (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkDiodeGr


% --- Executes on button press in checkTLaserpl.
function checkTLaserpl_Callback(hObject, eventdata, handles)
% hObject    handle to checkTLaserpl (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkTLaserpl


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
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if get(hObject,'Value')
    Valveword=bitset(statusData(lastrow,col.ValveLift),8);
    set(hObject,'BackgroundColor','g','String','Valve Dyelaser ON');
else
    Valveword=bitset(statusData(lastrow,col.ValveLift),8,0);
    set(hObject,'BackgroundColor','c','String','Valve Dyelaser OFF');
end
system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
system(['/lift/bin/eCmd @Lift w 0xa408 ', num2str(Valveword)]);
system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open


% --- Executes on button press in toggleVacuumDye.
function toggleVacuumDye_Callback(hObject, eventdata, handles)
% hObject    handle to toggleVacuumDye (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleVacuumDye
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if get(hObject,'Value')
    Valveword=bitset(statusData(lastrow,col.ValveLift),11);
    set(hObject,'BackgroundColor','g','String','Valve Vacuum ON');
else
    Valveword=bitset(statusData(lastrow,col.ValveLift),11,0);
    set(hObject,'BackgroundColor','c','String','Valve Vacuum OFF');
end
system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
system(['/lift/bin/eCmd @Lift w 0xa408 ', num2str(Valveword)]);
system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open


% --- Executes on button press in toggleN2Dye.
function toggleN2Dye_Callback(hObject, eventdata, handles)
% hObject    handle to toggleN2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleN2
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if get(hObject,'Value')
    Valveword=bitset(statusData(lastrow,col.ValveLift),9);
    set(hObject,'BackgroundColor','g','String','Valve N2 ON');
else
    Valveword=bitset(statusData(lastrow,col.ValveLift),9,0);
    set(hObject,'BackgroundColor','c','String','Valve N2 OFF');
end
system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
system(['/lift/bin/eCmd @Lift w 0xa408 ', num2str(Valveword)]);
system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open


% --- Executes on button press in toggleAmbientDye.
function toggleAmbientDye_Callback(hObject, eventdata, handles)
% hObject    handle to toggleAmbientDye (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleAmbientDye
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if get(hObject,'Value')
    Valveword=bitset(statusData(lastrow,col.ValveLift),10);
    set(hObject,'BackgroundColor','g','String','Valve Ambient ON');
else
    Valveword=bitset(statusData(lastrow,col.ValveLift),10,0);
    set(hObject,'BackgroundColor','c','String','Valve Ambient OFF');
end
system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
system(['/lift/bin/eCmd @Lift w 0xa408 ', num2str(Valveword)]);
system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open


function set_pos_Callback(hObject, eventdata, handles)
% hObject    handle to set_pos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of set_pos as text
%        str2double(get(hObject,'String')) returns contents of set_pos as a double
onlinepos=uint16(str2double(get(hObject,'String')));
if isnan(onlinepos)
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','white');
    set(hObject,'string',num2str(onlinepos));
end


% --- Executes during object creation, after setting all properties.
function set_pos_CreateFcn(hObject, eventdata, handles)
% hObject    handle to set_pos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function scan_start_pos_Callback(hObject, eventdata, handles)
% hObject    handle to scan_start_pos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of scan_start_pos as text
%        str2double(get(hObject,'String')) returns contents of scan_start_pos as a double
scanstartpos=uint16(str2double(get(hObject,'String')));
if isnan(scanstartpos)
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','white');
    set(hObject,'string',num2str(scanstartpos));
end


% --- Executes during object creation, after setting all properties.
function scan_start_pos_CreateFcn(hObject, eventdata, handles)
% hObject    handle to scan_start_pos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function scan_stop_pos_Callback(hObject, eventdata, handles)
% hObject    handle to scan_stop_pos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of scan_stop_pos as text
%        str2double(get(hObject,'String')) returns contents of scan_stop_pos as a double
scanstoppos=uint16(str2double(get(hObject,'String')));
if isnan(scanstoppos)
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','white');
    set(hObject,'string',num2str(scanstoppos));
end


% --- Executes during object creation, after setting all properties.
function scan_stop_pos_CreateFcn(hObject, eventdata, handles)
% hObject    handle to scan_stop_pos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function scan_step_Callback(hObject, eventdata, handles)
% hObject    handle to scan_step (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of scan_step as text
%        str2double(get(hObject,'String')) returns contents of scan_step as a double
scanstep=uint8(str2double(get(hObject,'String')));
if isnan(scanstep)
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','white');
    scanstep=scanstep/4*4;
    if scanstep<=0, scanstep=4; end;
    set(hObject,'string',num2str(scanstep));
end


% --- Executes during object creation, after setting all properties.
function scan_step_CreateFcn(hObject, eventdata, handles)
% hObject    handle to scan_step (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in scan_pushbutton.
function scan_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to scan_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
scanstartpos=uint16(str2double(get(handles.scan_start_pos,'String')));
scanstoppos=uint16(str2double(get(handles.scan_stop_pos,'String')));
scanstep=uint8(str2double(get(handles.scan_step,'String')));
if (isnan(scanstartpos)| isnan(scanstoppos) | isnan(scanstep))
    error('invalid values');
else
    system(['/lift/bin/eCmd @Lift s etalonnop']);    
    system(['/lift/bin/eCmd @Lift s etalonscanstart ',num2str(scanstartpos)])
    system(['/lift/bin/eCmd @Lift s etalonscanstop ',num2str(scanstoppos)]);
    system(['/lift/bin/eCmd @Lift s etalonscanstep ',num2str(scanstep)]);
    system('/lift/bin/eCmd @Lift s etalonscan');
end


% --- Executes on button press in online_pushbutton.
function online_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to online_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');

% read in current online position from gui txt-field
curonlinepos=str2double(get(handles.txtonline,'String'));
% find last data point when etalon was in online position
i=find(data.CurPos==curonlinepos,1,'last');
% find maximum reference Signal and corresponding data point 
[calcOnlSign,icalcOnlSign]=max(statusData(:,col.ccCounts0));
% if the maximum reference signal is bigger than the last online reference signal
% set new online and go there
if isempty(i)
    system(['/lift/bin/eCmd @Lift s etalonnop']);    
    system(['/lift/bin/eCmd @Lift w 0xa510 ',num2str(data.CurPos(icalcOnlSign))]);
    system(['/lift/bin/eCmd @Lift s etalononline ',num2str(data.CurPos(icalcOnlSign))]);
    set(handles.txtonline,'String',num2str(data.CurPos(icalcOnlSign)));    
elseif calcOnlSign>statusData(i,col.ccCounts0)
    system(['/lift/bin/eCmd @Lift s etalonnop']);    
    system(['/lift/bin/eCmd @Lift w 0xa510 ',num2str(data.CurPos(icalcOnlSign))]);
    system(['/lift/bin/eCmd @Lift s etalononline ',num2str(data.CurPos(icalcOnlSign))]);
    set(handles.txtonline,'String',num2str(data.CurPos(icalcOnlSign)));
else % go to old online position
    system(['/lift/bin/eCmd @Lift s etalonnop']);    
    system(['/lift/bin/eCmd @Lift w 0xa510 ',num2str(curonlinepos)]);
end
system(['/lift/bin/eCmd @Lift s etalonditheronline']);


% --- Executes on button press in offline_pushbutton.
function offline_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to offline_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
onlinepos=uint16(str2double(get(handles.set_pos,'String')));
offlinepos=onlinepos+1000;
if isnan(onlinepos)
    error('invalid values');
else
    system(['/lift/bin/eCmd @Lift s etalonnop']);    
    system(['/lift/bin/eCmd @Lift w 0xa510 ',num2str(offlinepos)]);
end


% --- Executes on button press in toggle_pushbutton.
function toggle_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to toggle_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
onlinepos=uint16(str2double(get(handles.txtonline,'String')));
system(['/lift/bin/eCmd @Lift s etalonnop']);    
%system(['/lift/bin/eCmd @Lift s etalononline ',num2str(onlinepos)]);
%system(['/lift/bin/eCmd @Lift s etalonofflineleft 1000']);
%system(['/lift/bin/eCmd @Lift s etalonofflineright 1000']);
%system('/lift/bin/eCmd @Lift s etalondither 8');
system('/lift/bin/eCmd @Lift s etalontoggle');

% --- Executes on button press in home_pushbutton.
function home_pushbutton_Callback(hObject, eventdata, handles)
% hObject    handle to home_pushbutton (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
system(['/lift/bin/eCmd @Lift s etalonnop']);    
system('/lift/bin/eCmd @Lift w 0xa510 0');


% --- Executes on button press in toggleFilament.
function toggleFilament_Callback(hObject, eventdata, handles)
% hObject    handle to toggleFilament (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleFilament
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if get(hObject,'Value')
    if statusData(lastrow,col.PRef)<=10500 % check if pressure in reference cell is low enough
        Valveword=bitset(statusData(lastrow,col.ValveLift),14);
        system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(15*140))]);% 15V needed to switch filament relay on
        system(['/lift/bin/eCmd @Lift w 0xa408 ', num2str(Valveword)]);
        system('sleep 1');
        system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open
        set(hObject,'String','Filament is ON');
        set(hObject,'BackgroundColor','g');
    else 
        set(hObject,'Value',0);
    end
else
    Valveword=bitset(statusData(lastrow,col.ValveLift),14,0);
    system(['/lift/bin/eCmd @Lift w 0xa408 ', num2str(Valveword)]);
    set(hObject,'String','Filament is OFF');
    set(hObject,'BackgroundColor','c');
end


% --- Executes on button press in toggleShutter.
function toggleShutter_Callback(hObject, eventdata, handles)
% hObject    handle to toggleShutter (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleShutter
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;
if get(hObject,'Value')
    Valveword=bitset(statusData(lastrow,col.ValveLift),13);
    system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(20*140))]);% 20V needed to close shutter
    system(['/lift/bin/eCmd @Lift w 0xa408 ', num2str(Valveword)]);
    set(hObject,'String','Shutter is CLOSED');
    set(hObject,'BackgroundColor','c');
    system('sleep 1');
    system(['/lift/bin/eCmd @Lift w 0xa468 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open
else    
    Valveword=bitset(statusData(lastrow,col.ValveLift),13,0);
    system(['/lift/bin/eCmd @Lift w 0xa408 ', num2str(Valveword)]);
    set(hObject,'String','Shutter is OPEN');
    set(hObject,'BackgroundColor','g');
end


% --- Executes on button press in chkEtCurPos.
function chkEtCurPos_Callback(hObject, eventdata, handles)
% hObject    handle to chkEtCurPos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkEtCurPos


% --- Executes on button press in chkEtSetPos.
function chkEtSetPos_Callback(hObject, eventdata, handles)
% hObject    handle to chkEtSetPos (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkEtSetPos


% --- Executes on button press in chkTDyelaser.
function chkTDyelaser_Callback(hObject, eventdata, handles)
% hObject    handle to chkTDyelaser (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkTDyelaser



% --- Executes on selection change in popupmirror.
function popupmirror_Callback(hObject, eventdata, handles)
% hObject    handle to popupmirror (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns popupmirror contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popupmirror
set(handles.textPos,'String','0');


% --- Executes during object creation, after setting all properties.
function popupmirror_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmirror (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
set(hObject,'Value',2);


% --- Executes on button press in radiohor.
function radiohor_Callback(hObject, eventdata, handles)
% hObject    handle to radiohor (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiohor
if get(hObject,'Value')
    set(handles.radiover,'Value',0);
end
set(handles.textPos,'String','0');


% --- Executes on button press in radiover.
function radiover_Callback(hObject, eventdata, handles)
% hObject    handle to radiover (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if get(hObject,'Value')
    set(handles.radiohor,'Value',0);
end
set(handles.textPos,'String','0');


% --- Executes on button press in radiofor.
function radiofor_Callback(hObject, eventdata, handles)
% hObject    handle to radiofor (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if get(hObject,'Value')
    set(handles.radiorev,'Value',0);
end


% --- Executes on button press in radiorev.
function radiorev_Callback(hObject, eventdata, handles)
% hObject    handle to radiorev (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if get(hObject,'Value')
    set(handles.radiofor,'Value',0);
end



function editsteps_Callback(hObject, eventdata, handles)
% hObject    handle to editsteps (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editsteps as text
%        str2double(get(hObject,'String')) returns contents of editsteps as a double
steps=uint16(str2double(get(hObject,'String')));
if isnan(steps)
    set(hObject,'BackgroundColor','red');
else 
    set(hObject,'BackgroundColor','white');
    set(hObject,'string',num2str(steps));
end


% --- Executes during object creation, after setting all properties.
function editsteps_CreateFcn(hObject, eventdata, handles)
% hObject    handle to editsteps (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in pushgo.
function pushgo_Callback(hObject, eventdata, handles)
% hObject    handle to pushgo (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
set(handles.textPos,'BackgroundColor','r');
set(handles.pushgo,'BackgroundColor','r');

serport=handles.serport;
steps=get(handles.editsteps,'String');
hor=get(handles.radiohor,'Value');
forw=get(handles.radiofor,'Value');
oldpos=str2double(get(handles.textPos,'String'));

switch get(handles.popupmirror,'Value')
    case 1
        driver='a1';
        if hor==1 chl='0';
        else chl='1';
        end
    case 2
        if hor==1;
            driver='a1';
            chl='2';
        else 
            driver='a2';
            chl='0';
        end
    case 3 
        driver='a2';
        if hor==1 chl='1';
        else chl='2';
        end
end
fprintf(serport,['chl ',driver,'=',chl]);
if forw==1
    fprintf(serport,['rel ',driver,' ',steps]);
else
    fprintf(serport,['rel ',driver,' -',steps]);
end
fprintf(serport,['go ',driver]);

% check if motor is still moving
fprintf(serport,['pos ',driver]);
x=find(serport.UserData=='=');
pos2=str2double(serport.UserData(x+1:length(serport.UserData)-1));
%serport.UserData
pos1=pos2-1;
while pos2~=pos1
    pos1=pos2;
    pause(1);
    fprintf(serport,['pos ',driver]);
    x=find(serport.UserData=='=');
    pos2=str2double(serport.UserData(x+1:length(serport.UserData)-1));
end
% display new motor position after motor has stopped
newpos=oldpos+pos2;
set(handles.textPos,'String',num2str(newpos),'BackgroundColor','w');
set(handles.pushgo,'BackgroundColor','w');


% --- Executes on button press in pushStop.
function pushStop_Callback(hObject, eventdata, handles)
% hObject    handle to pushStop (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
fprintf(handles.serport,'hal');



% --- Executes on button press in pushgoto.
function pushgoto_Callback(hObject, eventdata, handles)
% hObject    handle to pushgoto (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
setpos=uint16(str2double(get(handles.set_pos,'String')));
if isnan(setpos)
    error('invalid values');
else
    system(['/lift/bin/eCmd @Lift s etalonnop']);    
    system(['/lift/bin/eCmd @Lift w 0xa510 ',num2str(setpos)]);
end




% --- Executes on button press in toggleShutter.
function t_Callback(hObject, eventdata, handles)
% hObject    handle to toggleShutter (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of toggleShutter


