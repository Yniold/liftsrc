function tcpipdatacallback(obj, event)
% reads in commands received in tcpip and writes them into handles assigned
% to the command numbers
%tcpcommand=fscanf(obj); % read in command
tcpcommand=fgetl(obj); % read in command
set(obj,'UserData',[obj.UserData,tcpcommand]);
%disp(obj.UserData)






