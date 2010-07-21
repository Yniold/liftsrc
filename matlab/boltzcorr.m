function x=boltzcorr(Tcal,CELLT)
% MM, 6/2005
% adapted from Script 'boltz2',
% performs the necessary calculation of the variation of 
% sensitivity with detection cell temperature.  
% THIS FUNCTION IS ONLY APPLICABLE TO USE OF THE Q1(2) ABSORPTION BAND OF OH.
% For other lines output has to be redefined.
% The correction is made by first calculating the rotational partition
% function, but only summing over the PI_3/2 and PI_1/2 manifolds not
% including the additional lambda-splitting levels.  Thus the rotational
% partition functions may seem small, but it is the ratio of different
% temperature's functions that is crucial to this correction.  The algorithm
% presented below was found in a paper by Dieke and Crosswhite in the
% Journal of Quantitative Spectroscopy and Radiative Transfer, 'The Ultra-
% violet Bands of OH - Fundamental Data' v2 p97 (1962).  All constants
% used herein are from that paper. 

couplc = 87.17;    % unitless coupling constant of OH 2PI term
                   % (electron spin with nuclear rotation)
		   % (=a(a-4) in D&C paper.)
%
B = 18.515;
D = 0.00187;       % units are in cm^-1 (wavenumber)
%
k_B = 0.69504;     % Boltzmann constant in units of cm^-1/K     
 
% Calculate the term values for the first 20 rotational states of
% the PI_3/2 (lower state,"rotE(:,1)") and the PI_1/2 (rotE(:,2)) terms:
% K = the angular momentum quantum no. when electron spin is disregarded

for K=0:20
  rotE(K+1,1)=B*((K+1)^2 -1 -.5*(4*(K+1)^2 + couplc)^.5) - D*K^2*(K+1)^2;
  rotE(K+1,2)=B*(K^2 -1 +.5*(4*K^2 + couplc)^.5) - D*K^2*(K+1)^2;
end

% Calculate the Rotational Partition Function at both temperatures 
% (lab and in-field):  Q_rot(1)=lab, Q_rot(2)=field.

Q_rot(1:2)=[0.0 0.0];

% Sum over f_1 and f_2 states (i.e., OMEGA=+/-1/2, OMEGA=+/-3/2)
% (All referenced to the lowest PI_3/2 state (rotE(1,1))

for K=0:19
   Q_rot(1)=Q_rot(1)+(2*(K+.5)+1)*exp((rotE(1,1)-rotE(K+1,1))/(k_B*Tcal));
   Q_rot(1)=Q_rot(1)+(2*(K-.5)+1)*exp((rotE(1,1)-rotE(K+1,2))/(k_B*Tcal));

   Q_rot(2)=Q_rot(2)+(2*(K+.5)+1)*exp((rotE(1,1)-rotE(K+1,1))/(k_B*CELLT));
   Q_rot(2)=Q_rot(2)+(2*(K-.5)+1)*exp((rotE(1,1)-rotE(K+1,2))/(k_B*CELLT));
end

% Calculate populations of each state for lab & field temperatures:

for P=0:19
n_labp1(P+1)=(2*(P+.5)+1)*exp((rotE(1,1)-rotE(P+1,1))/(k_B*Tcal))/Q_rot(1)*100;
n_labp2(P+1)=(2*(P-.5)+1)*exp((rotE(1,1)-rotE(P+1,2))/(k_B*Tcal))/Q_rot(1)*100;
n_fltp1(P+1)=(2*(P+.5)+1)*exp((rotE(1,1)-rotE(P+1,1))/(k_B*CELLT))/Q_rot(2)*100;
n_fltp2(P+1)=(2*(P-.5)+1)*exp((rotE(1,1)-rotE(P+1,2))/(k_B*CELLT))/Q_rot(2)*100;
Kind(P+1)=P;
end

% Corrections factors for the three most frequently used OH lines

P11_x= (n_fltp1(1)+n_fltp2(1))/(n_labp1(1)+n_labp2(1));

Q12_x= (n_fltp1(2)+n_fltp2(2))/(n_labp1(2)+n_labp2(2));

Q13_x= (n_fltp1(3)+n_fltp2(3))/(n_labp1(3)+n_labp2(3));

Q14_x= (n_fltp1(4)+n_fltp2(4))/(n_labp1(4)+n_labp2(4));

x=Q12_x;