# Cppmp3
Split up visualization,mp3player and gui completely
</br>
For Audio Playing/Decoding: https://github.com/mackron/miniaudio
</br>
For Visualization: raylib/Opengl
</br>
For Playlists: nlohmannJson
</br>
For fft: fftw3


## How a playlist looks like:
```json
{
"name":"PLAYLIST_NAME",
"dir":"ABSOLUTE_PATH_TO_THE_FOLDER",
"songs":[
{
    "songname":"NAME_OF_SONG",
    "path":"FILENAME"
},
{
    "songname":"science-documentary",
    "path":"TestMp3s/science-documentary.mp3"
},
{
    "songname":"once-in-paris",
    "path":"TestMp3s/once-in-paris.mp3"
},
{
    "songname":"titanium",
    "path":"TestMp3s/titanium.mp3"
}
]

}
```
