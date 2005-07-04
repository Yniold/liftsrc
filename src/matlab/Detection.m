
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
col=horusdata.col;
fcts2val=horusdata.fcts2val;

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
x=double(statusData(:,col.Temp3WP)); eval(['TDet=',fcts2val.Temp3WP,';']);
x=double(statusData(:,col.P20)); eval(['P20=',fcts2val.P20,';']);
%x=double(statusData(:,col.P1000)); eval(['P1000=',fcts2val.P1000,';']);
x=double(statusData(:,col.DiodeWZ1out)); eval(['DiodeWZ1out=',fcts2val.DiodeWZ1out,';']);
x=double(statusData(:,col.DiodeWZ2out)); eval(['DiodeWZ2out=',fcts2val.DiodeWZ2out,';']);
x=double(statusData(:,col.DiodeWZ1in)); eval(['DiodeWZ1in=',fcts2val.DiodeWZ1in,';']);
x=double(statusData(:,col.DiodeWZ2in)); eval(['DiodeWZ2in=',fcts2val.DiodeWZ2in,';']);
x=double(statusData(:,col.PNO)); eval(['PNO=',fcts2val.PNO,';']);
x=double(statusData(:,col.MFCFlow)); eval(['MFCFlow=',fcts2val.MFCFlow,';']);


set(handles.txtWZ1in,'String',[num2str(DiodeWZ1in(lastrow),3),' mW']);
set(handles.txtWZ1out,'String',[num2str(DiodeWZ1out(lastrow),3),' mW']);
set(handles.txtWZ2in,'String',[num2str(DiodeWZ2in(lastrow),3),' mW']);
set(handles.txtWZ2out,'String',[num2str(DiodeWZ2out(lastrow),3),' mW']);
set(handles.txtP1000,'String',statusData(lastrow,col.P1000));
set(handles.txtP20,'String',[num2str(P20(lastrow),3),' mbar']);
set(handles.txtPNO,'String',[num2str(PNO(lastrow),4),' mbar']);
set(handles.txtVHV,'String',statusData(lastrow,col.VHV));
set(handles.txtTDet,'String',[num2str(TDet(lastrow),3),' C']);
set(handles.txtMFC,'String',[num2str(MFCFlow(lastrow),3),' sccm']);

% warn for ADC signals out of allowed range for measurements
if P20(lastrow)<3 | P20(lastrow)>4
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
if MFCFlow(lastrow)<5.5 | MFCFlow>6
    set(handles.txtMFC,'BackgroundColor','r');
else 
    set(handles.txtMFC,'BackgroundColor',[0.7 0.7 0.7]);
end
if statusData(lastrow,col.VHV)<12630
    set(handles.txtVHV,'BackgroundColor','r');
else 
    set(handles.txtVHV,'BackgroundColor',[0.7 0.7 0.7]);
end




% plot checked parameters vs. time
hold(handles.axes1,'off'); 
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
xlim(handles.axes1,[limTime1 limTime2]);
grid(handles.axes1);

%plot PMT and MCP signals

PMTBase=col.ccData0;
MCP1Base=col.ccData1;
MCP2Base=col.ccData2;
AVGBase=1;
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
PMTOfflineAvg(OfflineRightFilter & statusData(:,col.RAvgOnOffFlag)==1)=PMTOfflineRightAvg(OfflineRightFilter & statusData(:,col.RAvgOnOffFlag)==1);
PMTOfflineAvg(OfflineLeftFilter & statusData(:,col.RAvgOnOffFlag)==2)=PMTOfflineLeftAvg(OfflineLeftFilter & statusData(:,col.RAvgOnOffFlag)==2);

PMTAvg(statusData(:,col.RAvgOnOffFlag)==3)=PMTOnlineAvg(statusData(:,col.RAvgOnOffFlag)==3);
PMTAvg(statusData(:,col.RAvgOnOffFlag)==2)=PMTOfflineLeftAvg(statusData(:,col.RAvgOnOffFlag)==2);
PMTAvg(statusData(:,col.RAvgOnOffFlag)==1)=PMTOfflineRightAvg(statusData(:,col.RAvgOnOffFlag)==1);
PMTAvg(statusData(:,col.RAvgOnOffFlag)==0)=NaN;

MCP1OnlineAvg(OnlineFilter)=AvgData(OnlineFilter,AVGBase+3);
MCP1OnlineAvg(~OnlineFilter)=NaN;
MCP1OfflineLeftAvg(OfflineLeftFilter)=AvgData(OfflineLeftFilter,AVGBase+4);
MCP1OfflineLeftAvg(~OfflineLeftFilter)=NaN;
MCP1OfflineRightAvg(OfflineRightFilter)=AvgData(OfflineRightFilter,AVGBase+5);
MCP1OfflineRightAvg(~OfflineRightFilter)=NaN;

MCP1OfflineAvg(1:size(statusData,1))=NaN;
MCP1OfflineAvg(OfflineRightFilter & statusData(:,col.RAvgOnOffFlag)==1)=MCP1OfflineRightAvg(OfflineRightFilter & statusData(:,col.RAvgOnOffFlag)==1);
MCP1OfflineAvg(OfflineLeftFilter & statusData(:,col.RAvgOnOffFlag)==2)=MCP1OfflineLeftAvg(OfflineLeftFilter & statusData(:,col.RAvgOnOffFlag)==2);

MCP1Avg(statusData(:,col.RAvgOnOffFlag)==3)=MCP1OnlineAvg(statusData(:,col.RAvgOnOffFlag)==3);
MCP1Avg(statusData(:,col.RAvgOnOffFlag)==2)=MCP1OfflineLeftAvg(statusData(:,col.RAvgOnOffFlag)==2);
MCP1Avg(statusData(:,col.RAvgOnOffFlag)==1)=MCP1OfflineRightAvg(statusData(:,col.RAvgOnOffFlag)==1);
MCP1Avg(statusData(:,col.RAvgOnOffFlag)==0)=NaN;

MCP2OnlineAvg(OnlineFilter)=AvgData(OnlineFilter,AVGBase+6);
MCP2OnlineAvg(~OnlineFilter)=NaN;
MCP2OfflineLeftAvg(OfflineLeftFilter)=AvgData(OfflineLeftFilter,AVGBase+7);
MCP2OfflineLeftAvg(~OfflineLeftFilter)=NaN;
MCP2OfflineRightAvg(OfflineRightFilter)=AvgData(OfflineRightFilter,AVGBase+8);
MCP2OfflineRightAvg(~OfflineRightFilter)=NaN;

MCP2OfflineAvg(1:size(statusData,1))=NaN;
MCP2OfflineAvg(OfflineRightFilter & statusData(:,col.RAvgOnOffFlag)==1)=MCP2OfflineRightAvg(OfflineRightFilter & statusData(:,col.RAvgOnOffFlag)==1);
MCP2OfflineAvg(OfflineLeftFilter & statusData(:,col.RAvgOnOffFlag)==2)=MCP2OfflineLeftAvg(OfflineLeftFilter & statusData(:,col.RAvgOnOffFlag)==2);

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

%calculate OH and HO2 mixing ratios
if statusData(lastrow,col.ValidSlaveDataFlag)
    radlife=1.45e6;			% Radiative lifetime (Hz) from D. Heard data
    % THESE PARAMETERS NEED TO BE CONSIDERED WHEN RUNNING THE INSTRUMENT IN A
    % DIFFERENT SETUP
    gate1=200e-9;			   % Approximate gate setting for rising edge (sec)
    gate2=620e-9;		   	% ...for falling edge (sec)
    Tcal=293;			      % Cell Temperature during lab calibration (K)
    Pcal=4.0;
    PowCal=4.0;		%OHUVPower during lab calibration (mW)
    PowCalb=4.0;		%HO2UVPower during lab calibration (mW)
    PowDep=0.0;		%sensitivity decrease per mW as a fraction from value at 0 mW
    wmrcal=8E-3;	         % Calibration reference water concentration
    cOH=12.5;    %The following parameters are sensitivity
    cHO2b=12.5;		%HO2 axis

    k_qcal=getq(Tcal,wmrcal);
    GAMMAcal= k_qcal*Pcal + radlife;  
    densCal=(6.022E+23/22400)*273/Tcal*4.9/1013;

    bc=boltzcorr(Tcal,TDet(lastrow)+273);
    k_q=getq(TDet(lastrow)+273,str2double(get(handles.editH2O,'String')));
    GAMMA = k_q*P20(lastrow) + radlife;

    quen = (1/GAMMA).*((exp(-gate1*GAMMA)-exp(-gate2*GAMMA)));
    quencal = (1/GAMMAcal).*((exp(-gate1*GAMMAcal)-exp(-gate2*GAMMAcal)));

    Dens=6.023E23/22400*273./(TDet(lastrow)+273)*P20/1013; %Converting to density

    COH=quen.*bc.*(str2double(get(handles.editC,'String'))/quencal/densCal)*Dens;
    COH=COH.*DiodeWZ1in(lastrow).*[1-PowDep*DiodeWZ1in(lastrow)]/[1-PowDep*PowCal];

    CHO2b=quen.*bc.*(str2double(get(handles.editC,'String'))/quencal/densCal)*Dens;
    CHO2b=CHO2b.*DiodeWZ2in(lastrow).*[1-PowDep*DiodeWZ2in(lastrow)]/[1-PowDep*PowCal];
    if rank(COH)~=0
        XOH = MCP1OnlineAvg.*5./COH';
    else
        XOH = MCP1OnlineAvg; XOH(:)=NaN;
    end
    if rank(CHO2b)~=0
        XHOx = MCP2OnlineAvg.*5./CHO2b';
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



% check Butterfly
if bitget(statusData(lastrow,col.Valve2armAxis),7)==0
%    set(handles.togButterfly,'Value',1)
    set(handles.togButterfly,'BackgroundColor','g','String','Butterfly OPEN');
else
%    set(handles.togButterfly,'Value',0)
    set(handles.togButterfly,'BackgroundColor','c','String','Butterfly CLOSED');
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
        if isequal(get(hObject,'BackgroundColor'),[0 1 1])
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
        if isequal(get(hObject,'BackgroundColor'),[0 1 0])
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
function togButterfly_Callback(hObject, eventdata, handles)
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
    %    system(['/lift/bin/eCmd @armAxis w 0xa462 0']);
        set(hObject,'BackgroundColor','g','String','Butterfly OPEN');
    else
    %    system('/lift/bin/eCmd @armAxis w 0xa462 1800'); 
        set(hObject,'BackgroundColor','c','String','Butterfly CLOSED');
    end
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
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open
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
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open
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
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open
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
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open
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
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open
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
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open
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
    system(['/lift/bin/eCmd @armAxis w 0xa460 ', num2str(uint16(8*140))]); % 8V needed to keep solenoids open
end



function editxlim1_Callback(hObject, eventdata, handles)
% hObject    handle to editxlimit1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editxlimit1 as text
%        str2double(get(hObject,'String')) returns contents of editxlimit1 as a double
xlim1=uint8(str2double(get(hObject,'String')));
if (xlim1>200) set(hObject,'String','200');
else set(hObject,'String',num2str(xlim1));
end



function editxlim2_Callback(hObject, eventdata, handles)
% hObject    handle to editxlim2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of editxlim2 as text
%        str2double(get(hObject,'String')) returns contents of editxlim2 as a double
xlim2=uint8(str2double(get(hObject,'String')));
if xlim2>200 set(hObject,'String','200');
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
