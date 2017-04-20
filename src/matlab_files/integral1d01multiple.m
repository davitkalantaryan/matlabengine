function [ ret_value ] = integral1d01multiple( a_functionName,a_left, a_right, a_dx )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

left1=a_left;
%right1=left1+(a_right-a_left)/4;
right1 = left1 + 0*(a_right-a_left);

left2=right1;
right2=left2+0.5*(a_right-a_left);

left3=right2;
right3=a_right;

%remote_call('znnomos41','--timeout','no-wait'); % -10 means no-wait
remote_call('znanser0','--timeout',-10); % -10 means no-wait

tic

%ans12=remote_call('znnomos41','integral1d01single',a_functionName,left1,right1,a_dx);
ans12=remote_call('znanser0','integral1d01single',a_functionName,left2,right2,a_dx);
sum3=integral1d01single( a_functionName,left3, right3, a_dx );
%sum1=remote_call('znnomos41','--receive');
sum1=0;
sum2=remote_call('znanser0','--receive');

ret_value=sum1+sum2+sum3;

toc

end

