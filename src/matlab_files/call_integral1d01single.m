function [ ret_value ] = call_integral1d01single( a_function,a_left, a_right, a_dx )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
% example of usage: call_integral1d01single('sin',0,pi,0.00001)

tic

ret_value=integral1d01single( a_function,a_left, a_right, a_dx );

toc


end

