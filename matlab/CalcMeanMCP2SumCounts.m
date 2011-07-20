%find last 10 s in each etalon position

etaOnlinePos=int16(statusData(:,803));
etaCurPos=int16(statusData(:,645));
online=PMTSumCounts'>2500 & abs(etaCurPos-etaOnlinePos)<20;
offline1=PMTSumCounts'<=2500 & (etaCurPos-etaOnlinePos)==-600;
offline2=PMTSumCounts'<=2500 & (etaCurPos-etaOnlinePos)==600; 
for i=85:size(iZeit,1)
if online(iZeit(i))==1
    MeanMCP2SumCounts(iZeit(i))=mean(MCP2SumCounts(online(iZeit(i-(9*5)):iZeit(i))==1));
elseif offline1(iZeit(i))==1
    MeanMCP2SumCounts(iZeit(i))=mean(MCP2SumCounts(offline1(iZeit(i-(19*5)):iZeit(i))==1));
elseif offline2(iZeit(i))==1
    MeanMCP2SumCounts(iZeit(i))=mean(MCP2SumCounts(offline2(iZeit(i-(19*5)):iZeit(i))==1));
else
    MeanMCP2SumCounts(iZeit(i))=NaN;
end
if size(MeanMCP2SumCounts)<i MeanMCP2SumCounts(i)=NaN; end
end