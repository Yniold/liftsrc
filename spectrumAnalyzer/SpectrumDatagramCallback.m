function SpectrumDatagramCallback(Object,Event)

MyCallbackData = get(Object,'UserData');
[TheDataMatrix,Bytes,msg] = fread(Object,4096,'uint16');
if(Bytes > 0)

    TheDataMatrix = uint16(TheDataMatrix);

    % calculate packet number and max packets %
    sz = size(TheDataMatrix);
    MaximumYValue = TheDataMatrix(2);
    MyCallbackData.OldMaxPackets = MyCallbackData.MaxPackets;
    MyCallbackData.MaxPackets = bitshift(TheDataMatrix(1),-8);
    PacketNumber = bitand(uint16(TheDataMatrix(1)),uint16(255));
    DataOffset = TheDataMatrix(3)./2;
    BufferSize = TheDataMatrix(4);

    % check if new plot or packet style changed
    if(MyCallbackData.StartNewPlot == 1) || (MyCallbackData.MaxPackets ~= MyCallbackData.OldMaxPackets)
        MyCallbackData.PacketsSeen = zeros(MyCallbackData.MaxPackets,1);
        MyCallbackData.ThePlotBuffer = zeros(BufferSize/2,1);
        MyCallbackData.ThePlotBuffer = uint16(MyCallbackData.ThePlotBuffer);
        MyCallbackData.StartNewPlot = 0;
    end;

    MyCallbackData.PacketsSeen(PacketNumber) = 1;

    TheDataMatrix = TheDataMatrix(5:end,1); % cut metadata %
    MatrixSize = size(TheDataMatrix);
    DataSize = uint16(MatrixSize(:,1));
    EndOfTarget = uint16(DataOffset + DataSize);
    MyCallbackData.ThePlotBuffer(DataOffset+1:EndOfTarget,1) = TheDataMatrix(1:end,1);

    if(sum(MyCallbackData.PacketsSeen) == size(MyCallbackData.PacketsSeen,1))
        % processing %    
        WaveLen = zeros(size(MyCallbackData.ThePlotBuffer,1),1);
        PixelVal = zeros(size(MyCallbackData.ThePlotBuffer,1),1);

        size(MyCallbackData.ThePlotBuffer);
        ReshapedMatrix = reshape(MyCallbackData.ThePlotBuffer,2,(size(MyCallbackData.ThePlotBuffer,1))*(size(MyCallbackData.ThePlotBuffer,2)/2))';    
        WaveLen = double(ReshapedMatrix(:,1));
        WaveLen = WaveLen./50;
        PixelVal = ReshapedMatrix(:,2);

        % plot data %
        plot(MyCallbackData.handles.TheSpectrum,WaveLen,PixelVal);
        xlim(MyCallbackData.handles.TheSpectrum,[min(WaveLen) max(WaveLen)]);
        ylim(MyCallbackData.handles.TheSpectrum,[0 MaximumYValue]);
        MyCallbackData.StartNewPlot = 1;
    end;
end;
set(Object,'UserData',MyCallbackData);
