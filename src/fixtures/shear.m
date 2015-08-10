#!/usr/bin/octave --silent

xrange = [0 2];
yrange = [0 1];

n = 40;

u = repmat([1:n]./n,n,1);
v = zeros(n);

save -mat-binary shear.mat xrange yrange u v;
