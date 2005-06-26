function k_q = getq(tint,wmr)
% getq.m   Get quenching rate as function of internal temperature
%          and water mixing ratio supplied in wmr (mole fraction).
% getq takes as input the internal temperature 'tint' and returns
% the OH fluorescence quenching rate from an 80/20 mixture of N2
% and O2.  The coefficients a, b, c were derived from data supplied 
% by Heard (Faraday Trans., V93, 1997 for O2 and N2; private comm.
% for H2O.)
%
% quenching cross-sections were approximated linearly, of the form
%	sigma = -mT + n		T in Kelvins
%
% k_q(T) = sigma(T) * <v>
%
% <v> is the average thermal collision velocity [8RT/(pi*mu)]^1/2,
% where mu is the reduced mass of the collision.
%
% the individual quenching rates have the form
% 	k = aT^(1/2) - bT^(3/2) + c
% revised by I. Faloona (12/97)
% H2O coefficients revised by MM (1/02) according to [Bailey et al., Chem. Phys. Lett., 1999]

kB = 1.381e-19;

% N2

an2 = -1.668e-11;
bn2 = -1.731e-14;
cn2 = 2.313e-10;

% O2

ao2 = 1.008e-11;
bo2 = 1.655e-14;
co2 = 5.129e-11;

% H20

ah2o = -4.017e-10;
bh2o = -4.4686e-13;
ch2o = 5.3137e-09;		

% calculate quenching rate constants

kn2 = an2*tint.^(0.5) - bn2*tint.^(1.5) + cn2;
ko2 = ao2*tint.^(0.5) - bo2*tint.^(1.5) + co2;
kh2o = ah2o*tint.^(0.5) - bh2o*tint.^(1.5) + ch2o;


k_q = ((1-wmr).* (0.791*kn2 + 0.209*ko2) + wmr.*kh2o)./(kB*tint);
