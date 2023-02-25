#!/bin/sh

ffmpeg -framerate 30 -pattern_type glob -i 'frames/*.png' -c:v libx264 out.mp4
