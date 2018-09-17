apollo_record
=============

apollo_record is a monitoring program of loudness measurement for external
audio input source. The EBU R 128 standard is used for loudness measurement.
The website is https://github.com/lowasis/apollo_record/.

Features
--------

* Realtime momentary, shortterm, integrated loudness measurement.
* Pre-scheduled loudness measurement.
* Loudness measurement logging.
* Video and audio recording for non-realtime loudness measurement.
* Video and audio UDP streaming for preview.

Essential external library
--------------------------
apollo_record use some external libraries. The following libraries should be
pre-installed for building and running.

* libebur128 ~ Loudness measurement. [​https://github.com/jiixyj/libebur128/]
* FFmpeg ~ Video and audio recording. [https://www.ffmpeg.org/]
* lirc ~ External Set-top-box remote control. [http://www.lirc.org/]
* libftdi ~ USB type serial port driver for lirc.
            [https://www.intra2net.com/en/developer/libftdi/]

Build
-----

In the root folder, type:

    make

Installation
------------

In the root folder, type:

    sudo make install

Run
---

apollo_record consists of two daemon programs. One is 'apollo_monitor', and
the other is 'apollo_controller'. You should to run these daemon programs.
