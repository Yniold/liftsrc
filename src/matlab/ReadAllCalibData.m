%
%
% $ID: $
%
% $Log: ReadAllCalibData.m,v $
% Revision 1.1  2007-06-11 15:50:34  rudolf
% added helper scripts to read all datasets in one directory in one huge structure
%
%
%
% ******************************************************** %
% read all .CAL files in current directory into data array %
% ******************************************************** %

function CalibData = ReadAllCalibData()
myfiles = dir;
iCounter = 1;
[numfiles,dummy] = size(myfiles);

% filter out .m files etc, only files with .CAL in name wanted
for iLoop=1:numfiles
    if((myfiles(iLoop).isdir == false) && (~isempty(findstr(myfiles(iLoop).name,'.cal'))))
        filename{iCounter} = {myfiles(iLoop).name};
        iCounter = iCounter+1;
    end
end
[dummy,iCounter] = size(filename);

if(iCounter == 1)
    CalibData = ReadCalibData(char(filename{1}));
else

    % read first dataset to have the proper structures set
    CalibData = ReadCalibData(char(filename{1}));
    CurrentDataSet = struct;

    % import all data
    for iLoop=2:iCounter
        CurrentDataSet = ReadCalibData(char(filename{iLoop}));
        CalibData = concat(CurrentDataSet,CalibData);
    end
end

% *********************** %
% SUBFUNCTIONS BELOW HERE %
% *********************** %


% function to concetenate two identical structures to a single large structure%
function concatedstruct = concat(a,b)
    result = struct(a);
    concatedstruct = copy_element(a,b,result);

% recursively iterate through the structure to the first non-struct element %

function result = copy_element(a,b,result)
items = fieldnames(a);
numitems = length(items);

for iLoop=1:numitems
    if(isstruct(a.(items{iLoop})))
%       disp(a.(items{iLoop}));%
       result.(items{iLoop}) = copy_element(a.(items{iLoop}),b.(items{iLoop}),result.(items{iLoop}));
    else
       result.(items{iLoop}) = [b.(items{iLoop}); a.(items{iLoop})];
    end
end
