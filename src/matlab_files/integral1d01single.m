function [ ret_value ] = integral1d01single( a_functionName,a_left, a_right, a_dx )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

nextX=a_left;
ret_value=0;

fprintf(1,'integral1d01single("%s",%f,%f)\n',a_functionName,a_left,a_right);

aFunction=str2func(a_functionName);

while (nextX<a_right)
    resFunc=aFunction(nextX);
    ret_value = ret_value + resFunc*a_dx;
    nextX = nextX + a_dx;
end


end

