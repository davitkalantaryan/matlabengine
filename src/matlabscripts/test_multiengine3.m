function [ ret2 ] = test_multiengine3(numberOfThreads,a_from,a_to,a_step)
%
% example of usage:
% >>ret=square_sum(4,[1 2 3 4 5 6 7]); % here first argument 
%                                       is number of threads
%

localTic=tic;

aLength=a_to-a_from;
nIterations=floor(aLength/a_step);
if(numberOfThreads>nIterations)
    numberOfThreads=nIterations;
end

ret2=0;

if(numberOfThreads<1)
    fprintf(2,'numberOfThreads should be greater than 0\n');
    return;
end

numberOfThreadsMin1 = numberOfThreads-1;
perThreadReg=aLength/numberOfThreads;

fprintf(1,'iterations=%d,perThreadReg:%d\n',nIterations,perThreadReg);

%ret1=multiengine(@testcallback1,nLengthMin1+1,'--replace-by-index',vectorArgs);
ret1=multiengine(@MyTestCallback,numberOfThreads,'--replace-by-index', ...
    a_from, a_to, a_step,numberOfThreads,perThreadReg);

toc(localTic);
assignin('base','ret1',ret1);

for p1=0:numberOfThreadsMin1
    %retByEngine=multiengine('--output-from-engine',p1);
    retByEngine = ret1{p1+1};
    %fprintf(1,'eng:%d, ret=%d\n',p1,ret1{p1+1});
    ret2 = ret2 + retByEngine;
end

fprintf(1,'ret=%f\n',ret2);

end


%% Function as a callback
function a_ret=MyTestCallback(a_threadIndex,a_from, a_to, a_step, ...
    a_numberOfThreads,a_perThreadReg)

if(a_threadIndex ~=0 )
    a_from=double(a_from+a_threadIndex*a_perThreadReg+a_step);
end
if(a_threadIndex~=(a_numberOfThreads-1))
    a_to=a_from+a_perThreadReg;
end

a_ret=0;

for p1=a_from:a_step:a_to
    a_ret = a_ret + p1*p1;
end
    
end
