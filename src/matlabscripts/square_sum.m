function [ ret1 ] = square_sum(vectorArgs)
%UNTITLED2 Summary of this function goes here
%   Detailed explanation goes here

nLengthMin1 = length(vectorArgs)-1;

ret1=multiengine(@testcallback1,nLengthMin1+1,'--replace-by-index',vectorArgs);
if (isempty(ret1))
    ret1=0;
end

fprintf('length=%d\n',nLengthMin1+1);

for p1=0:nLengthMin1
    retByEngine=multiengine('--output-from-engine',p1);
    ret1 = ret1 + retByEngine;
end


end

