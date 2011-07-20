%{ 
$RCSfile: PlotSpectra.m,v $ last changed on $Date: 2007-02-18 21:04:47 $ by $Author: rudolf $ 
Standalone Application for displaying of spectrometer UDP data

$Log: PlotSpectra.m,v $
Revision 1.2  2007-02-18 21:04:47  rudolf
corrected function names to avoid warning

Revision 1.1  2007-02-18 20:59:05  rudolf
initial revision
 

%}


% kill any existing open socket %
OldSockets = instrfind;
delete(OldSockets);

% create a new UDP socket %
MySocket = udp('127.0.0.1');
MySocket.LocalPort = 4711;
MySocket.InputBufferSize = 8192;
MySocket.ByteOrder = 'littleEndian';
fopen(MySocket);
set(MySocket,'Timeout',1000);
figure;
ylim([0 16384]);

% init packet sequencing logic %
StartNewPlot = 1;
MaxPackets = 0;
OldMaxPackets = 0;

% main loop %
while(1)
    [TheDataMatrix,Bytes] = fread(MySocket,[2 4096],'uint16');
    if(Bytes > 0)
        if(Bytes~=8192) && (Bytes~=6424) && (Bytes~=16)
        TheDataMatrix = uint16(TheDataMatrix);

        % calculate packet number and max packets %
        sz = size(TheDataMatrix);
        MaximumYValue = TheDataMatrix(2);
        OldMaxPackets = MaxPackets;
        MaxPackets = bitshift(TheDataMatrix(1),-8);
        PacketNumber = bitand(uint16(TheDataMatrix(1)),uint16(255));
        DataOffset = TheDataMatrix(3)./2;
        BufferSize = TheDataMatrix(4);

        % check if new plot or packet style changed
        if(StartNewPlot == 1) || (MaxPackets ~= OldMaxPackets)
            PacketsSeen = zeros(MaxPackets,1);
            ThePlotBuffer = zeros(BufferSize/2,1);
            ThePlotBuffer = uint16(ThePlotBuffer);
            StartNewPlot = 0;
        end;

        PacketsSeen(PacketNumber) = 1;

        TheDataMatrix = TheDataMatrix(5:end,1); % cut metadata %
        MatrixSize = size(TheDataMatrix);
        DataSize = uint16(MatrixSize(:,1));
        EndOfTarget = uint16(DataOffset + DataSize);
        ThePlotBuffer(DataOffset+1:EndOfTarget,1) = TheDataMatrix(1:end,1);

        if(sum(PacketsSeen) == size(PacketsSeen,1))
            % processing %    
            WaveLen = zeros(size(ThePlotBuffer,1),1);
            PixelVal = zeros(size(ThePlotBuffer,1),1);

            ReshapedMatrix = reshape(ThePlotBuffer,2,(size(ThePlotBuffer,1))*(size(ThePlotBuffer,2)/2))';    
            WaveLen = double(ReshapedMatrix(:,1));
            WaveLen = WaveLen./50;
            PixelVal = ReshapedMatrix(:,2);

            % plot data %
            plot(WaveLen,PixelVal);
            xlim([min(WaveLen) max(WaveLen)]);
            ylim([0 MaximumYValue]);
            StartNewPlot = 1;
            pause(0.1);
        end;
        end;
    end;
end;

fclose(MySocket);
delete(MySocket);
