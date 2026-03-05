%Main code
function [hdr, audio] = testAudio(filepath,varargin)

    if (length(varargin) < 2)
        disp('Usage: [hdr, audio] = testAudio(filepath[string], plot-audio[bool], play-audio[bool])');
        return;
    end
    
    plots = varargin{1};
    plays = varargin{2};

    [audio,fs] = audioread(filepath);
    hdr = audioinfo(filepath);

    if plots
        t = (0:length(audio)-1)/fs;
        figure;
        plot(t,audio);
        xlabel('Time (s)');
        ylabel('Amplitude');
        title('Audio Signal');
    end

    if plays
        sound(audio, fs);
    end
end


