function [ ret1 ] = test_multiengine2(numberOfThreads,a_from,a_to,a_step)
%
% example of usage:
% >>ret=square_sum(4,[1 2 3 4 5 6 7]); % here first argument 
%                                       is number of threads
%


aLength=a_to-a_from;
nIterations=floor(aLength/a_step);
if(numberOfThreads>nIterations)
    numberOfThreads=nIterations;
end

if(numberOfThreads<1)
    fprintf(2,'numberOfThreads should be greater than 0\n');
    ret1=0;
    return;
end

numberOfThreadsMin1 = numberOfThreads-1;
perThreadReg=aLength/numberOfThreads;

fprintf(1,'perThreadReg:%d\n',perThreadReg);

%ret1=multiengine(@testcallback1,nLengthMin1+1,'--replace-by-index',vectorArgs);
ret1=multiengine(@MyTestCallback,numberOfThreads,'--replace-by-index', ...
    a_from, a_to, a_step,numberOfThreads,perThreadReg);
if (isempty(ret1))
    ret1=0;
end


for p1=0:numberOfThreadsMin1
    retByEngine=multiengine('--output-from-engine',p1);
    fprintf(1,'eng:%d, ret=%d\n',p1,retByEngine);
    ret1 = ret1 + retByEngine;
end


end


%% Function as a callback
function a_ret=MyTestCallback(a_threadIndex,a_from, a_to, a_step, ...
    a_numberOfThreads,a_perThreadReg)

if(a_threadIndex ~=0 )
    a_from=a_from+a_threadIndex*a_perThreadReg+a_step;
end
if(a_threadIndex~=(a_numberOfThreads-1))
    a_to=aFrom+a_perThreadReg;
end

a_ret=0;

for p1=a_from:a_step:a_to
    a_ret = a_ret + p1*p1;
end
    
end
