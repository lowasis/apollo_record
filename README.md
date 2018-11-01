apollo_record
=============

apollo_record is a monitoring program of loudness measurement for external
audio input source. The EBU R 128 standard is used for loudness measurement.
The source repository is https://github.com/lowasis/apollo_record/ and the web
site is https://cafe.naver.com/apolloopenproject/.

Features
--------

* Realtime momentary, shortterm, integrated loudness measurement.
* Pre-scheduled loudness measurement.
* Loudness measurement logging.
* Video and audio recording for non-realtime loudness measurement.
* Video and audio UDP streaming for preview.
* Channel changing of set-top-box for pre-scheduled loudness measurement.

Essential external library
--------------------------
apollo_record use some external libraries. The following libraries should be pre-installed as SHARED LIBRARY for building and running apollo_record.

* libebur128 ~ Loudness measurement. [​https://github.com/jiixyj/libebur128/]
* FFmpeg ~ Video and audio recording. [https://www.ffmpeg.org/]
* lame ~ MP3 encoder for FFmpeg. [http://lame.sourceforge.net/]
* lirc ~ External Set-top-box remote control. [http://www.lirc.org/]
* libftdi ~ USB type serial port driver for lirc.
            [https://www.intra2net.com/en/developer/libftdi/]
* libxml2 ~ XML C parser and toolkit. [http://xmlsoft.org/]
* sqlite ~ SQL database engine. [https://www.sqlite.org/]

Essential external source
-------------------------
apollo_record use some external sources. But the following sources is
pre-included in apollo_record source, so you don't need anymore.

* epg2xml ~ Gathering EPG from internet server and generating XML. (MODIFIED)
            [https://github.com/wonipapa/epg2xml/]

Build
-----

In the root folder, type:

    make

Installation
------------

In the root folder, type:

    make install

Run
---

apollo_record consists of two daemon programs. One is 'apollo_monitor', and
the other is 'apollo_controller'.
'apollo_monitor' is the program processing external video and audio input
source, and measuring loudness. It can run as many as the number of external
input sources you want to measure simultaneously.
Below is the usage of 'apollo_monitor'

    Usage: ./apollo_monitor [options]
    Options:
    -h | --help           Print this message
    -v | --video name     Input video device name (ex : /dev/video0)
    -a | --audio name     Input audio device name (ex : hw:1,0)
    -f | --fifo name      AV FIFO name
    -s | --socket name    IPC socket name

'apollo_controller' is the program communicating with management program, and
controlling 'apollo_monitor' programs and managing database. It can run with one
and more 'apollo_monitor'.
Below is the usage of 'apollo_controller'

    Usage: ./apollo_controller [options]
    Options:
    -h | --help               Print this message
    -i | --ipc name           IPC socket name(s) (max 5 sockets)
    -l | --lircd name         lircd socket name(s) (max 5 sockets)
    -s | --sqlite name        Sqlite database name
    -m | --messenger number   Messenger port number
    -L | --log path           Loudness log path
    -R | --record path        AV record path
    -E | --epg path           EPG XML path

'apollo_monitor' and 'apollo_controller' should be run together firmly.
Below are some run example of 'apollo_monitor' and 'apollo_controller'.

    (Use one external input source)
    $ apollo_monitor -v /dev/video0 -a hw:1,0 -f apollo.ts -s apollo
    $ apollo_controller -i apollo -l lircd -s apollo.db -m 3400 -L ./log/
    -R ./record/ -E ./epg/

    (Use three external input sources)
    $ apollo_monitor -v /dev/video0 -a hw:1,0 -f apollo0.ts -s apollo0
    $ apollo_monitor -v /dev/video1 -a hw:2,0 -f apollo1.ts -s apollo1
    $ apollo_monitor -v /dev/video2 -a hw:3,0 -f apollo2.ts -s apollo2
    $ apollo_controller -i apollo0 -i apollo1 -i apollo2 -l lircd0 -l lircd1
    -l lircd2 -s apollo.db -m 3400 -L ./log/ -R ./record/ -E ./epg/

Contact us
----------
If you have question or interest of contributing, Please visit to web site
https://cafe.naver.com/apolloopenproject/ or send E-mail to dev@lowasis.com.

Thank you.
