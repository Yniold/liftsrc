function serialdatacallback(obj, event)
% reads in commands received in tcpip and writes them into handles assigned
% to the command numbers
serialcommand=fscanf(obj); % read in command
set(obj,'UserData',serialcommand);
%disp(obj.Userdata)






