# -*- octave -*-

xrange = [0 2];
yrange = [0 1];

u = zeros(128);
v = ones(128);

save -mat-binary shear.mat xrange yrange u v;
