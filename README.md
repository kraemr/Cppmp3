# glsl-Audio
uses glsl shaders for audio visualization and has  a builtin mp3-player, with playlists and all the things you would expect.
https://rajithsworkbench.wordpress.com/2013/11/21/discrete-fourier-transformation-dft-based-spectrum-tracer/
https://stackoverflow.com/questions/23532379/how-to-resolve-frequency-from-pcm-samples



For testing, in each sample draw concentric circles per frequency whose radius is the intensity and the color is frequency a.k.a polar graphic analyzer! – 
rmi
 May 8, 2014 at 17:33

## The Current State
Currently im mostly working on getting the mp3 player working properly with low cpu-usage, for that ill have to use a different library as libao is pretty outdated.
So This is not even close to finished.

## dependencies:
libfftw3,libao,libmpg123

```bash
sudo apt install libao-dev libfftw3-dev libmpg123-dev
```
