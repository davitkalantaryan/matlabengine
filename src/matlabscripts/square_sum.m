function [ ret1 ] = square_sum(vectorArgs)
%UNTITLED2 Summary of this function goes here
%   Detailed explanation goes here

nLength = length(vectorArgs);

ret1=multiengine(@testcallback1,nLength,'--replace-by-index',vectorArgs);
if (isempty(ret1))
    disp('empty');
    ret1=0;
end

disp(nLength);

for p1=1:nLength
    retByEngine=multiengine('--last-output-from-engine',p1);
    ret1 = ret1 + retByEngine;
end


end

