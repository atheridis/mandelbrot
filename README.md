Mandelbrot Image Renderer
=========================

Can render images or frames for a video render, which can then be
combined with ffmpeg.

Build
-----
Run `$ make` to build and `$ make install` to install.

To create the image, just run the command `$ mandelbrot`.
This will create the files `frames/frame_N.png` in your current working
directory of the mandelbrot set.

You can use `create_video.sh` to combine the frames to make a video.
