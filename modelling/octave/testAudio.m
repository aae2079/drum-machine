%Main code
function [hdr, audio] =testAudio(filepath)

    [audio,fs] = audioread(filepath);
    hdr = audioinfo(filepath);
    %keyboard;
end


