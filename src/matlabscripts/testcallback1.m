function ret1=testcallback1(aIndex,vectorArgs)

fprintf(1,'aIndex=%d\n',aIndex);

ret1=vectorArgs(aIndex+1)*vectorArgs(aIndex+1);
