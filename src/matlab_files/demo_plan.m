
function demo_plan
% DEMO PLAN
% 1. Open new MATLAB 
%      a) %init_root_and_call matlab_R2016a&
% 2. Show the image (znnomos41)
%      a) >>cd W:/matlabengine/src/matlab_files
%      b) >>camNames=webcamlist
%      c) >>cam=webcam(camNames{1})
%      d) >>img = snapshot(cam);
%      e) >>imshow(img)
%      f) >>clear cam
% 2. integral calculation using remote MATLAB (znnomos41)
%      b) >>ans1=call_integral1d01single('sin',0,pi/2,0.000001)
%      c) >>ans1=integral1d01multiple('sin',0,pi/2,0.0000001)
% 3. device control test (wgs13)
%      a) >>disp1=wmtr_downstripe(100,50);
%      b) >>remote_call('znella','slm',disp1);
%
%