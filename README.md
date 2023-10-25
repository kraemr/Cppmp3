Split up visualization,mp3player and gui completely

For Audio Playing/Decoding: https://github.com/mackron/miniaudio
For Visualization: raylib/Opengl
For Playlists: nlohmannJson
For fft: fftw3



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