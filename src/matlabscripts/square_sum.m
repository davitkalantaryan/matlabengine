function [ ret1 ] = square_sum(numberOfThreads,vectorArgs)
%
% example of usage:
% >>ret=square_sum(4,[1 2 3 4 5 6 7]); % here first argument 
%                                       is number of threads
%

nLength=length(vectorArgs);
if(numberOfThreads>nLength)
    numberOfThreads=nLength;
end

if(numberOfThreads<1)
    fprintf(2,'numberOfThreads should be greater than 0\n');
    ret1=0;
    return;
end

numberOfThreadsMin1 = numberOfThreads-1;
perThread=floor(nLength/numberOfThreads);

fprintf(1,'perThread:%d\n',perThread);

%ret1=multiengine(@testcallback1,nLengthMin1+1,'--replace-by-index',vectorArgs);
ret1=multiengine(@MyTestCallback,numberOfThreads,'--replace-by-index',vectorArgs,numberOfThreads,nLength,perThread);
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
function a_ret=MyTestCallback(a_threadIndex,a_vectorArgs,a_numberOfThreads,a_nLengt,a_perThread)

aIndex1=a_threadIndex*a_perThread+1;
if(a_threadIndex==(a_numberOfThreads-1))
    aIndex2=a_nLengt;
else
    aIndex2=aIndex1+a_perThread-1;
end

a_ret=0;

for p1=aIndex1:aIndex2
    a_ret = a_ret + ...
        a_vectorArgs(p1)* ...
        a_vectorArgs(p1);
end
    
end
