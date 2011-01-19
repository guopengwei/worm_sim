% Load the injection rates;
rateL; rateP;

% Normalize the injection rates
sim_len = 200000;
rL = rL/sim_len; rP = rP/sim_len; 

% Load the total communication energy values
total_energyP; total_energyL;

% plot the energy consumption before and after insertion of LRL
plot(rP,tEP,'--b');
hold on, grid
plot(rL,tEL,'r')
legend('4x4 Mesh network','4x4 Mesh network + long-range links')
