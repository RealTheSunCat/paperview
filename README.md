# PAPERVIEW-Celeste

This is a fork of Paperview aiming to imitate the following Wallpaper Engine wallpaper: https://steamcommunity.com/sharedfiles/filedetails/?l=latam&id=1439758130

![](screenshot.png)

## Build

    make # NOTE: SDL2 is required

## Single Monitor Use

    ./paperview background.bmp sprite.bmp

## Running Background Daemon

Append an (&) to a paperview command to have it run as a background process. Eg:

    ./paperview FOLDER SPEED &

To stop this backgroud process, use `killall`:

    killall paperview

## Performance

Running on a Thinkpad X230 from 2012 at 1920x1080 and 60fps with an integrated Intel GPU:

    intel_gpu_time ./paperview castle 5

    user: 1.904135s, sys: 0.357277s, elapsed: 100.458648s, CPU: 2.3%, GPU: 11.7%

## Known Issues

Picom, Compton (and possibly other compositors) seem to already write to the base root X11 window
which may overwrite the render done by paperview.
