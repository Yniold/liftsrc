function varargout = FlyDetection(varargin)
% DETECTION M-file for FlyDetection.fig
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
%      applied to the GUI before FlyDetection_OpeningFunction gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to FlyDetection_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Copyright 2002-2003 The MathWorks, Inc.

% Edit the above text to modify the response to help FlyDetection

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @FlyDetection_OpeningFcn, ...
                   'gui_OutputFcn',  @FlyDetection_OutputFcn, ...
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


% --- Executes just before FlyDetection is made visible.
function FlyDetection_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to FlyDetection (see VARARGIN)

% Choose default command line output for FlyDetection
handles.output = hObject;

% get horus handle
if length(varargin)==2 & varargin{1}=='handle'
    handles.parenthandle=str2double(varargin{2});
end

% choose Text size
set(double(get(handles.figure1,'Children')),'FontSize',8)

%setup Timer function
handles.Timer = timer('ExecutionMode','fixedDelay',...
      'Period',0.7,...    
      'BusyMode','drop',...
      'TimerFcn', {@FlyDetRefresh,handles});   

data.Timer=handles.Timer;

data.calstatus=0; %needed for in-flight calibration
data.PitotTime=0; %needed for in-flight calibration
data.tglPitot=handles.tglPitot;

% Update handles structure
guidata(hObject, handles);
setappdata(handles.output, 'Detdata', data);
start(handles.Timer);


function FlyDetRefresh(arg1,arg2,handles)

data = getappdata(handles.output, 'Detdata');

horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
AvgData=horusdata.AvgData;
col=horusdata.col;
fcts2val=horusdata.fcts2val;
statustime=horusdata.statustime;
maxLen=horusdata.maxLen;
lastrow=horusdata.lastrow;
indexZeit=horusdata.indexZeit;
horustxtBlower=horusdata.txtBlower;
if ~isequal(get(horustxtBlower,'BackgroundColor'),[0 1 1])
    tcpBlower=horusdata.tcpBlower;
end

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
disptime=statustime(lastrow)-double(statusData(lastrow,6))/86400000.0;
set(handles.txtTimer,'String',strcat(datestr(disptime,13),'.',num2str(statusData(lastrow,6)/100)));

if statusData(lastrow,col.ValidSlaveDataFlag) % only if Arm is on
    % start pitot zero after lamp was switched on or off
    if data.calstatus==1 & bitget(statusData(lastrow,col.Valve1armAxis),12)==0 % lamp just switched and pitot zero is not already on
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),12); %switch on pitot zero
        system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch
        system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
        system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to hold solenoids
        
        data.PitotTime=statustime;
        data.calstatus=0;
    end
    % stop pitot zero after 10 s
    if data.PitotTime~=0 %zeroing process active ?
        if (statustime-data.PitotTime)*86400>10 % for more than 10 s already ?
            Valveword=bitset(statusData(lastrow,col.Valve1armAxis),12,0); %switch off pitot zero
            system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch
            system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
            system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to hold solenoids
        
            data.PitotTime=0; 
        end
    end
end            
            
% calculate parameters from ADC counts
x=double(statusData(:,col.DiodeUV)); eval(['DiodeUV=',fcts2val.DiodeUV,';']);
x=double(statusData(:,col.TDet)); 
if x>10000
    eval(['TDet=',fcts2val.TDet,';']);
else
    TDet=statustime; TDet(:)=NaN;
end
%TDet=double(statusData(:,col.TDet));
x=double(statusData(:,col.P20)); eval(['P20=',fcts2val.P20,';']);
%x=double(statusData(:,col.P1000)); eval(['P1000=',fcts2val.P1000,';']);
x=double(statusData(:,col.DiodeWZ1out)); eval(['DiodeWZ1out=',fcts2val.DiodeWZ1out,';']);
x=double(statusData(:,col.DiodeWZ2out)); eval(['DiodeWZ2out=',fcts2val.DiodeWZ2out,';']);
x=double(statusData(:,col.DiodeWZ1in)); eval(['DiodeWZ1in=',fcts2val.DiodeWZ1in,';']);
x=double(statusData(:,col.DiodeWZ2in)); eval(['DiodeWZ2in=',fcts2val.DiodeWZ2in,';']);
x=double(statusData(:,col.PNO)); eval(['PNO=',fcts2val.PNO,';']);
x=double(statusData(:,col.MFCFlow)); eval(['MFCFlow=',fcts2val.MFCFlow,';']);
if ~isnan(col.TempDetFunnel)
    x=double(statusData(:,col.TempDetFunnel)); eval(['TempDetFunnel=',fcts2val.TempDetFunnel,';']);
else
    TempDetFunnel=statustime; TempDetFunnel(:)=NaN;
end
if ~isnan(col.TempPenray)
    x=double(statusData(:,col.TempPenray)); eval(['TempPenray=',fcts2val.TempPenray,';']);
else
    TempPenray=statustime; TempPenray(:)=NaN;
end
x=double(statusData(:,col.PitotAbs)); eval(['PitotAbs=',fcts2val.PitotAbs,';']);
x=double(statusData(:,col.PitotDiff)); eval(['PitotDiff=',fcts2val.PitotDiff,';']);
x=double(statusData(:,col.MFCC3F6Flow)); eval(['MFCC3F6Flow=',fcts2val.MFCC3F6Flow,';']);
x=double(statusData(:,col.MFCPropFlow)); eval(['MFCPropFlow=',fcts2val.MFCPropFlow,';']);
x=double(statusData(:,col.MFCIPISynAirFlow)); eval(['MFCIPISynAirFlow=',fcts2val.MFCIPISynAirFlow,';']);
x=double(statusData(:,col.MFCShowerFlow)); eval(['MFCShowerFlow=',fcts2val.MFCShowerFlow,';']);


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
set(handles.txtTDetFunnel,'String',[num2str(TempDetFunnel(lastrow),3),' C']);
set(handles.txtTLamp,'String',[num2str(TempPenray(lastrow),3),' C']);
set(handles.txtMFCC3F6,'String',[num2str(MFCC3F6Flow(lastrow),3),' sccm']);
set(handles.txtPabs,'String',[num2str(PitotAbs(lastrow),4),' mbar']);
set(handles.txtPdiff,'String',[num2str(PitotDiff(lastrow),3),' mbar']);
set(handles.txtMFCProp,'String',[num2str(MFCPropFlow(lastrow),3),' sccm']);
set(handles.txtMFCIPISynAir,'String',[num2str(MFCIPISynAirFlow(lastrow),4),' sccm']);
set(handles.txtMFCShower,'String',[num2str(MFCShowerFlow(lastrow),4),' sccm']);
set(handles.txtMFCNO,'String',[num2str(MFCFlow(lastrow),3),' sccm']);

% warn for ADC signals out of allowed range for measurements
if P20(lastrow)<1 | P20(lastrow)>10
    set(handles.txtP20,'BackgroundColor','r');
else 
    set(handles.txtP20,'BackgroundColor',[0.7 0.7 0.7]);
end
if DiodeWZ1in(lastrow)<2
    set(handles.txtWZ1in,'BackgroundColor','r');
else 
    set(handles.txtWZ1in,'BackgroundColor',[0.7 0.7 0.7]);
end
if DiodeWZ1out(lastrow)<0.6*DiodeWZ1in
    set(handles.txtWZ1out,'BackgroundColor','y');
else 
    set(handles.txtWZ1out,'BackgroundColor',[0.7 0.7 0.7]);
end
if DiodeWZ2in(lastrow)<0.4
    set(handles.txtWZ2in,'BackgroundColor','r');
else 
    set(handles.txtWZ2in,'BackgroundColor',[0.7 0.7 0.7]);
end
if DiodeWZ2out(lastrow)<0.6*DiodeWZ2in
    set(handles.txtWZ2out,'BackgroundColor','y');
else 
    set(handles.txtWZ2out,'BackgroundColor',[0.7 0.7 0.7]);
end
if MFCFlow(lastrow)<4 | MFCFlow(lastrow)>9
    set(handles.txtMFCNO,'BackgroundColor','r');
else 
    set(handles.txtMFCNO,'BackgroundColor',[0.7 0.7 0.7]);
end
if statusData(lastrow,col.VHV)<12400
    set(handles.txtVHV,'BackgroundColor','y');
else 
    set(handles.txtVHV,'BackgroundColor',[0.7 0.7 0.7]);
end
%if PCuvette(lastrow)>2
%    if (bitget(statusData(lastrow,col.Valve1armAxis),13)==1 | bitget(statusData(lastrow,col.Valve1armAxis),13)==1)
%        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),13,0);
%        Valveword=bitset(Valveword,14,0);
%        system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
%        system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
%        system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to keep Pitot Zero open
%    end
%end

%if statusData(lastrow,col.PhototubeLamp1)>10010;
%    set(handles.txtMFCProp,'BackgroundColor','r');
%else 
%    set(handles.txtMFCProp,'BackgroundColor',[0.7 0.7 0.7]);
%end
%if statusData(lastrow,col.PhototubeLamp2)>10010;
%    set(handles.txtMFCShower,'BackgroundColor','r');
%else 
%    set(handles.txtMFCShower,'BackgroundColor',[0.7 0.7 0.7]);
%end




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
if get(handles.chkMFCNO,'Value')
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
if get(handles.chkTDetFunnel,'Value')
    if ~isnan(col.TempDetFunnel)
        plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.TempDetFunnel),'r');
        hold(handles.axes1,'on');
    end
end 
if get(handles.chkTLamp,'Value')
    if ~isnan(col.TempPenray)
        plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.TempPenray),'r');
        hold(handles.axes1,'on');
    end
end 
if get(handles.chkMFCC3F6,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.MFCC3F6Flow),'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkMFCProp,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.MFCPropFlow),'r');
    hold(handles.axes1,'on');
end
if get(handles.chkMFCIPISynAir,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.MFCIPISynAirFlow),'r');
    hold(handles.axes1,'on');
end 
if get(handles.chkMFCShower,'Value')
    plot(handles.axes1,statustime(iZeit),statusData(iZeit,col.MFCShowerFlow),'r');
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
set(handles.txtPMTCounts,'String',num2str(statusData(lastrow,col.ccCounts0),3));
set(handles.txtMCP1Counts,'String',num2str(statusData(lastrow,col.ccCounts1),2));
set(handles.txtMCP2Counts,'String',num2str(statusData(lastrow,col.ccCounts2),2));

set(handles.txtPMTPulses,'String',num2str(statusData(lastrow,col.ccPulses0),4));
set(handles.txtMCP1Pulses,'String',num2str(statusData(lastrow,col.ccPulses1),4));
set(handles.txtMCP2Pulses,'String',num2str(statusData(lastrow,col.ccPulses2),4));

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
set(handles.txtPMTOffline,'String',num2str(PMTOfflineAvg(lastrow),3));
set(handles.txtMCP1Offline,'String',num2str(MCP1OfflineAvg(lastrow),2));
set(handles.txtMCP2Offline,'String',num2str(MCP2OfflineAvg(lastrow),2));
set(handles.txtPMTOnline,'String',num2str(PMTOnlineAvg(lastrow),4));
set(handles.txtMCP1Online,'String',num2str(MCP1OnlineAvg(lastrow),4));
set(handles.txtMCP2Online,'String',num2str(MCP2OnlineAvg(lastrow),4));

% warn if Offline Signals are zero, possible problem with MCPs
if MCP1OnlineAvg(lastrow)==0
    set(handles.txtMCP1Online,'BackgroundColor','r');
else
    set(handles.txtMCP1Online,'BackgroundColor',[0.7 0.7 0.7]);
end
if MCP2OnlineAvg(lastrow)==0
    set(handles.txtMCP2Online,'BackgroundColor','r');
else
    set(handles.txtMCP2Online,'BackgroundColor',[0.7 0.7 0.7]);
end

% warn if PMTOnline is too low for valid online Signal
if PMTOnlineAvg(lastrow)<2*PMTOfflineAvg
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
    if ~isnan(COH)
        if rank(COH)~=0
             XOH = (MCP1OnlineAvg-MCP1OfflineAvg).*5./COH';
        end
    else
        XOH = MCP1OnlineAvg; XOH(:)=NaN;
    end
    if ~isnan(COH)
        if rank(CHO2b)~=0
            XHOx = (MCP2OnlineAvg-MCP2OfflineAvg).*5./CHO2b';
        end
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
           plot(handles.axeCounts,statustime(iZeit),MCP2SumCounts(iZeit),'r'); %statusData(iZeit,MCP2Base+204));
           hold(handles.axeCounts,'on');
        case 2           
           plot(handles.axeCounts,statustime(30:end),MCP2Avg(30:end),'r');   
           hold(handles.axeCounts,'on');
        case 3           
           plot(handles.axeCounts,statustime(30:end),XHOx(30:end),'r');   
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

% check Pump, Bit 10 is Leybold, Bit 7 is Scroll Pump
if ~isequal(get(handles.togPump,'BackgroundColor'),[1 1 0])  % if Pump is not just being switched
    if ~isequal(get(horustxtBlower,'BackgroundColor'),[0 1 1]) % Blower connected via tcp (ground configuration)
        % no actual check is done, to recheck push button in horus has to be used
        BlowerStatus=get(horustxtBlower,'String');
        if (strcmp(BlowerStatus,'Pump ON') | strcmp(BlowerStatus,'Blower ON'))
            set(handles.togPump,'BackgroundColor','g','String','Pump ON');
        else 
            set(handles.togPump,'BackgroundColor','c','String','Pump OFF');
        end
    else % Blower connected directly to ARMaxis (air configuration)
        if (bitget(statusData(lastrow,col.Valve2armAxis),10) & bitget(statusData(lastrow,col.Valve2armAxis),7))
            set(handles.togPump,'BackgroundColor','g','String','Pump ON');
        else
            set(handles.togPump,'BackgroundColor','c','String','Pump OFF');
        end
    end
end

% check Blower
if ~isequal(get(handles.togBlower,'BackgroundColor'),[1 1 0])  % if Blower is not just being switched
    if ~isequal(get(horustxtBlower,'BackgroundColor'),[0 1 1]) % Blower connected via tcp (ground configuration)
        % no actual check is done, to recheck push button in horus has to be used
        BlowerStatus=get(horustxtBlower,'String');
        if strcmp(BlowerStatus,'Blower ON')
            set(handles.togBlower,'BackgroundColor','g','String','Blower ON');
        else
            set(handles.togBlower,'BackgroundColor','c','String','Blower OFF');
        end
    else % Blower connected directly to ARMaxis (air configuration)
        if ((bitget(statusData(lastrow,col.Valve2armAxis),9) & ...
            bitget(statusData(lastrow,col.Valve2armAxis),1)))
            set(handles.togBlower,'BackgroundColor','g','String','Blower ON');
        else
            set(handles.togBlower,'BackgroundColor','c','String','Blower OFF');
        end
    end
end

% check Butterfly
% Butterfly with relay
if bitget(statusData(lastrow,col.Valve2armAxis),2)==1
        set(handles.togButterfly,'BackgroundColor','c','String','Butterfly CLOSED');
else
        set(handles.togButterfly,'BackgroundColor','g','String','Butterfly OPEN');
end
% end Butterfly with relay

% Butterfly with stepper motor
%if statusData(lastrow,col.ButterflyPositionValid)==0
%    set(handles.togButterfly,'BackgroundColor','r','String','Butterfly INIT');
%else
%    if statusData(lastrow,col.ButterflyCurrentPosition)==42
%        set(handles.togButterfly,'BackgroundColor','c','String','Butterfly CLOSED');
%    elseif statusData(lastrow,col.ButterflyCurrentPosition)==(625+42)
%        set(handles.togButterfly,'BackgroundColor','g','String','Butterfly OPEN');
%    else
%        set(handles.togButterfly,'BackgroundColor','r','String','MOVING');
%    end
%end
% end Butterfly with stepper motor

% check Lamp
if bitget(statusData(lastrow,col.Valve2armAxis),11)
    set(handles.tglLamp,'BackgroundColor','r','String','Lamp ON');
else
    set(handles.tglLamp,'BackgroundColor','c','String','Lamp OFF');
end

% check Pitot Zeroing Valve
if bitget(statusData(lastrow,col.Valve1armAxis),12)
    set(handles.tglPitot,'BackgroundColor','y','String','Pitot 0 ON');
else
    set(handles.tglPitot,'BackgroundColor','c','String','Pitot 0 OFF');
end

% check Heaters
% Heater Lamp
if bitget(statusData(lastrow,col.Valve2armAxis),3)
    set(handles.tglHeatLamp,'BackgroundColor','g');
else
    set(handles.tglHeatLamp,'BackgroundColor','y');
end
% Heater Prallplatte
if bitget(statusData(lastrow,col.Valve2armAxis),4)
    set(handles.tglHeatPrall,'BackgroundColor','g');
else
    set(handles.tglHeatPrall,'BackgroundColor','y');
end
% Heater Phototube 2
if bitget(statusData(lastrow,col.Valve2armAxis),5)
    set(handles.tglHeatPhoto2,'BackgroundColor','g');
else
    set(handles.tglHeatPhoto2,'BackgroundColor','y');
end

% check solenoids
if bitget(statusData(lastrow,col.Valve1armAxis),4)==0
    set(handles.toggleC3F6,'BackgroundColor','c');
else 
    set(handles.toggleC3F6,'BackgroundColor','r');
end
if bitget(statusData(lastrow,col.Valve1armAxis),3)==0
    set(handles.toggleN2,'BackgroundColor','c');
else 
    set(handles.toggleN2,'BackgroundColor','r');
end
if bitget(statusData(lastrow,col.Valve1armAxis),2)==0
    set(handles.toggleHO2Inj,'BackgroundColor','c');
else 
    set(handles.toggleHO2Inj,'BackgroundColor','g');
end
if bitget(statusData(lastrow,col.Valve1armAxis),1)==0
    set(handles.toggleOHInj,'BackgroundColor','c');
else 
    set(handles.toggleOHInj,'BackgroundColor','r');
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
if bitget(statusData(lastrow,col.Valve1armAxis),14)==0   % C3F6 Valve
    set(handles.tglN2O,'BackgroundColor','c');
else 
    set(handles.tglN2O,'BackgroundColor','g');
end
if bitget(statusData(lastrow,col.Valve1armAxis),13)==0   % Propene
    set(handles.tglVac,'BackgroundColor','c');
else 
    set(handles.tglVac,'BackgroundColor','g');
end
%if bitget(statusData(lastrow,col.Valve2armAxis),12)==0
%    set(handles.tglKuv,'BackgroundColor','c');
%else 
%    set(handles.tglKuv,'BackgroundColor','g');
%end
if bitget(statusData(lastrow,col.Valve1armAxis),4)==0
    set(handles.tglVent,'BackgroundColor','c');
else 
    set(handles.tglVent,'BackgroundColor','r');
end

data.lastrow=lastrow;
setappdata(handles.output, 'Detdata', data);

% --- Outputs from this function are returned to the command line.
function varargout = FlyDetection_OutputFcn(hObject, eventdata, handles) 
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
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;
col=horusdata.col;

%close N2 valves to detection tubes
if statusData(lastrow,col.ValidSlaveDataFlag)
    Valveword=bitset(statusData(lastrow,col.Valve1armAxis),13,0);
    Valveword=bitset(Valveword,14,0);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to keep Pitot Zero open
end

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
horustxtBlower=horusdata.txtBlower;
if ~isequal(get(horustxtBlower,'BackgroundColor'),[0 1 1]) % blower connected via tcpip (ground configuration)
    set(hObject,'BackgroundColor','y');
    tcpBlower=horusdata.tcpBlower;
    % check Blower and Pump status 
    fprintf(tcpBlower,'status'); 
    pause(0.5);
    BlowerStatus=tcpBlower.UserData;
    tcpBlower.UserData=[];
    if BlowerStatus(strfind(BlowerStatus,'Pump')+7)=='f'
        PumpSwitch=0;
    elseif BlowerStatus(strfind(BlowerStatus,'Pump')+7)=='n'
        PumpSwitch=1;
    else PumpSwitch=-1;
    end
    if BlowerStatus(strfind(BlowerStatus,'Inverter')+11)=='f'
        InverterSwitch=0;
    elseif BlowerStatus(strfind(BlowerStatus,'Inverter')+11)=='n'
        InverterSwitch=1;
    else InverterSwitch=-1;
    end
    if BlowerStatus(strfind(BlowerStatus,'Ramp')+7)=='f'
        RampSwitch=0;
    elseif BlowerStatus(strfind(BlowerStatus,'Ramp')+7)=='n'
        RampSwitch=1;
    else RampSwitch=-1;
    end
end

if statusData(lastrow,col.ValidSlaveDataFlag) % only if armaxis is active
    if get(hObject,'Value') % switch on
        if ~isequal(get(horustxtBlower,'BackgroundColor'),[0 1 1]) % Blower connected via tcp (ground configuration)
                set(hObject,'BackgroundColor','y','String','switching Blower ON');
                % switch on Blower only when pump is on and cell pressure P1000
                % is low enough and Butterfly has been initialized
   %             if (PumpSwitch==0 | statusData(lastrow,col.P1000)>10300 | statusData(lastrow,col.ButterflyPositionValid)==0)
%                    set(handles.txtP1000,'BackgroundColor','r');
%                    disp('Pressure too high or Butterfly not initialized');
%                    set(hObject,'BackgroundColor','c','String','Blower OFF');                
    %            else
                    set(handles.txtP1000,'BackgroundColor',[0.7 0.7 0.7]);
                    fprintf(tcpBlower,'inverter on');
                    pause(0.5)
                    tcpBlower.UserData=[];
                    fprintf(tcpBlower,'ramp on');
                    pause(0.5)
                    tcpBlower.UserData=[];
     %           end

        else % Blower connected directly to armaxis (air configuration)
            if isequal(get(hObject,'BackgroundColor'),[0 1 1])
                set(hObject,'BackgroundColor','y','String','switching Blower ON');
                % switch on Blower only when pump is on and cell pressure
                % P1000
                % is low enough and Butterfly has been initialized
% Butterfly with stepper motor
                %                if ( (bitget(statusData(lastrow,col.Valve2armAxis),10)==0 | bitget(statusData(lastrow,col.Valve2armAxis),7)==0) ...
%                    | statusData(lastrow,col.P1000)>10300 | statusData(lastrow,col.ButterflyPositionValid)==0)
% end Butterfly with stepper motor
% Butterfly with relay
                                if ( (bitget(statusData(lastrow,col.Valve2armAxis),10)==0 | bitget(statusData(lastrow,col.Valve2armAxis),7)==0) ...
                    | statusData(lastrow,col.P1000)>10300)
% end Butterfly with relay
                    set(handles.txtP1000,'BackgroundColor','r');
                    disp('Pressure too high or Butterfly not initialized');
                    set(hObject,'BackgroundColor','c','String','Blower OFF');                
                else
                    set(handles.txtP1000,'BackgroundColor',[0.7 0.7 0.7]);
                    Valveword=bitset(statusData(lastrow,col.Valve2armAxis),10);  % make sure Leybold pump is not switched off
                    Valveword=bitset(Valveword,7);  % make sure Scroll pump is not switched off
                    Valveword=bitset(Valveword,9); % switch on blower
                    Valveword=bitset(Valveword,1); % ramp blower up 
                    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
                    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
                    set(hObject,'BackgroundColor','g','String','Blower ON');
                end
            end
        end
    else % switch off
        if ~isequal(get(horustxtBlower,'BackgroundColor'),[0 1 1]) % Blower connected via tcp (ground configuration)
                set(hObject,'BackgroundColor','y','String','switching Blower OFF');
% Butterfly with stepper motor
                system(['/lift/bin/eCmd @armAxis s butterflyposition ',num2str(42)]); % close Butterfly 
% end Butterfly with stepper motor
% Butterfly with relay
                Valveword=bitset(statusData(lastrow,col.Valve2armAxis),2,1); % close Butterfly
                system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
% end Butterfly with relay

                set(handles.togButterfly,'BackgroundColor','r','String','MOVING');
                fprintf(tcpBlower,'ramp off');  % ramp blower down
                pause(0.5);
                tcpBlower.UserData=[];
                pause(10);
                fprintf(tcpBlower,'inverter off');  % switch off blower
                tcpBlower.UserData=[];

        else % Blower connected directly to armaxis (air configuration)
            if isequal(get(hObject,'BackgroundColor'),[0 1 0]) | isequal(get(hObject,'BackgroundColor'),[1 0 0])
                set(hObject,'BackgroundColor','y','String','switching Blower OFF');
% Butterfly with stepper motor                
                system(['/lift/bin/eCmd @armAxis s butterflyposition ',num2str(42)]); % close Butterfly 
% end Butterfly with stepper motor
% Butterfly with relay
                Valveword=bitset(statusData(lastrow,col.Valve2armAxis),2,1); % close Butterfly
                system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
% end Butterfly with relay
                set(handles.togButterfly,'BackgroundColor','r','String','MOVING');
                Valveword=bitset(statusData(lastrow,col.Valve2armAxis),1,0); % ramp blower down
    %            Valveword=bitset(Valveword,13); % ventilate Pump
                system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
                system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
                system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to other valves working
    %            set(handles.tglVent,'BackgroundColor','r');
                pause(5);
                Valveword=bitset(statusData(lastrow,col.Valve2armAxis),1,0); % make sure ramp down switch is set
    %            Valveword=bitset(Valveword,13); % ventilate Pump
                Valveword=bitset(Valveword,9,0); % switch off blower
                system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
                system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
                set(hObject,'BackgroundColor','c','String','Blower OFF');
            end
        end
    end
end

%for ground configuration, recheck pump status
if ~isequal(get(horustxtBlower,'BackgroundColor'),[0 1 1])
    fprintf(tcpBlower,'status'); 
    pause(0.5);
    BlowerStatus=tcpBlower.UserData;
    tcpBlower.UserData=[];
    if BlowerStatus(strfind(BlowerStatus,'Pump')+7)=='f'
        PumpSwitch=0;
    elseif BlowerStatus(strfind(BlowerStatus,'Pump')+7)=='n'
        PumpSwitch=1;
    else PumpSwitch=-1;
    end
    if BlowerStatus(strfind(BlowerStatus,'Inverter')+11)=='f'
        InverterSwitch=0;
    elseif BlowerStatus(strfind(BlowerStatus,'Inverter')+11)=='n'
        InverterSwitch=1;
    else InverterSwitch=-1;
    end
    if BlowerStatus(strfind(BlowerStatus,'Ramp')+7)=='f'
        RampSwitch=0;
    elseif BlowerStatus(strfind(BlowerStatus,'Ramp')+7)=='n'
        RampSwitch=1;
    else RampSwitch=-1;
    end
   
    if PumpSwitch==0
        set(horustxtBlower,'String','Pump OFF'); 
    else
        set(horustxtBlower,'String','Pump ON'); 
    end
    if (RampSwitch==0 | InverterSwitch==0)
        set(hObject,'BackgroundColor','c','String','Blower OFF');
    else
        set(horustxtBlower,'String','Blower ON','BackgroundColor','g');
        set(hObject,'BackgroundColor','g','String','Blower ON');
    end
    if (PumpSwitch==-1 | RampSwitch==-1 | InverterSwitch==-1)
        set(hObject,'BackgroundColor','r','String','Blower ERR');
        set(horustxtBlower,'String','Blower ERROR','BackgroundColor','r');
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
        set(hObject,'BackgroundColor','r','String','Lamp ON');
    else
        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),11,0);
        set(hObject,'BackgroundColor','c','String','Lamp OFF');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
    data.calstatus=1;
    setappdata(handles.output, 'Detdata', data);
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
        set(hObject,'BackgroundColor','r');
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
        set(hObject,'BackgroundColor','r');
    else
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),3,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to keep Pitot Zero open
end


% --- Executes on button press in toggleC3F6. (internal addition of C3F6!)
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
        set(hObject,'BackgroundColor','r');
    else
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),4,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to keep Pitot Zero open
end


% --- Executes on button press in toggleNO1. --> meanwhile used for
% IPISynAir valve
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
    pause(0.5);
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
MaxFlow=34.18;
setMFC=str2double(get(hObject,'String'));
if setMFC<0 | isnan(setMFC) setMFC=0; end
if setMFC>MaxFlow setMFC=MaxFlow; end
set(hObject,'String',num2str(setMFC));

Value=setMFC/MaxFlow*255;
%system(['/lift/bin/eCmd @armAxis w 0xa404 0xFF']); % initialise
system(['/lift/bin/eCmd @armAxis w 0xa440 ', num2str(uint16(Value))]);



% --- Executes on button press in chkMFCNO.
function chkMFCNO_Callback(hObject, eventdata, handles)
% hObject    handle to chkMFCNO (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkMFCNO


% --- Executes on button press in chkDiodeUV.
function chkDiodeUV_Callback(hObject, eventdata, handles)
% hObject    handle to chkDiodeUV (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkDiodeUV




% --- Executes on button press in chkTDetFunnel.
function chkTDetFunnel_Callback(hObject, eventdata, handles)
% hObject    handle to chkTDetFunnel (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkTDetFunnel


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
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),14,1);
        set(hObject,'BackgroundColor','g');
    else
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),14,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
    pause(0.5);
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
    pause(0.5);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to keep Pitot Zero open
end


% --- Executes on button press in tglKuv.
%function tglKuv_Callback(hObject, eventdata, handles)
% hObject    handle to tglKuv (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of tglKuv
%horusdata = getappdata(handles.parenthandle, 'horusdata');
%statusData=horusdata.statusData;
%col=horusdata.col;
%data = getappdata(handles.output, 'Detdata');
%lastrow=data.lastrow;

%if statusData(lastrow,col.ValidSlaveDataFlag)
%    if get(hObject,'Value')
%        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),12);
%        set(hObject,'BackgroundColor','g');
%    else
%        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),12,0);
%        set(hObject,'BackgroundColor','c');
%    end
%    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
%    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
%    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to other valves working
%end


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


% --- Executes on button press in chkMFCProp.
function chkMFCProp_Callback(hObject, eventdata, handles)
% hObject    handle to chkMFCProp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkMFCProp


% --- Executes on button press in chkMFCShower.
function chkMFCShower_Callback(hObject, eventdata, handles)
% hObject    handle to chkMFCShower (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkMFCShower




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
        set(hObject,'BackgroundColor','y','String','Pitot 0 ON');
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
    else
        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),3,0);
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


% --- Executes on button press in chkMFCC3F6.
function chkMFCC3F6_Callback(hObject, eventdata, handles)
% hObject    handle to chkMFCC3F6 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkMFCC3F6


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
    else
        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),4,0);
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
    else
        Valveword=bitset(statusData(lastrow,col.Valve2armAxis),5,0);
    end
    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
end


% --- Executes on button press in tglVent.
function tglVent_Callback(hObject, eventdata, handles)
% hObject    handle to tglVent (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
col=horusdata.col;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;

if statusData(lastrow,col.ValidSlaveDataFlag)
    if get(hObject,'Value')
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),4);
        set(hObject,'BackgroundColor','r');
    else
        Valveword=bitset(statusData(lastrow,col.Valve1armAxis),4,0);
        set(hObject,'BackgroundColor','c');
    end
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
    system(['/lift/bin/eCmd @armAxis w 0xa408 ', num2str(Valveword)]);
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(15*140))]); % 15V needed to keep Pitot Zero open
end


% --- Executes on button press in togButterfly.
function togButterfly_Callback(hObject, eventdata, handles)
% hObject    handle to togButterfly (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togButterfly
horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;
col=horusdata.col;
% Butterfly with stepper motor
%if statusData(lastrow,col.ButterflyPositionValid)==0
%    system('/lift/bin/eCmd @armAxis s butterflyposition 2500'); % move to find index position
%    set(hObject,'BackgroundColor','r','String','MOVING');
%    pause(2);
%    system(['/lift/bin/eCmd @armAxis s butterflyposition ',num2str(625+42)]); % open Butterfly 
%    set(hObject,'Value',1);
%else
%    if get(hObject,'Value')
%        system('/lift/bin/eCmd @armAxis s butterflyposition 2500'); % move to find index position
%        set(hObject,'BackgroundColor','r','String','MOVING');
%        pause(2);
%        system(['/lift/bin/eCmd @armAxis s butterflyposition ',num2str(625+42)]); % open Butterfly 
%    else
%        system(['/lift/bin/eCmd @armAxis s butterflyposition ',num2str(42)]); % close Butterfly 
%        set(hObject,'BackgroundColor','r','String','MOVING');
%    end
%end
% end Butterfly with stepper motor

% Butterfly with relay
if get(hObject,'Value')
    Valveword=bitset(statusData(lastrow,col.Valve2armAxis),2,0); % open Butterfly
    set(hObject,'BackgroundColor','r','String','MOVING');
else
    Valveword=bitset(statusData(lastrow,col.Valve2armAxis),2,1); % close Butterfly
    set(hObject,'BackgroundColor','r','String','MOVING');
end
    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
% end Butterfly with relay



% --- Executes on button press in togPump.
function togPump_Callback(hObject, eventdata, handles)
% hObject    handle to togPump (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togPump

horusdata = getappdata(handles.parenthandle, 'horusdata');
statusData=horusdata.statusData;
data = getappdata(handles.output, 'Detdata');
lastrow=data.lastrow;
col=horusdata.col;
horustxtBlower=horusdata.txtBlower;
if ~isequal(get(horustxtBlower,'BackgroundColor'),[0 1 1])
    set(hObject,'BackgroundColor','y');
    tcpBlower=horusdata.tcpBlower;
    % check if Blower and/or pump are on or off
    fprintf(tcpBlower,'status'); 
    pause(0.5);
    BlowerStatus=tcpBlower.UserData;
    tcpBlower.UserData=[];
    if BlowerStatus(strfind(BlowerStatus,'Pump')+7)=='f'
        PumpSwitch=0;
    elseif BlowerStatus(strfind(BlowerStatus,'Pump')+7)=='n'
        PumpSwitch=1;
    else PumpSwitch=-1;
    end
    if BlowerStatus(strfind(BlowerStatus,'Inverter')+11)=='f'
        InverterSwitch=0;
    elseif BlowerStatus(strfind(BlowerStatus,'Inverter')+11)=='n'
        InverterSwitch=1;
    else InverterSwitch=-1;
    end
    if BlowerStatus(strfind(BlowerStatus,'Ramp')+7)=='f'
        RampSwitch=0;
    elseif BlowerStatus(strfind(BlowerStatus,'Ramp')+7)=='n'
        RampSwitch=1;
    else RampSwitch=-1;
    end
end

if get(hObject,'Value') % switch on
    if ~isequal(get(horustxtBlower,'BackgroundColor'),[0 1 1]) % Blower connected via tcp (ground configuration)
            fprintf(tcpBlower,'pump on'); % switch pump on
            pause(0.5);
            tcpBlower.UserData=[];
    else % Blower connected directly to armaxis (air configuration)
        if statusData(lastrow,col.ValidSlaveDataFlag) % only if armaxis is active
            if isequal(get(hObject,'BackgroundColor'),[0 1 1])
                Valveword=bitset(statusData(lastrow,col.Valve2armAxis),10);  % switch Leybold pump on
                Valveword=bitset(Valveword,7);  % switch Scroll pump on
                system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to switch
                system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
                set(hObject,'BackgroundColor','g','String','Pump ON');
            end
        end
    end
else % switch off
    if ~isequal(get(horustxtBlower,'BackgroundColor'),[0 1 1]) % Blower connected via tcp (ground configuration)
            if InverterSwitch==0 % make sure blower is ramped down
                fprintf(tcpBlower,'pump off'); % switch pump off
                pause(0.5);
                tcpBlower.UserData=[];
% Butterfly with stepper motor                
                system(['/lift/bin/eCmd @armAxis s butterflyposition ',num2str(42)]); % close Butterfly 
% end Butterfly with stepper motor
% Butterfly with relay
                Valveword=bitset(statusData(lastrow,col.Valve2armAxis),2,1); % close Butterfly
                system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
% end Butterfly with relay

                set(handles.togButterfly,'BackgroundColor','r','String','MOVING');
            end
    else % Blower connected directly to armaxis (air configuration)
        if statusData(lastrow,col.ValidSlaveDataFlag) % only if armaxis is active
            if isequal(get(hObject,'BackgroundColor'),[0 1 0]) | isequal(get(hObject,'BackgroundColor'),[1 0 0])
   %             set(handles.tglVent,'BackgroundColor','r');
                if bitget(statusData(lastrow,col.Valve2armAxis),1)==0 % make sure blower is ramped down
                    Valveword=bitset(statusData(lastrow,col.Valve2armAxis),9,0); % switch off blower
                    Valveword=bitset(Valveword,10,0);  % switch off Leybold Pump
                    Valveword=bitset(Valveword,7,0);  % switch off Scroll Pump
    %                Valveword=bitset(Valveword,13);  % ventilate Pump
% Butterfly with stepper motor                
                    system(['/lift/bin/eCmd @armAxis s butterflyposition ',num2str(42)]); % close Butterfly 
% end Butterfly with stepper motor
% Butterfly with relay
                    Valveword=bitset(statusData(lastrow,col.Valve2armAxis),2,1); % close Butterfly
                    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
% end Butterfly with relay
                    set(handles.togButterfly,'BackgroundColor','r','String','MOVING');
                    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(24*140))]); % 24V needed to switch solenoids on
                    system(['/lift/bin/eCmd @armAxis w 0xa40a ', num2str(Valveword)]);
                    system(['/lift/bin/eCmd @armAxis w 0xa462 ', num2str(uint16(18*140))]); % 18V needed to other valves working
                    set(hObject,'BackgroundColor','c','String','Pump OFF');
    %                set(handles.tglVent,'BackgroundColor','r');
                end
            end
        end
    end
end

%for ground configuration, recheck pump status 
if ~isequal(get(horustxtBlower,'BackgroundColor'),[0 1 1])
    fprintf(tcpBlower,'status'); 
    pause(0.5);
    BlowerStatus=tcpBlower.UserData;
    tcpBlower.UserData=[];
    if BlowerStatus(strfind(BlowerStatus,'Pump')+7)=='f'
        PumpSwitch=0;
    elseif BlowerStatus(strfind(BlowerStatus,'Pump')+7)=='n'
        PumpSwitch=1;
    else PumpSwitch=-1;
    end
    if BlowerStatus(strfind(BlowerStatus,'Inverter')+11)=='f'
        InverterSwitch=0;
    elseif BlowerStatus(strfind(BlowerStatus,'Inverter')+11)=='n'
        InverterSwitch=1;
    else InverterSwitch=-1;
    end
    if BlowerStatus(strfind(BlowerStatus,'Ramp')+7)=='f'
        RampSwitch=0;
    elseif BlowerStatus(strfind(BlowerStatus,'Ramp')+7)=='n'
        RampSwitch=1;
    else RampSwitch=-1;
    end
   
    if PumpSwitch==0
        set(hObject,'BackgroundColor','c','String','Pump OFF');
        set(horustxtBlower,'String','Pump OFF'); 
    elseif (RampSwitch==0 | InverterSwitch==0)
        set(hObject,'BackgroundColor','g','String','Pump ON');
        set(horustxtBlower,'String','Pump ON'); 
    else
        set(horustxtBlower,'String','Blower ON','BackgroundColor','g');
    end
    if (PumpSwitch==-1 | RampSwitch==-1 | InverterSwitch==-1)
        set(hObject,'BackgroundColor','r','String','Pump ERR');
        set(horustxtBlower,'String','Blower ERROR','BackgroundColor','r');
    end
end



function editMFCShower_Callback(hObject, eventdata, handles)
% hObject    handle to editMFCShower (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editMFCShower as text
%        str2double(get(hObject,'String')) returns contents of editMFCShower as a double
MaxFlow=20000;
setMFC=str2double(get(hObject,'String'));
if setMFC<0 | isnan(setMFC) setMFC=0; end
if setMFC>MaxFlow setMFC=MaxFlow; end
set(hObject,'String',num2str(setMFC));

Value=setMFC/MaxFlow*255;
%system(['/lift/bin/eCmd @armAxis w 0xa404 0xFF']); % initialise
system(['/lift/bin/eCmd @armAxis w 0xa446 ', num2str(uint16(Value))]);


% --- Executes during object creation, after setting all properties.
function editMFCShower_CreateFcn(hObject, eventdata, handles)
% hObject    handle to editMFCShower (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function editMFCProp_Callback(hObject, eventdata, handles)
% hObject    handle to editMFCProp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editMFCProp as text
%        str2double(get(hObject,'String')) returns contents of editMFCProp as a double
MaxFlow=200*.41;
setMFC=str2double(get(hObject,'String'));
if setMFC<0 | isnan(setMFC) setMFC=0; end
if setMFC>MaxFlow setMFC=MaxFlow; end
set(hObject,'String',num2str(setMFC));

Value=setMFC/MaxFlow*255;
%system(['/lift/bin/eCmd @armAxis w 0xa404 0xFF']); % initialise
system(['/lift/bin/eCmd @armAxis w 0xa442 ', num2str(uint16(Value))]);


% --- Executes during object creation, after setting all properties.
function editMFCProp_CreateFcn(hObject, eventdata, handles)
% hObject    handle to editMFCProp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function editMFCC3F6_Callback(hObject, eventdata, handles)
% hObject    handle to editMFCC3F6 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editMFCC3F6 as text
%        str2double(get(hObject,'String')) returns contents of editMFCC3F6 as a double
MaxFlow=2000*0.249;
setMFC=str2double(get(hObject,'String'));
if setMFC<0 | isnan(setMFC) setMFC=0; end
if setMFC>MaxFlow setMFC=MaxFlow; end
set(hObject,'String',num2str(setMFC));

Value=setMFC/MaxFlow*255;
%system(['/lift/bin/eCmd @armAxis w 0xa404 0xFF']); % initialise
system(['/lift/bin/eCmd @armAxis w 0xa444 ', num2str(uint16(Value))]);


% --- Executes during object creation, after setting all properties.
function editMFCC3F6_CreateFcn(hObject, eventdata, handles)
% hObject    handle to editMFCC3F6 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function editMFCIPISynAir_Callback(hObject, eventdata, handles)
% hObject    handle to editMFCIPISynAir (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editMFCIPISynAir as text
%        str2double(get(hObject,'String')) returns contents of editMFCIPISynAir as a double


% --- Executes during object creation, after setting all properties.
function editMFCIPISynAir_CreateFcn(hObject, eventdata, handles)
% hObject    handle to editMFCIPISynAir (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in chkMFCIPISynAir.
function chkMFCIPISynAir_Callback(hObject, eventdata, handles)
% hObject    handle to chkMFCIPISynAir (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of chkMFCIPISynAir


