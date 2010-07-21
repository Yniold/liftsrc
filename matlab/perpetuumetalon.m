%perpetuumetalon
stepratio=2;
for i=1:1000
    setpos=0;
    encoderpos=str2double(get(handles.txtEtEncPos,'String'));
    setpos=floor((str2double(get(handles.set_pos,'String'))+currentpos-encoderpos)/stepratio);
    if setpos<0
        setpos=setpos+2^32/2;
        negpos=1;
    else
        negpos=0;
    end
    setposhex=dec2hex(floor(setpos));
    setposhex=[zeros(1,8-length(setposhex)),setposhex];
    setposlow=hex2dec(setposhex(5:8));
    setposhigh=hex2dec(setposhex(1:4));
    if negpos==1
        setposhigh=bitset(setposhigh,16,1);
    end

    system(['/lift/bin/eCmd @Lift s etalonnop']);    
    %set etalon acc and spd to 0
    system(['/lift/bin/eCmd @Lift w 0xa514 0']);
    %set position
    system(['/lift/bin/eCmd @Lift w 0xa512 ',num2str(setposhigh)]);
    system(['/lift/bin/eCmd @Lift w 0xa510 ',num2str(setposlow)]);
    %set etalon acc and spd to 20
    system(['/lift/bin/eCmd @Lift w 0xa514 0x2020']);
end


    
