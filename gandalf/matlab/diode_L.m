function varargout = diode_L(varargin)
% =============================================================
% Diode Laser Controller Program
%
% $Revision: 1.1 $ by $Author: pollmann $ %
% ============================================================
%
%{
   $Log: diode_L.m,v $
   Revision 1.1  2008/08/12 10:53:28  pollmann
   restructured directories

   Revision 1.1  2008/07/08 13:21:24  pollmann
   initial checkin of Laser Controller GUi for the blue diode Laser

   
%}																							 
% ============================================================


% M-file for diode_L.fig
%      diode_L, by itself, creates a new diode_L or raises the existing
%      singleton*.
%
%      H = Diode_L returns the handle to a new Diode_L or the handle to
%      the existing singleton*.
%
%      Diode_L('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in HORUS.M with the given input arguments.
%
%      Diode_L('Property','Value',...) creates a new Diode_L or raises the
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

% Last Modified by GUIDE v2.5 08-Jul-2008 13:10:48

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @diode_L_OpeningFcn, ...
                   'gui_OutputFcn',  @diode_L_OutputFcn, ...
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

% --- Executes just before diode_L is made visible.
function diode_L_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to diode_L (see VARARGIN)

% Choose default command line output for diode_L
handles.output = hObject;

% choose Text size
set(double(get(handles.figure1,'Children')),'FontSize',8)

% Update handles structure
guidata(hObject, handles);

% get NO2LIF handle
if length(varargin)==2 & varargin{1}==handle
    handles.parenthandle=str2double(varargin{2});
end

% choose Text size
%set(double(get(handles.figure1,'Children')),'FontSize',8)

%t = -300:10:-10; %predefine timeseries for diode monitoring
%Temp=ones (1,30)*25;
%tseries=[t;Temp];
%timer=-10;
%connected=0;

ll='off';
handles.ll=ll;    
%Laser variable describes Laser Status on = 2 (7 byte mode), off = 1 (16 byte mode); ll=laser light
% open tcpip port for communication with Laser,
%tport=tcpip('10.111.111.28',8000);
 tport = serial ('/dev/ttyS0','BaudRate',9600);
 set(tport,'ReadAsyncMode','continuous');
 set(tport,'BytesAvailableFcn',{'serialdatacallback'});
 set(tport,'Terminator','CR/LF');
 set(tport,'BytesAvailableFcnMode','Terminator');
 
 try
 fopen(tport);
 handles.tport=tport;
 data.tport=handles.tport;
 set(handles.txttport,'String','communication INITIALIZED','BackgroundColor','g');
 set(handles.pb_Exit,'backgroundcolor','g');
 
catch  
 % if communication with laser did not work
    fclose(tport);
    delete(tport);
    clear('tport');
    set(handles.txttport,'String','communication FAILED','BackgroundColor','r');
    set(handles.pb_Exit,'backgroundcolor','r');
 end



% Update handles structure
guidata(hObject, handles);

%if isfield(handles,'tport')
 %   Update_Callback(hObject, eventdata, handles);
%end

set(handles.rb_Laser_Connect,'BackgroundColor','r');
set(handles.rb_Laser_Control,'BackgroundColor','r');
set(handles.rb_analogue_on,'BackgroundColor','r');
set(handles.rb_pp_on,'BackgroundColor','r');
set(handles.pb_status,'BackgroundColor','r');
set(handles.txtenter_val_pp,'BackgroundColor','r');
set(handles.txtpp,'BackgroundColor','r');
set(handles.txtpp_value,'BackgroundColor','r');
set(handles.txtmod,'BackgroundColor','r');
set(handles.txtlaserout,'BackgroundColor','r');
set(handles.txtlaserpower,'BackgroundColor','r');
set(handles.txtfullpower,'backgroundcolor','r')
set(handles.rb_fullpower,'backgroundcolor','r');
% UIWAIT makes diode_L wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = diode_L_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Get default command line output from handles structure
varargout{1} = handles.output;
% tport=handles.tport;
%    if mod(sec,10) ==0
%        if connected == 1
%            fprintf(tport, 'Stat1');
%            Diode=tport.UserData;
%            tport.UserData= [];
            % answer:SL321 321 321 T 321 V 321 (T 321 is diode
            % temperature)
%            diode_T = 
%            timer=timer +10;
%            new=[timer;diode_T];
            %write to harddisk [time;diode]
%            tseries(:,1)=[]; % delete oldest T reading 
%            tseries=[tseries new]; %add new reading
            % update figure
%        end   
%    end   
  
    
% --- Executes on button press in rb_Laser_Control.

function rb_Laser_Control_Callback(hObject, eventdata, handles)
% hObject    handle to rb_Laser_Control (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

tport=handles.tport;

if get(hObject,'Value')
    
            Input='LON           ';
            Input=char(Input);
            set(handles.txtLaser_Res,'String',Input);
            fprintf(tport,Input);
            pause(1)
            Laser_Reply=tport.UserData;
            Laser_Reply=char(Laser_Reply);
            set(handles.txtLaser_Res,'String',Laser_Reply);
            tport.UserData=[];
                if strcmp('LONOK',Laser_Reply); 
                    set(handles.txtlaserpower,'String','Laser is on','backgroundcolor','g');%Laser is on
                    set(handles.rb_Laser_Control,'backgroundcolor','g');
                    ll='on';
                    handles.ll=ll;
                    set(handles.light,'String',ll);
                    pause(1);
                    set(handles.light,'String','Laser Emission','backgroundcolor','g');
                elseif strcmp('UK',Laser_Reply); 
                    set(handles.txtlaserpower,'String','error','backgroundcolor','r');%unknown error
                    set(handles.rb_Laser_Control,'backgroundcolor','r');
                else
                    set(handles.txtlaserpower,'String','connect to Laser first','backgroundcolor','r'); %print connect to Laser first   
                end
       
else
    
 Input='LF   ';
    Input=char(Input);
    set(handles.txtLaser_Res,'String',Input);
    fprintf(tport,Input);
    pause(1);
    Laser_Reply=tport.UserData;
    Laser_Reply=char(Laser_Reply);
    set(handles.txtLaser_Res,'String',Laser_Reply);
    tport.UserData=[];
        if strcmp('LOFFOK',Laser_Reply);
            set(handles.rb_Laser_Control,'backgroundcolor','r');
            set(handles.txtlaserpower,'String','Laser is off','backgroundcolor','r')
            ll='off';
            set(handles.light,'String',ll);
            pause(1);
            handles.ll=ll;
            set(handles.light,'String','Laser Dark','backgroundcolor','r');
        elseif strcmp('UK',Laser_Reply);
            set(handles.txtlaserpower,'String','error','backgroundcolor','r') %print unknown error
        else
            set(handles.txtlaserpower,'String','communication failed','backgroundcolor','r')
        end    
end



function txt_enter_L_commands_Callback(hObject, eventdata, handles)
% hObject    handle to txt_enter_L_commands (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of txt_enter_L_commands as text
%        str2double(get(hObject,'String')) returns contents of txt_enter_L_commands as a double
% send command and read answer, if any

tport=handles.tport;
ll=handles.ll;
LaserCmd=get(hObject,'String');
LaserCmd=char(LaserCmd);
set(handles.txtLaser_Res,'String',LaserCmd);
fprintf(tport,LaserCmd);
pause(1);
L_Answer=tport.UserData;
L_Answer=char(L_Answer);
tport.UserData=[];
set(handles.txtLaser_Res,'String',L_Answer);


% --- Executes during object creation, after setting all properties.
function txt_enter_L_commands_CreateFcn(hObject, eventdata, handles)
% hObject    handle to txt_enter_L_commands (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called
% tport=handles.tport;
% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
%if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
%    set(hObject,'BackgroundColor','white');
%plot t-series



% --- Executes on button press in rb_Laser_Connect.
function rb_Laser_Connect_Callback(hObject, eventdata, handles)
% hObject    handle to rb_Laser_Connect (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

tport=handles.tport;
ll='off';
handles.ll=ll;

if get(hObject,'Value')
  Input='RS            ';
     Input=char(Input);
     set(handles.txtLaser_Res,'String',Input);
     fprintf(tport,Input); 
     pause(1);
     Reset=tport.UserData;
     set(handles.txtLaser_Res,'String',Reset);
     Reset=char(Reset);
     tport.UserData=[];
           if strcmp(Reset,'RSOK'); %Laser now conncected
               set(handles.txtlaserout,'String','Laser is now connected','backgroundcolor','g')
               set(handles.rb_Laser_Connect,'backgroundcolor','g');
               set(handles.pb_status,'BackgroundColor','c');
               set(handles.txtenter_val_pp,'BackgroundColor','c');
               set(handles.txtpp,'BackgroundColor','r');
               set(handles.txtpp_value,'BackgroundColor','r');
               set(handles.txtmod,'BackgroundColor','r');
               set(handles.txtlaserout,'BackgroundColor','r');
               set(handles.txtlaserpower,'BackgroundColor','r');
           elseif strcmp(Reset,'RSOKRSOK'); %error Temperature too low
               set(handles.txtlaserout,'String','Laser is now connected','backgroundcolor','g')
               set(handles.rb_Laser_Connect,'backgroundcolor','g');
               set(handles.pb_status,'BackgroundColor','c');
               set(handles.txtenter_val_pp,'BackgroundColor','c');
               set(handles.txtpp,'BackgroundColor','c');
               set(handles.txtpp_value,'BackgroundColor','c');
               set(handles.txtmod,'BackgroundColor','c');
               set(handles.txtlaserout,'BackgroundColor','c');
               set(handles.txtlaserpower,'BackgroundColor','c');
           elseif strcmp(Reset,'TLO'); %error Temperature too low
               set(handles.txtlaserout,'String','error: Temperature too low')
           elseif strcmp(Reset,'THI'); %error Temperature too high
               set(handles.txtlaserout,'String','error: Temperature too high')
           elseif strcmp(Reset,'INT'); %no diode Temperature regulation
               set(handles.txtlaserout,'String','error: no diode temperature regulation')
           elseif strcmp(Reset,'UK'); %unknown error
               set(handles.txtlaserout,'String','unknown error, unplug laser')
           elseif strcmp(Reset,'RSBAD'); % Really bad error not able to connect
               set(handles.txtlaserout,'String','serious error')
           else
               set(handles.txtlaserout,'String','failed to connect')
               set(handles.rb_Laser_Connect,'backgroundcolor','r');
           end
        
else 
   
 Input='LF   ';
    Input=char(Input);
    set(handles.txtLaser_Res,'String',Input);
    fprintf(tport,Input);
    pause(1);
    Reset=tport.UserData;
    Reset=char(Reset);
    tport.UserData=[];
       %hier Abfrage von laserstatus wenn 1 einfach ausstellen wenn 2 untere Funktion!! 
       if strcmp(Reset,'LOFFOK');
            set(handles.rb_Laser_Control,'backgroundcolor','r')
            set(handles.txtLaser_Res,'String','Laser is offoff','backgroundcolor','r')
        elseif strcmp(Reset,'UK'); 
            set(handles.txtLaser_Res,'String','error')%print unknown error
        else
                    set(handles.txtLaser_Res,'String','failed to connect')
        end  
end      


% --- Executes on button press in pb_Exit.
function pb_Exit_Callback(hObject, eventdata, handles)
% hObject    handle to pb_Exit (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

tport=handles.tport;
    Input='LF   ';
    Input=char(Input);
    set(handles.txtLaser_Res,'String',Input);
    fprintf(tport,Input);
    pause(1)
    Input='A2DF ';
    Input=char(Input);
    set(handles.txtLaser_Res,'String',Input);
    fprintf(tport,Input);
    pause(1)
    Input='IPF  ';
    Input=char(Input);
    set(handles.txtLaser_Res,'String',Input);
    fprintf(tport,Input);
    pause(1)
    Input='MO1  ';
    Input=char(Input);
    set(handles.txtLaser_Res,'String',Input);
    fprintf(tport,Input);
    pause(1);
    fclose(tport);
    delete(tport);
    clear('tport');
    set(handles.txtfullpower,'backgroundcolor','r')
    set(handles.rb_fullpower,'backgroundcolor','r');
    set(handles.txttport,'String','Communication stopped','backgroundcolor','r')
    set(handles.rb_Laser_Control,'backgroundcolor','r');
    set(handles.rb_Laser_Connect,'backgroundcolor','r');
    set(handles.txtlaserpower,'backgroundcolor','r');
    set(handles.txtlaserout,'String','Restart Program','backgroundcolor','r')
    set(handles.pb_Exit,'backgroundcolor','r');
    set(handles.rb_analogue_on,'BackgroundColor','r');
    set(handles.rb_pp_on,'BackgroundColor','r');
    set(handles.pb_status,'BackgroundColor','r');
    set(handles.txtenter_val_pp,'BackgroundColor','r');


% --- Executes on button press in rb_analogue_on.
function rb_analogue_on_Callback(hObject, eventdata, handles)
% hObject    handle to rb_analogue_on (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of rb_analogue_on

tport=handles.tport;
ll=handles.ll;

if get(hObject,'Value')
    if strcmp(ll,'on')
        Input=('A2DO ');
        Input=char(Input);
        set(handles.txtLaser_Res,'String',Input);
        fprintf(tport,Input); 
        pause(1);
        Reset=tport.UserData;
        set(handles.txtLaser_Res,'String',Reset);
        Reset=char(Reset);
        tport.UserData=[];
           if strcmp(Reset,'A2D ON'); %Laser now conncected
               set(handles.txtmod,'String','modulation enabled','backgroundcolor','g')
               set(handles.rb_analogue_on,'backgroundcolor','g');
           elseif strcmp(Reset,'UK'); %error Temperature too low
               set(handles.txtmod,'String','unknown error','backgroundcolor','r')
           else    
               set(handles.txtmod,'String','communication failed','backgroundcolor','r')
           end
           
    elseif strcmp(ll,'off')
        Input=('A2DO          ');
        Input=char(Input);
        set(handles.txtLaser_Res,'String',Input);
        fprintf(tport,Input); 
        pause(1);
        Reset=tport.UserData;
        set(handles.txtLaser_Res,'String',Reset);
        Reset=char(Reset);
        tport.UserData=[];
           if strcmp(Reset,'A2D ON'); %Laser now conncected
               set(handles.txtmod,'String','modulation enabled','backgroundcolor','g')
               set(handles.rb_analogue_on,'backgroundcolor','g');
           elseif strcmp(Reset,'UK'); %error Temperature too low
               set(handles.txtmod,'String','unknown error','backgroundcolor','r')
           else    
               set(handles.txtmod,'String','communication failed','backgroundcolor','r')
           end
    else
        set(handles.txtpp,'String','connect to laser first','backgroundcolor','r')
    end        
    
else
    if strcmp(ll,'on')
        Input=('A2DF ');
        %set(handles.txtmod,'String',ll);
        Input=char(Input);
        set(handles.txtLaser_Res,'String',Input);
        fprintf(tport,Input); 
        pause(1);
        Reset=tport.UserData;
        set(handles.txtLaser_Res,'String',Reset);
        Reset=char(Reset);
        tport.UserData=[];
           if strcmp(Reset,'A2D OFF'); %Laser now conncected
               set(handles.txtmod,'String','modulation disabled','backgroundcolor','r')
               set(handles.rb_analogue_on,'backgroundcolor','r');
           elseif strcmp(Reset,'UK'); %error Temperature too low
               set(handles.txtmod,'String','unknown error','backgroundcolor','r')
           else    
               set(handles.txtmod,'String','communication failed','backgroundcolor','r')
           end
    elseif strcmp(ll,'off')
        Input=('A2DF          ');
        Input=char(Input);
        set(handles.txtLaser_Res,'String',Input);
        fprintf(tport,Input); 
        pause(1);
        Reset=tport.UserData;
        set(handles.txtLaser_Res,'String',Reset);
        Reset=char(Reset);
        tport.UserData=[];
           if strcmp(Reset,'A2D OFF'); %Laser now conncected
               set(handles.txtmod,'String','modulation disabled','backgroundcolor','r')
               set(handles.rb_analogue_on,'backgroundcolor','r');
           elseif strcmp(Reset,'UK'); %error Temperature too low
               set(handles.txtmod,'String','unknown error','backgroundcolor','r')
           else    
               set(handles.txtmod,'String','communication failed','backgroundcolor','r')
           end
    else
        set(handles.txtpp,'String','connect to laser first','backgroundcolor','r')
    end      
end
    

% --- Executes on button press in rb_pp_on.
function rb_pp_on_Callback(hObject, eventdata, handles)
% hObject    handle to rb_pp_on (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of rb_pp_on

tport=handles.tport;
ll=handles.ll;

if get(hObject,'Value')
    if strcmp(ll,'on')
        Input=('IPO  ');
        Input=char(Input);
        set(handles.txtLaser_Res,'String',Input);
        fprintf(tport,Input); 
        pause(1);
        Reset=tport.UserData;
        set(handles.txtLaser_Res,'String',Reset);
        Reset=char(Reset);
        tport.UserData=[];
           if strcmp(Reset,'IPO'); %Laser now conncected
               set(handles.txtpp,'String','peakpower enabled','backgroundcolor','g')
               set(handles.rb_pp_on,'backgroundcolor','g');
           elseif strcmp(Reset,'UK'); %error Temperature too low
               set(handles.txtpp,'String','unknown error','backgroundcolor','r')
               set(handles.rb_pp_on,'backgroundcolor','r');
           else    
               set(handles.txtpp,'String','communication failed','backgroundcolor','r')
               set(handles.rb_pp_on,'backgroundcolor','r');
           end
      
     elseif strcmp(ll,'off')
        Input=('IPO           ');
        Input=char(Input);
        set(handles.txtLaser_Res,'String',Input);
        fprintf(tport,Input); 
        pause(1);
        Reset=tport.UserData;
        set(handles.txtLaser_Res,'String',Reset);
        Reset=char(Reset);
        tport.UserData=[];
           if strcmp(Reset,'IPO'); 
               set(handles.txtpp,'String','peakpower enabled','backgroundcolor','g')
               set(handles.rb_pp_on,'backgroundcolor','g');
           elseif strcmp(Reset,'UK'); 
               set(handles.txtpp,'String','unknown error','backgroundcolor','r')
               set(handles.rb_pp_on,'backgroundcolor','r');
           else    
               set(handles.txtpp,'String','communication failed','backgroundcolor','r')
               set(handles.rb_pp_on,'backgroundcolor','r');
           end
    else
        set(handles.txtpp,'String','connect to laser first','backgroundcolor','r')
    end

else
   if strcmp(ll,'on')
        Input=('IPF  ');
        Input=char(Input);
        set(handles.txtLaser_Res,'String',Input);
        fprintf(tport,Input); 
        pause(1);
        Reset=tport.UserData;
        set(handles.txtLaser_Res,'String',Reset);
        Reset=char(Reset);
        tport.UserData=[];
           if strcmp(Reset,'IPO'); 
               set(handles.txtpp,'String','peakpower disabled','backgroundcolor','r')
               set(handles.rb_pp_on,'backgroundcolor','r');
           elseif strcmp(Reset,'UK'); %error Temperature too low
               set(handles.txtpp,'String','unknown error','backgroundcolor','r')
               set(handles.rb_pp_on,'backgroundcolor','r');
           else    
               set(handles.txtpp,'String','communication failed','backgroundcolor','r')
               set(handles.rb_pp_on,'backgroundcolor','r');
           end
      
     elseif strcmp(ll,'off')
        Input=('IPF           ');
        Input=char(Input);
        set(handles.txtLaser_Res,'String',Input);
        fprintf(tport,Input); 
        pause(1);
        Reset=tport.UserData;
        set(handles.txtLaser_Res,'String',Reset);
        Reset=char(Reset);
        tport.UserData=[];
           if strcmp(Reset,'IPF'); 
               set(handles.txtpp,'String','peakpower disabled','backgroundcolor','r')
               set(handles.rb_pp_on,'backgroundcolor','r');
           elseif strcmp(Reset,'UK'); 
               set(handles.txtpp,'String','unknown error')
              
           else    
               set(handles.txtpp,'String','communication failed','backgroundcolor','r')
               
           end
    else
        set(handles.txtpp,'String','connect to laser first','backgroundcolor','r')
   end
end

function txtenter_val_pp_Callback(hObject, eventdata, handles)%input laser peak power
% hObject    handle to txtenter_val_pp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of txtenter_val_pp as text
%        str2double(get(hObject,'String')) returns contents of txtenter_val_pp as a double

ll=handles.ll;
tport=handles.tport;

pp=get(hObject,'String');
ppout=['peakpower ',pp,'%'];
set(handles.txtpp_value,'string',ppout);
try
        if strcmp ('max',pp)
            if strcmp (ll,'on')
                LaserCmd=('PPFFF');
            else
                LaserCmd=('PPFFF         ');
            end    
        end
    LaserCmd=char(LaserCmd);
    set(handles.txtLaser_Res,'String',LaserCmd);
    fprintf(tport,LaserCmd);
    pause(1);
    L_Answer=tport.UserData;
    L_Answer=char(L_Answer);
    tport.UserData=[];
    set(handles.txtLaser_Res,'String',L_Answer);

catch
    pp1=floor(str2double(pp));
    set(handles.txtLaser_Res,'String',pp1);
    if pp1>=0 & pp1 <= 100
            %pp1=floor(str2double(pp));
            pp1=pp1*40;
            pphex=dec2hex(pp1,3);
            a='PP';
                if strcmp(ll,'on')
                    LaserCmd=[a,pphex];
                else
                    LaserCmd=[a,pphex,'         '];
                end    
            LaserCmd=char(LaserCmd);
            set(handles.txtLaser_Res,'String',LaserCmd);
            fprintf(tport,LaserCmd);
            pause(1);
            L_Answer=tport.UserData;
            L_Answer=char(L_Answer);
            tport.UserData=[];
            set(handles.txtLaser_Res,'String',L_Answer);
    else
        set(handles.txtLaser_Res,'String','Input must be integer between 0-100 or max');
    end    
end

% --- Executes during object creation, after setting all properties.
function txtenter_val_pp_CreateFcn(hObject, eventdata, handles)
% hObject    handle to txtenter_val_pp (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
%if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
%    set(hObject,'BackgroundColor','white');
%end


    

    


% --- Executes on button press in pb_status.
function pb_status_Callback(hObject, eventdata, handles)
% hObject    handle to pb_status (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
tport=handles.tport;
ll=handles.ll;
if strcmp (ll,'on')
    LaserCmd='STAT1';
elseif strcmp (ll,'off')
    LaserCmd =('STAT1         ');
else
    set(handles.txtlaserparameter,'String','Connect to laser first');
end
LaserCmd=char(LaserCmd);
set(handles.txtLaser_Res,'String',LaserCmd);
fprintf(tport,LaserCmd);
pause(1);
L_Answer=tport.UserData;
L_Answer=char(L_Answer);
tport.UserData=[];
set(handles.txtlaserparameter,'String',L_Answer,'backgroundcolor','c');


% --- Executes on button press in rb_fullpower.
function rb_fullpower_Callback(hObject, eventdata, handles)
% hObject    handle to rb_fullpower (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of rb_fullpower
tport=handles.tport;
ll=handles.ll;

if get(hObject,'Value')
    if strcmp(ll,'on')
        Input=('MO3  ');
        Input=char(Input);
        set(handles.txtLaser_Res,'String',Input);
        fprintf(tport,Input); 
        pause(1);
        Reset=tport.UserData;
        set(handles.txtLaser_Res,'String',Reset);
        Reset=char(Reset);
        tport.UserData=[];
           if strcmp(Reset,'MO3'); %Laser now conncected
               set(handles.txtfullpower,'String','Lasing enabled','backgroundcolor','g')
               set(handles.rb_fullpower,'backgroundcolor','g');
           elseif strcmp(Reset,'UK'); %error Temperature too low
               set(handles.txtfullpower,'String','unknown error','backgroundcolor','r')
           else    
               set(handles.txtfullpower,'String','communication failed','backgroundcolor','r')
           end
           
    elseif strcmp(ll,'off')
        Input=('MO3           ');
        Input=char(Input);
        set(handles.txtLaser_Res,'String',Input);
        fprintf(tport,Input); 
        pause(1);
        Reset=tport.UserData;
        set(handles.txtLaser_Res,'String',Reset);
        Reset=char(Reset);
        tport.UserData=[];
           if strcmp(Reset,'MO3'); %Laser now conncected
               set(handles.txtfullpower,'String','Lasing enabled','backgroundcolor','g')
               set(handles.rb_fullpower,'backgroundcolor','g');
           elseif strcmp(Reset,'UK'); %error Temperature too low
               set(handles.txtfullpower,'String','unknown error','backgroundcolor','r')
           else    
               set(handles.txtfullpower,'String','communication failed','backgroundcolor','r')
           end
    else
        set(handles.txtfullpower,'String','connect to laser first','backgroundcolor','r')
    end        
    
else
    if strcmp(ll,'on')
        Input=('MO1  ');
        %set(handles.txtmod,'String',ll);
        Input=char(Input);
        set(handles.txtLaser_Res,'String',Input);
        fprintf(tport,Input); 
        pause(1);
        Reset=tport.UserData;
        set(handles.txtLaser_Res,'String',Reset);
        Reset=char(Reset);
        tport.UserData=[];
           if strcmp(Reset,'MO1'); %Laser now conncected
               set(handles.txtfullpower,'String','Lasing disabled','backgroundcolor','r')
               set(handles.rb_fullpower,'backgroundcolor','r');
           elseif strcmp(Reset,'UK'); %error Temperature too low
               set(handles.txtfullpower,'String','unknown error','backgroundcolor','r')
           else    
               set(handles.txtfullpower,'String','communication failed','backgroundcolor','r')
           end
    elseif strcmp(ll,'off')
        Input=('MO1           ');
        Input=char(Input);
        set(handles.txtLaser_Res,'String',Input);
        fprintf(tport,Input); 
        pause(1);
        Reset=tport.UserData;
        set(handles.txtLaser_Res,'String',Reset);
        Reset=char(Reset);
        tport.UserData=[];
           if strcmp(Reset,'MO1'); %Laser now conncected
               set(handles.txtfullpower,'String','Lasing disabled','backgroundcolor','r')
               set(handles.rb_fullpower,'backgroundcolor','r');
           elseif strcmp(Reset,'UK'); %error Temperature too low
               set(handles.txtfullpower,'String','unknown error','backgroundcolor','r')
           else    
               set(handles.txtfullpower,'String','communication failed','backgroundcolor','r')
           end
    else
        set(handles.txtfullpower,'String','connect to laser first','backgroundcolor','r')
    end      
end

