% script to load all binary files in the current directory 
% and store them as one merge file

clear all;
    d=dir('5160*.bin');
    start=1;
    %allocate memory space
    %Time=ones(20000,1); 
    %Time(:)=NaN;
    %LiftData=ones(20000,1000);
    %LiftData(:,:)=NaN;
    % loop through all binary files for day jd
    k=1;
    for i=(start):size(d,1)
        d(i).name %print out bin file being processed
        try
            [x,y]=ReadDataAvg(d(i).name,25,100); % make averages, return average data y and Time x            
            %LiftData(k:k+size(x,1),:)=x;
            %LiftData=[LiftData,x];
            %k=k+size(x,1);
        catch
        end
        try
           LiftData=[LiftData;x];
        catch
            LiftData=x;
        end

    end
 