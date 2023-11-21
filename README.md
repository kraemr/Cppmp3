# Cppmp3
A Simple mp3player api for C++ with basic json playlists and configuration.
With some cool examples and a fully functional mp3player with Opengl Shader visualization.

## Building
´´´bash
cd build
make
´´´

## playlist.json
```json
{
    "name":"playlist_name",
    "songs":[
        {"songname":"some custom name","path":"some/absolutepath"},{"songname":"some custom name","path":"$SOME_ENV_VARIABLE"}
    ]
}
```


## settings.json
the gui mp3player takes this file to configure itself.
playlist_dirs and music_dirs currently only take absolute paths, but later they will get env variable parsing.
So something like "music_dirs":["$HOME/Music"] would lead to /home/user/Music on linux.
```json
{
    "playlist_dirs":[

    ],
    "music_dirs":[

    ],
    "nanodelay":10000000,
    "fps":60,
    "initial-screen-size-x":800,
    "initial-screen-size-y":450,
    "start-maximized":true,
    "start-fullscreen":false
}
```