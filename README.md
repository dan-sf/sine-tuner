# Sine Tuner

A small program for tuning a guitar's open strings to pure sine wave tones.

<p align="center">
<img width="291" alt="sine_tuner_screen_shot" src="https://user-images.githubusercontent.com/5980346/96147268-00809780-0ebc-11eb-9dfb-614a64fdacc0.png">
<p/>

## Overview

This program runs a small UI application that consists of six different buttons
each corresponding to the open string notes on a guitar (in standard tuning).

Only a single tone plays at once. Clicking a button will cause that note's sine
tone to start playing. If you click the same button again it will mute the tone.
Clicking to other buttons will switch the tone.

The low E string tone is in the top row and high E string tone is in the bottom
row.

I cover the development process for this project on my site
[here](https://www.dsfcode.com/posts/sine-tuner-project/) if you're interested.

## Motivation

I usually tune my guitar by ear using a 440 hz tone. This is pretty common since
440 hz is a standard pitch for tuning many instruments. However, I wanted an
easy way to tune each of the open strings on a guitar by ear using pure sine
tones. Plus, it sounded like a fun project to dig deeper into audio processing.

Thus, this sine tuner program was born. Its pretty basic, but I learned a lot
building it. I don't have much UI programming experience, so going into this
project I decided to use [immediate
mode](https://en.wikipedia.org/wiki/Immediate_mode_GUI) style implementation for
the UI. It felt like a more natural approach and was easy enough to implement
myself.

## Building

The main dependency for this program is [SDL](https://www.libsdl.org/). I use
SDL to handle both graphics and audio processing. I also made use of the [stb
libraries](https://github.com/nothings/stb), specifically `stb_truetype.h` for
the font rendering. That header is included with this project so no need to
download it separately.

You'll need to install SDL if you don't already have it to build this project.
You can download SDL [here](https://www.libsdl.org/download-2.0.php). `gcc` or
`clang` is also needed to handle compilation. Once SDL is installed you can
build and run the program with the following:

```bash
git clone https://github.com/dan-sf/sine-tuner.git
cd sine-tuner
./build.sh
./sine_tuner
```

This program was built and tested on MacOS. It should theoretically work on
Windows and Linux if SDL is installed. However, this hasn't been tested so your
mileage may vary.

The lower tones can be difficult to hear on some speakers so headphones or
earbuds are best for clarity.

