function [ ret_img ] = get_one_image_from_cam( varargin )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

camNames=webcamlist;

if(~isempty(camNames))
    try
        cam=webcam(camNames{1});
        ret_img = snapshot(cam);
    catch aExcpt
        ret_img=aExcpt.message;
    end
else
    ret_img='No camera attached!';
end


end

