Sine Tuner
==========

This project is a sine wave based guitar tuner. I usually tune my guitar by ear
using a 440 hz tone. This is pretty common since 440 hz is a standard pitch for
tuning many instruments. However, I wanted an easy way to tune each of the open
strings on a guitar (in standard tuning) by ear using a pure tone.

Thus this sine tuner program was born. Its pretty basic but I learned a lot
building it. It's a small UI program with six buttons that each correspond to
the open note strings on a guitar:

<img width="291" alt="sine_tuner_screen_shot" src="https://user-images.githubusercontent.com/5980346/96147268-00809780-0ebc-11eb-9dfb-614a64fdacc0.png">

Low E in the top row and high E in the bottom.

You can click a button and the related sine wave will play. If you click the
same button, it will turn off that note. Only a single note can be played at
once so clicking to other notes will just switch the tone.

The main dependency for this program is [SDL](https://www.libsdl.org/). I use
SDL to handle both graphics and audio processing. I also made use of the [stb
libraries](https://github.com/nothings/stb), specifically `stb_truetype.h` for
the font rendering. The UI interface is based on immediate mode style UI
processing.

Building
--------

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

