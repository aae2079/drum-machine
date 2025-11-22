%Main code
filepath = '/Users/aaron_escbr/workspace/gitRepos/drum-machine/test/output_audio.bin';
fid = fopen(filepath,"rb");
audio = fread(fid, 'float32');
fclose(fid);
figure;
plot(audio);

% run audio
sound(audio, 48000);

