#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from __future__ import print_function
import imp
import os
import sys
import json
import locale
import datetime
import codecs
import socket
import re
from xml.sax.saxutils import escape, unescape
import argparse
import pprint
from functools import partial
import time

Channelstring = ' \
[ \
{ "Id": 1, "Name": "9colors", "DLIVE Name": "", "DLIVECh": null, "KT Name": "9colors", "KTCh": 163, "LG Name": "나인컬러스", "LGCh": 178, "SK Name": "9colors", "SKCh": 220, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/tRhzBgK.png", "Source": "SK", "ServiceId": "285" }, \
{ "Id": 2, "Name": "애니박스", "DLIVE Name": "애니박스", "DLIVECh": 210, "KT Name": "애니박스", "KTCh": 135, "LG Name": "애니박스", "LGCh": 148, "SK Name": "애니박스", "SKCh": 179, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ipM9AXe.png", "Source": "SK", "ServiceId": "191" }, \
{ "Id": 3, "Name": "Animal Planet", "DLIVE Name": "", "DLIVECh": null, "KT Name": "Animal Planet", "KTCh": 179, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ae70Di1.png", "Source": "NAVER", "ServiceId": "815374" }, \
{ "Id": 4, "Name": "ANIMAX", "DLIVE Name": "ANIMAX", "DLIVECh": 202, "KT Name": "ANIMAX", "KTCh": 133, "LG Name": "애니맥스", "LGCh": 167, "SK Name": "Animax", "SKCh": 173, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/2Gfqhuj.png", "Source": "SK", "ServiceId": "371" }, \
{ "Id": 5, "Name": "애니원", "DLIVE Name": "", "DLIVECh": null, "KT Name": "애니원", "KTCh": 134, "LG Name": "애니원", "LGCh": 153, "SK Name": "애니원", "SKCh": 174, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/sIp2MZ3.png", "Source": "SK", "ServiceId": "379" }, \
{ "Id": 6, "Name": "예술 TV아르떼", "DLIVE Name": "예술TVArte", "DLIVECh": 140, "KT Name": "예술 TV아르떼", "KTCh": 91, "LG Name": "예술TV 아르떼", "LGCh": 139, "SK Name": "Arte TV", "SKCh": 234, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/zReu7df.png", "Source": "SK", "ServiceId": "421" }, \
{ "Id": 7, "Name": "Asia UHD", "DLIVE Name": "", "DLIVECh": null, "KT Name": "Asia UHD", "KTCh": 109, "LG Name": "", "LGCh": null, "SK Name": "Asia UHD", "SKCh": 72, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/1u80OGN.png", "Source": "NAVER", "ServiceId": "4124120" }, \
{ "Id": 8, "Name": "AsiaN", "DLIVE Name": "아시아N", "DLIVECh": 147, "KT Name": "AsiaN", "KTCh": 111, "LG Name": "아시아N", "LGCh": 88, "SK Name": "Asia N", "SKCh": 106, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/gleMSFq.png", "Source": "SK", "ServiceId": "177" }, \
{ "Id": 9, "Name": "Australia Plus", "DLIVE Name": "", "DLIVECh": null, "KT Name": "Australia Plus", "KTCh": 258, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/6k9j2Hj.png", "Source": "NAVER", "ServiceId": "815391" }, \
{ "Id": 10, "Name": "AXN", "DLIVE Name": "AXN", "DLIVECh": 39, "KT Name": "AXN", "KTCh": 113, "LG Name": "AXN", "LGCh": 45, "SK Name": "AXN", "SKCh": 102, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/QrAXBEm.png", "Source": "LG", "ServiceId": "744" }, \
{ "Id": 11, "Name": "SK stoa", "DLIVE Name": "SK stoa", "DLIVECh": 30, "KT Name": "SK stoa", "KTCh": 30, "LG Name": "SK stoa", "LGCh": 28, "SK Name": "SK stoa", "SKCh": 21, "Radio Name": "", "RadioCh": null, "Icon_url": "https://i.imgur.com/8pwPc5R.png", "Source": "LG", "ServiceId": "738" }, \
{ "Id": 14, "Name": "Baby TV", "DLIVE Name": "베이비TV", "DLIVECh": 217, "KT Name": "Baby TV", "KTCh": 146, "LG Name": "", "LGCh": null, "SK Name": "Baby TV", "SKCh": 195, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/mU7QuTz.png", "Source": "SKB", "ServiceId": "785" }, \
{ "Id": 15, "Name": "BBC Earth", "DLIVE Name": "BBC Earth", "DLIVECh": 172, "KT Name": "BBC Earth", "KTCh": 172, "LG Name": "BBC Earth", "LGCh": 130, "SK Name": "BBC earth", "SKCh": 265, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/SJZjuqO.png", "Source": "SK", "ServiceId": "472" }, \
{ "Id": 18, "Name": "BBC WN", "DLIVE Name": "BBC월드", "DLIVECh": 191, "KT Name": "BBC WN", "KTCh": 192, "LG Name": "BBC World News", "LGCh": 126, "SK Name": "BBC World News", "SKCh": 160, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/IwJBNd4.png", "Source": "SK", "ServiceId": "778" }, \
{ "Id": 19, "Name": "BBS 불교방송", "DLIVE Name": "BBS불교방송", "DLIVECh": 307, "KT Name": "BBS불교방송", "KTCh": 232, "LG Name": "BBS", "LGCh": 186, "SK Name": "BBS 불교방송", "SKCh": 306, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/B34jpmo.png", "Source": "NAVER", "ServiceId": "815103" }, \
{ "Id": 20, "Name": "빌리어즈티비", "DLIVE Name": "Billiards TV", "DLIVECh": 154, "KT Name": "빌리어즈티비", "KTCh": 116, "LG Name": "빌리어즈TV", "LGCh": 63, "SK Name": "Billiards TV", "SKCh": 130, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/f0OW8wX.png", "Source": "SK", "ServiceId": "122" }, \
{ "Id": 21, "Name": "Bloomberg", "DLIVE Name": "블룸버그", "DLIVECh": 193, "KT Name": "Bloomberg", "KTCh": 196, "LG Name": "", "LGCh": null, "SK Name": "Bloomberg TV", "SKCh": 162, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/WWNLhh3.png", "Source": "SK", "ServiceId": "775" }, \
{ "Id": 23, "Name": "BTN불교TV", "DLIVE Name": "BTN불교TV", "DLIVECh": 306, "KT Name": "BTN불교TV", "KTCh": 233, "LG Name": "BTN", "LGCh": 185, "SK Name": "BTN 불교TV", "SKCh": 305, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/XU4pwpC.png", "Source": "NAVER", "ServiceId": "815112" }, \
{ "Id": 24, "Name": "C channel", "DLIVE Name": "C채널", "DLIVECh": 305, "KT Name": "C Channel", "KTCh": 235, "LG Name": "C채널", "LGCh": 182, "SK Name": "C채널", "SKCh": 304, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/SFouN1c.png", "Source": "NAVER", "ServiceId": "815109" }, \
{ "Id": 26, "Name": "C TIME", "DLIVE Name": "C TIME", "DLIVECh": 47, "KT Name": "C TIME", "KTCh": 69, "LG Name": "C타임", "LGCh": 86, "SK Name": "C TIME", "SKCh": 86, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/2FwWidb.png", "Source": "LG", "ServiceId": "775" }, \
{ "Id": 27, "Name": "Cbeebies", "DLIVE Name": "", "DLIVECh": null, "KT Name": "Cbeebies", "KTCh": 152, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/4bIQeie.png", "Source": "NAVER", "ServiceId": "814991" }, \
{ "Id": 28, "Name": "CBS", "DLIVE Name": "CBS", "DLIVECh": 302, "KT Name": "CBS", "KTCh": 238, "LG Name": "CBS", "LGCh": 181, "SK Name": "CBS", "SKCh": 300, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/yBNo2mS.png", "Source": "NAVER", "ServiceId": "815104" }, \
{ "Id": 31, "Name": "CCTV4", "DLIVE Name": "CCTV4", "DLIVECh": 196, "KT Name": "CCTV4", "KTCh": 280, "LG Name": "CCTV4", "LGCh": 120, "SK Name": "CCTV4", "SKCh": 277, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ACwvhcc.png", "Source": "NAVER", "ServiceId": "815401" }, \
{ "Id": 32, "Name": "CGNTV", "DLIVE Name": "CGNTV", "DLIVECh": 304, "KT Name": "CGNTV", "KTCh": 237, "LG Name": "CGNTV", "LGCh": 183, "SK Name": "CGNTV", "SKCh": 302, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/9fNvKEX.png", "Source": "NAVER", "ServiceId": "815106" }, \
{ "Id": 34, "Name": "Channel [V]", "DLIVE Name": "", "DLIVECh": null, "KT Name": "Channel [V]", "KTCh": 89, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/t1PNA6v.png", "Source": "KT", "ServiceId": "89" }, \
{ "Id": 35, "Name": "채널 J", "DLIVE Name": "채널J", "DLIVECh": 66, "KT Name": "채널 J", "KTCh": 108, "LG Name": "채널J", "LGCh": 145, "SK Name": "채널J", "SKCh": 103, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/V9jGCZm.png", "Source": "LG", "ServiceId": "656" }, \
{ "Id": 36, "Name": "Channel News Asia", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "Channel News Asia", "SKCh": 163, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/hahdOp1.png", "Source": "SKB", "ServiceId": "777" }, \
{ "Id": 38, "Name": "cineF", "DLIVE Name": "씨네프", "DLIVECh": 79, "KT Name": "", "KTCh": null, "LG Name": "시네프", "LGCh": 42, "SK Name": "Cinef", "SKCh": 58, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/TfhQXIv.png", "Source": "NAVER", "ServiceId": "814729" }, \
{ "Id": 39, "Name": "UXN", "DLIVE Name": "UXN", "DLIVECh": 165, "KT Name": "UXN", "KTCh": 101, "LG Name": "UXN", "LGCh": 2, "SK Name": "UXN", "SKCh": 70, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/5rRW0R3.png", "Source": "NAVER", "ServiceId": "2843916" }, \
{ "Id": 40, "Name": "CJ오쇼핑", "DLIVE Name": "CJ오쇼핑", "DLIVECh": 12, "KT Name": "CJ오쇼핑", "KTCh": 4, "LG Name": "CJ오쇼핑", "LGCh": 8, "SK Name": "CJ오쇼핑", "SKCh": 6, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/uBClUx6.png", "Source": "SK", "ServiceId": "324" }, \
{ "Id": 41, "Name": "CJ오쇼핑 플러스", "DLIVE Name": "CJ오쇼핑플러스", "DLIVECh": 46, "KT Name": "CJ오쇼핑플러스", "KTCh": 28, "LG Name": "CJ오쇼핑+", "LGCh": 32, "SK Name": "CJ오쇼핑 플러스", "SKCh": 33, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/WlK2YDk.png", "Source": "SKB", "ServiceId": "340" }, \
{ "Id": 42, "Name": "CLASSICA", "DLIVE Name": "Classica", "DLIVECh": 152, "KT Name": "CLASSICA", "KTCh": 90, "LG Name": "클래시카", "LGCh": 146, "SK Name": "STINGRAY CLASSICA", "SKCh": 235, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/FEfMS0W.png", "Source": "SK", "ServiceId": "787" }, \
{ "Id": 43, "Name": "CMC가족오락TV", "DLIVE Name": "CMC가족오락TV", "DLIVECh": 85, "KT Name": "CMC가족오락TV", "KTCh": 126, "LG Name": "", "LGCh": null, "SK Name": "CMC 가족오락TV", "SKCh": 93, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/fNuqGzR.png", "Source": "NAVER", "ServiceId": "814797" }, \
{ "Id": 44, "Name": "CMTV", "DLIVE Name": "CMTV", "DLIVECh": 161, "KT Name": "CMTV", "KTCh": 262, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ITweLdv.png", "Source": "KT", "ServiceId": "44" }, \
{ "Id": 45, "Name": "CNBC", "DLIVE Name": "CNBC", "DLIVECh": 192, "KT Name": "CNBC", "KTCh": 197, "LG Name": "CNBC", "LGCh": 118, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/Mx8ZXqk.png", "Source": "NAVER", "ServiceId": "815128" }, \
{ "Id": 46, "Name": "CNN International", "DLIVE Name": "CNN", "DLIVECh": 190, "KT Name": "CNN International", "KTCh": 191, "LG Name": "CNN International", "LGCh": 117, "SK Name": "CNN International", "SKCh": 158, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/RxsYny9.png", "Source": "SK", "ServiceId": "774" }, \
{ "Id": 47, "Name": "CNN US", "DLIVE Name": "CNN US", "DLIVECh": 151, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "CNN US", "SKCh": 159, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/EL6i9mA.png", "Source": "SK", "ServiceId": "782" }, \
{ "Id": 48, "Name": "CNTV", "DLIVE Name": "CNTV", "DLIVECh": 51, "KT Name": "CNTV", "KTCh": 68, "LG Name": "CNTV", "LGCh": 85, "SK Name": "CNTV", "SKCh": 43, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/lCGq3v1.png", "Source": "NAVER", "ServiceId": "814709" }, \
{ "Id": 49, "Name": "CTS기독교TV", "DLIVE Name": "CTS기독교TV", "DLIVECh": 301, "KT Name": "CTS기독교TV", "KTCh": 236, "LG Name": "CTS", "LGCh": 180, "SK Name": "CTS", "SKCh": 301, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/lW9nvaB.png", "Source": "NAVER", "ServiceId": "815110" }, \
{ "Id": 50, "Name": "CUBE TV", "DLIVE Name": "CUBE TV", "DLIVECh": 74, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "CUBE TV", "SKCh": 89, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/xvEjFJa.png", "Source": "NAVER", "ServiceId": "1725243" }, \
{ "Id": 51, "Name": "디스커버리채널", "DLIVE Name": "Discovery", "DLIVECh": 171, "KT Name": "디스커버리채널", "KTCh": 177, "LG Name": "", "LGCh": null, "SK Name": "Discovery Channel", "SKCh": 261, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/1SgSI0P.png", "Source": "SKB", "ServiceId": "437" }, \
{ "Id": 52, "Name": "Dog TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "Dog TV", "KTCh": 201, "LG Name": "DOG TV", "LGCh": 89, "SK Name": "DOG TV", "SKCh": 79, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/4Xqehq9.png", "Source": "SKB", "ServiceId": "255" }, \
{ "Id": 53, "Name": "Dream Works Channel", "DLIVE Name": "", "DLIVECh": null, "KT Name": "Dream Works Channel", "KTCh": 131, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ETfF49T.png", "Source": "KT", "ServiceId": "131" }, \
{ "Id": 54, "Name": "DW-TV Asia+", "DLIVE Name": "", "DLIVECh": null, "KT Name": "DW-TV Asia+", "KTCh": 257, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/b0yhwNu.png", "Source": "NAVER", "ServiceId": "815529" }, \
{ "Id": 55, "Name": "E채널", "DLIVE Name": "E채널", "DLIVECh": 38, "KT Name": "E채널", "KTCh": 48, "LG Name": "E 채널", "LGCh": 104, "SK Name": "E채널", "SKCh": 83, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/0WmEGB4.png", "Source": "SK", "ServiceId": "886" }, \
{ "Id": 56, "Name": "EBS English", "DLIVE Name": "EBSEnglish", "DLIVECh": 222, "KT Name": "EBS English", "KTCh": 156, "LG Name": "EBS English", "LGCh": 162, "SK Name": "EBS English", "SKCh": 202, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/Rx4mgpm.png", "Source": "NAVER", "ServiceId": "815299" }, \
{ "Id": 57, "Name": "EBS kids", "DLIVE Name": "", "DLIVECh": null, "KT Name": "EBS kids", "KTCh": 145, "LG Name": "EBS kids", "LGCh": 168, "SK Name": "EBS kids", "SKCh": 194, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/RIgVjvP.png", "Source": "NAVER", "ServiceId": "815547" }, \
{ "Id": 58, "Name": "EBS PLUS1", "DLIVE Name": "EBS플러스1", "DLIVECh": 220, "KT Name": "EBS PLUS1", "KTCh": 157, "LG Name": "EBS+1", "LGCh": 163, "SK Name": "EBS +1", "SKCh": 203, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/WGlWiWy.png", "Source": "LG", "ServiceId": "714" }, \
{ "Id": 59, "Name": "EBS PLUS2", "DLIVE Name": "EBS플러스2", "DLIVECh": 221, "KT Name": "EBS PLUS2", "KTCh": 158, "LG Name": "EBS+2", "LGCh": 164, "SK Name": "EBS +2", "SKCh": 204, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/QPq2wcg.png", "Source": "LG", "ServiceId": "715" }, \
{ "Id": 60, "Name": "EBS", "DLIVE Name": "EBS", "DLIVECh": 13, "KT Name": "EBS", "KTCh": 13, "LG Name": "EBS1", "LGCh": 14, "SK Name": "EBS", "SKCh": 13, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/WViFruZ.png", "Source": "SK", "ServiceId": "15" }, \
{ "Id": 61, "Name": "EBS2", "DLIVE Name": "", "DLIVECh": null, "KT Name": "EBS2", "KTCh": 95, "LG Name": "EBS2", "LGCh": 95, "SK Name": "EBS2", "SKCh": 95, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/RIrBF4D.png", "Source": "SK", "ServiceId": "63" }, \
{ "Id": 62, "Name": "Edge TV", "DLIVE Name": "EDGE TV", "DLIVECh": 55, "KT Name": "Edge TV", "KTCh": 79, "LG Name": "엣지TV", "LGCh": 68, "SK Name": "EDGE TV", "SKCh": 44, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/gsnjG4A.png", "Source": "NAVER", "ServiceId": "815028" }, \
{ "Id": 63, "Name": "edu TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "edu TV", "KTCh": 159, "LG Name": "에듀TV", "LGCh": 165, "SK Name": "edu TV", "SKCh": 205, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ZsMKNGe.png", "Source": "SK", "ServiceId": "823" }, \
{ "Id": 67, "Name": "Euro News", "DLIVE Name": "", "DLIVECh": null, "KT Name": "Euro News", "KTCh": 193, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/6jswCZM.png", "Source": "NAVER", "ServiceId": "814935" }, \
{ "Id": 68, "Name": "Euro sport", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "Eurosport", "SKCh": 134, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/7hVKz6n.png", "Source": "SKB", "ServiceId": "120" }, \
{ "Id": 71, "Name": "Fashion N", "DLIVE Name": "패션앤", "DLIVECh": 91, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "Fashion N", "SKCh": 211, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/qPBnZpV.png", "Source": "SKB", "ServiceId": "274" }, \
{ "Id": 72, "Name": "FISHING TV", "DLIVE Name": "FISHING TV", "DLIVECh": 130, "KT Name": "FISHING TV", "KTCh": 119, "LG Name": "피싱TV", "LGCh": 65, "SK Name": "FISHING TV", "SKCh": 244, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/Go4FdK1.png", "Source": "SK", "ServiceId": "273" }, \
{ "Id": 74, "Name": "FOX", "DLIVE Name": "FOX채널", "DLIVECh": 67, "KT Name": "FOX", "KTCh": 107, "LG Name": "FOX채널", "LGCh": 44, "SK Name": "FOX", "SKCh": 101, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/8usDWOC.png", "Source": "LG", "ServiceId": "654" }, \
{ "Id": 75, "Name": "Fox life", "DLIVE Name": "FOXLIFE", "DLIVECh": 93, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "Fox life", "SKCh": 216, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/CqVujLk.png", "Source": "SKB", "ServiceId": "280" }, \
{ "Id": 76, "Name": "Fox News", "DLIVE Name": "", "DLIVECh": null, "KT Name": "Fox News", "KTCh": 195, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/9rGThzD.png", "Source": "NAVER", "ServiceId": "815413" }, \
{ "Id": 78, "Name": "FTV", "DLIVE Name": "FTV", "DLIVECh": 129, "KT Name": "FTV", "KTCh": 118, "LG Name": "FTV", "LGCh": 64, "SK Name": "FTV", "SKCh": 243, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/eI9wGgS.png", "Source": "NAVER", "ServiceId": "814887" }, \
{ "Id": 79, "Name": "FUN TV", "DLIVE Name": "FUN TV", "DLIVECh": 82, "KT Name": "FUN TV", "KTCh": 71, "LG Name": "", "LGCh": null, "SK Name": "FUN TV", "SKCh": 91, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/6V8X43L.png", "Source": "SKB", "ServiceId": "887" }, \
{ "Id": 80, "Name": "FX", "DLIVE Name": "FX", "DLIVECh": 86, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "FX", "SKCh": 90, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/DAnUsTS.png", "Source": "NAVER", "ServiceId": "814805" }, \
{ "Id": 82, "Name": "GMTV", "DLIVE Name": "GMTV", "DLIVECh": 142, "KT Name": "GMTV", "KTCh": 88, "LG Name": "GMTV", "LGCh": 101, "SK Name": "GMTV", "SKCh": 232, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/WPF3G5V.png", "Source": "NAVER", "ServiceId": "814815" }, \
{ "Id": 83, "Name": "Good TV", "DLIVE Name": "GoodTV", "DLIVECh": 303, "KT Name": "Good TV", "KTCh": 234, "LG Name": "", "LGCh": null, "SK Name": "Good TV", "SKCh": 303, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/fFN1YJb.png", "Source": "NAVER", "ServiceId": "815108" }, \
{ "Id": 84, "Name": "GS MY SHOP", "DLIVE Name": "GS MY SHOP", "DLIVECh": 42, "KT Name": "GS MY SHOP", "KTCh": 38, "LG Name": "GS마이샵", "LGCh": 30, "SK Name": "GS MY SHOP", "SKCh": 29, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/0Y0pRW4.png", "Source": "SK", "ServiceId": "343" }, \
{ "Id": 85, "Name": "GS SHOP", "DLIVE Name": "GS홈쇼핑", "DLIVECh": 10, "KT Name": "GS SHOP", "KTCh": 8, "LG Name": "GS샵", "LGCh": 6, "SK Name": "GS SHOP", "SKCh": 12, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/tz88mYd.png", "Source": "NAVER", "ServiceId": "815097" }, \
{ "Id": 86, "Name": "GTV", "DLIVE Name": "GTV", "DLIVECh": 61, "KT Name": "GTV", "KTCh": 73, "LG Name": "GTV", "LGCh": 136, "SK Name": "Gtv", "SKCh": 217, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/1qXSXRa.png", "Source": "NAVER", "ServiceId": "815032" }, \
{ "Id": 87, "Name": "히어로액션", "DLIVE Name": "", "DLIVECh": null, "KT Name": "히어로액션", "KTCh": 112, "LG Name": "히어로액션", "LGCh": 110, "SK Name": "히어로액션", "SKCh": 107, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/yBChQVh.png", "Source": "NAVER", "ServiceId": "814760" }, \
{ "Id": 89, "Name": "하이라이트TV", "DLIVE Name": "하이라이트TV", "DLIVECh": 59, "KT Name": "하이라이트TV", "KTCh": 74, "LG Name": "하이라이트TV", "LGCh": 91, "SK Name": "Highlight TV", "SKCh": 42, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/V9CQ4A6.png", "Source": "LG", "ServiceId": "701" }, \
{ "Id": 90, "Name": "History", "DLIVE Name": "히스토리", "DLIVECh": 40, "KT Name": "History", "KTCh": 169, "LG Name": "히스토리", "LGCh": 132, "SK Name": "History", "SKCh": 264, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/SSkop4X.png", "Source": "LG", "ServiceId": "664" }, \
{ "Id": 91, "Name": "HQ+", "DLIVE Name": "HQ+", "DLIVECh": 72, "KT Name": "HQ+", "KTCh": 253, "LG Name": "", "LGCh": null, "SK Name": "HQ+", "SKCh": 47, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/GTANuZS.png", "Source": "NAVER", "ServiceId": "3566359" }, \
{ "Id": 92, "Name": "아이넷TV", "DLIVE Name": "아이넷", "DLIVECh": 88, "KT Name": "아이넷TV", "KTCh": 92, "LG Name": "아이넷TV", "LGCh": 106, "SK Name": "아이넷 TV", "SKCh": 233, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/JYQpemw.png", "Source": "SKB", "ServiceId": "261" }, \
{ "Id": 93, "Name": "IB SPORTS", "DLIVE Name": "IB스포츠", "DLIVECh": 122, "KT Name": "IB SPORTS", "KTCh": 53, "LG Name": "IB스포츠", "LGCh": 62, "SK Name": "IB Sports", "SKCh": 129, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/wOOQEC1.png", "Source": "SK", "ServiceId": "123" }, \
{ "Id": 94, "Name": "i-Concerts", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "i-Concerts", "SKCh": 236, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/oUUZQdi.png", "Source": "SK", "ServiceId": "786" }, \
{ "Id": 95, "Name": "JEI EnglishTV", "DLIVE Name": "JEI잉글리쉬TV", "DLIVECh": 214, "KT Name": "JEI EnglishTV", "KTCh": 154, "LG Name": "JEI EnglishTV", "LGCh": 160, "SK Name": "JEI 재능 English", "SKCh": 200, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/R1oJmLo.png", "Source": "SK", "ServiceId": "825" }, \
{ "Id": 96, "Name": "JEI 재능TV", "DLIVE Name": "JEI재능TV", "DLIVECh": 206, "KT Name": "JEI 재능TV", "KTCh": 142, "LG Name": "JEI재능TV", "LGCh": 159, "SK Name": "JEI 재능TV", "SKCh": 192, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/NoNRdSt.png", "Source": "SK", "ServiceId": "378" }, \
{ "Id": 97, "Name": "JTBC", "DLIVE Name": "JTBC", "DLIVECh": 15, "KT Name": "JTBC", "KTCh": 15, "LG Name": "JTBC", "LGCh": 15, "SK Name": "JTBC", "SKCh": 15, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/YOYosLG.png", "Source": "SK", "ServiceId": "240" }, \
{ "Id": 98, "Name": "JTBC Golf", "DLIVE Name": "JTBC골프", "DLIVECh": 112, "KT Name": "JTBC Golf", "KTCh": 56, "LG Name": "JTBC골프", "LGCh": 54, "SK Name": "JTBC GOLF", "SKCh": 132, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/Bnw7O5U.png", "Source": "SK", "ServiceId": "127" }, \
{ "Id": 99, "Name": "JTBC2", "DLIVE Name": "JTBC2", "DLIVECh": 49, "KT Name": "JTBC2", "KTCh": 39, "LG Name": "JTBC2", "LGCh": 94, "SK Name": "jtbc2", "SKCh": 82, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/WtXDuFU.png", "Source": "SK", "ServiceId": "874" }, \
{ "Id": 100, "Name": "JTBC3", "DLIVE Name": "JTBC3 FOXSPORTS", "DLIVECh": 113, "KT Name": "JTBC3", "KTCh": 62, "LG Name": "JTBC3 FOX스포츠", "LGCh": 50, "SK Name": "JTBC3 FOXSPORTS", "SKCh": 126, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/KvGwEc1.png", "Source": "SK", "ServiceId": "436" }, \
{ "Id": 101, "Name": "K STAR", "DLIVE Name": "K Star", "DLIVECh": 27, "KT Name": "K STAR", "KTCh": 87, "LG Name": "K스타", "LGCh": 105, "SK Name": "K star", "SKCh": 88, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/MYtePna.png", "Source": "LG", "ServiceId": "662" }, \
{ "Id": 103, "Name": "KBS DRAMA", "DLIVE Name": "KBS드라마", "DLIVECh": 3, "KT Name": "KBS Drama", "KTCh": 35, "LG Name": "KBS드라마", "LGCh": 31, "SK Name": "KBS 드라마", "SKCh": 30, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/CrLMIEQ.png", "Source": "SK", "ServiceId": "902" }, \
{ "Id": 104, "Name": "KBS JOY", "DLIVE Name": "KBSJOY", "DLIVECh": 29, "KT Name": "KBS Joy", "KTCh": 41, "LG Name": "KBS조이", "LGCh": 3, "SK Name": "KBS joy", "SKCh": 80, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/D1o9by9.png", "Source": "SK", "ServiceId": "880" }, \
{ "Id": 105, "Name": "KBS kids", "DLIVE Name": "KBS키즈", "DLIVECh": 209, "KT Name": "KBS Kids", "KTCh": 144, "LG Name": "KBS키즈", "LGCh": 169, "SK Name": "KBS KIDS", "SKCh": 190, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/Jcfjs9E.png", "Source": "SK", "ServiceId": "382" }, \
{ "Id": 106, "Name": "KBSN Life", "DLIVE Name": "", "DLIVECh": null, "KT Name": "KBSN Life", "KTCh": 281, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/5Ni1YIT.png", "Source": "NAVER", "ServiceId": "815340" }, \
{ "Id": 107, "Name": "KBS N Sports", "DLIVE Name": "KBSN스포츠", "DLIVECh": 118, "KT Name": "KBS N Sports", "KTCh": 59, "LG Name": "KBSN스포츠", "LGCh": 59, "SK Name": "KBSN 스포츠", "SKCh": 121, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/xspD7UE.png", "Source": "LG", "ServiceId": "618" }, \
{ "Id": 108, "Name": "KBS W", "DLIVE Name": "KBS W", "DLIVECh": 56, "KT Name": "KBS W", "KTCh": 83, "LG Name": "KBS W", "LGCh": 77, "SK Name": "KBS W", "SKCh": 214, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/sVqBczs.png", "Source": "SK", "ServiceId": "425" }, \
{ "Id": 110, "Name": "KBS1", "DLIVE Name": "KBS1", "DLIVECh": 9, "KT Name": "KBS1", "KTCh": 9, "LG Name": "KBS1", "LGCh": 9, "SK Name": "KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "SK", "ServiceId": "11" }, \
{ "Id": 111, "Name": "KBS2", "DLIVE Name": "KBS2", "DLIVECh": 7, "KT Name": "KBS2", "KTCh": 7, "LG Name": "KBS2", "LGCh": 7, "SK Name": "KBS2", "SKCh": 7, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/XgibZbD.png", "Source": "SK", "ServiceId": "12" }, \
{ "Id": 112, "Name": "키즈톡톡 플러스", "DLIVE Name": "키즈톡톡플러스", "DLIVECh": 213, "KT Name": "키즈톡톡 플러스", "KTCh": 161, "LG Name": "키즈톡톡 플러스", "LGCh": 158, "SK Name": "키즈톡톡 플러스", "SKCh": 189, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/0loInJO.png", "Source": "NAVER", "ServiceId": "815316" }, \
{ "Id": 115, "Name": "KIDS-TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "KIDS-TV", "KTCh": 149, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/AVCSEdn.png", "Source": "NAVER", "ServiceId": "814999" }, \
{ "Id": 116, "Name": "K-NET TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "K-NET TV", "KTCh": 230, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/GelnqjB.png", "Source": "KT", "ServiceId": "230" }, \
{ "Id": 117, "Name": "KTV", "DLIVE Name": "KTV", "DLIVECh": 250, "KT Name": "KTV", "KTCh": 64, "LG Name": "KTV", "LGCh": 171, "SK Name": "KTV", "SKCh": 290, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/yUk2RhO.png", "Source": "NAVER", "ServiceId": "815090" }, \
{ "Id": 118, "Name": "K-바둑", "DLIVE Name": "K바둑", "DLIVECh": 135, "KT Name": "K-바둑", "KTCh": 121, "LG Name": "K-바둑", "LGCh": 107, "SK Name": "K-바둑", "SKCh": 241, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/EAk5ySy.png", "Source": "NAVER", "ServiceId": "814894" }, \
{ "Id": 119, "Name": "K쇼핑", "DLIVE Name": "K쇼핑", "DLIVECh": 32, "KT Name": "K쇼핑", "KTCh": 20, "LG Name": "", "LGCh": null, "SK Name": "K쇼핑", "SKCh": 25, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/6ORm0Rz.png", "Source": "SK", "ServiceId": "333" }, \
{ "Id": 121, "Name": "Mnet", "DLIVE Name": "M-NET", "DLIVECh": 35, "KT Name": "Mnet", "KTCh": 27, "LG Name": "엠넷", "LGCh": 22, "SK Name": "M.net", "SKCh": 27, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/putHm2b.png", "Source": "SK", "ServiceId": "873" }, \
{ "Id": 122, "Name": "MBC", "DLIVE Name": "MBC", "DLIVECh": 11, "KT Name": "MBC", "KTCh": 11, "LG Name": "MBC", "LGCh": 11, "SK Name": "MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "SK", "ServiceId": "13" }, \
{ "Id": 123, "Name": "MBC Every1", "DLIVE Name": "MBCEvery1", "DLIVECh": 31, "KT Name": "MBC Every1", "KTCh": 3, "LG Name": "MBC에브리원", "LGCh": 29, "SK Name": "MBC Every1", "SKCh": 28, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/oOnpFes.png", "Source": "SK", "ServiceId": "881" }, \
{ "Id": 124, "Name": "MBC MUSIC", "DLIVE Name": "MBC뮤직", "DLIVECh": 73, "KT Name": "MBC MUSIC", "KTCh": 97, "LG Name": "MBC뮤직", "LGCh": 99, "SK Name": "MBC Music", "SKCh": 231, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/6g56RDx.png", "Source": "SK", "ServiceId": "250" }, \
{ "Id": 125, "Name": "MBC NET", "DLIVE Name": "MBC-NET", "DLIVECh": 176, "KT Name": "MBC NET", "KTCh": 164, "LG Name": "MBCNET", "LGCh": 140, "SK Name": "MBC NET", "SKCh": 274, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/lxDOgY8.png", "Source": "SK", "ServiceId": "281" }, \
{ "Id": 126, "Name": "MBC SPORTS+", "DLIVE Name": "MBC스포츠+", "DLIVECh": 116, "KT Name": "MBC SPORT+", "KTCh": 60, "LG Name": "MBC스포츠+", "LGCh": 60, "SK Name": "MBC Sports+", "SKCh": 123, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/Su21uj3.png", "Source": "SK", "ServiceId": "131" }, \
{ "Id": 127, "Name": "MBC SPORTS+2", "DLIVE Name": "MBC스포츠+2", "DLIVECh": 115, "KT Name": "MBC SPORTS+2", "KTCh": 61, "LG Name": "MBC스포츠+2", "LGCh": 61, "SK Name": "MBC SPORTS+2", "SKCh": 124, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/frAuUS3.png", "Source": "SK", "ServiceId": "531" }, \
{ "Id": 128, "Name": "MBC Dramanet", "DLIVE Name": "MBC드라마넷", "DLIVECh": 43, "KT Name": "MBC Dramanet", "KTCh": 3, "LG Name": "MBC드라마넷", "LGCh": 35, "SK Name": "MBC 드라마", "SKCh": 32, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/VBMFcZ3.png", "Source": "SK", "ServiceId": "900" }, \
{ "Id": 129, "Name": "MBN", "DLIVE Name": "MBN", "DLIVECh": 18, "KT Name": "MBN", "KTCh": 16, "LG Name": "MBN", "LGCh": 16, "SK Name": "MBN", "SKCh": 16, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/p0mvIJN.png", "Source": "SK", "ServiceId": "241" }, \
{ "Id": 130, "Name": "MBN Plus", "DLIVE Name": "MBN PLUS", "DLIVECh": 71, "KT Name": "MBN Plus", "KTCh": 99, "LG Name": "MBN+", "LGCh": 116, "SK Name": "MBN 플러스", "SKCh": 98, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/yMZiUUp.png", "Source": "NAVER", "ServiceId": "5286722" }, \
{ "Id": 133, "Name": "마운틴TV", "DLIVE Name": "마운틴TV", "DLIVECh": 128, "KT Name": "마운틴TV", "KTCh": 117, "LG Name": "마운틴TV", "LGCh": 69, "SK Name": "Mountain TV", "SKCh": 247, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/4nLYnVC.png", "Source": "NAVER", "ServiceId": "814889" }, \
{ "Id": 134, "Name": "mplex", "DLIVE Name": "Mplex", "DLIVECh": 96, "KT Name": "mplex", "KTCh": 103, "LG Name": "엠플렉스", "LGCh": 46, "SK Name": "Mplex", "SKCh": 57, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/dOOBYJm.png", "Source": "NAVER", "ServiceId": "814712" }, \
{ "Id": 135, "Name": "머니투데이방송", "DLIVE Name": "머니투데이방송", "DLIVECh": 181, "KT Name": "머니투데이방송", "KTCh": 181, "LG Name": "MTN", "LGCh": 122, "SK Name": "MTN", "SKCh": 152, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/joWd14j.png", "Source": "SK", "ServiceId": "627" }, \
{ "Id": 136, "Name": "NatGeo People", "DLIVE Name": "", "DLIVECh": null, "KT Name": "NatGeo People", "KTCh": 171, "LG Name": "", "LGCh": null, "SK Name": "Natgeo People", "SKCh": 263, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/2ghKPUg.png", "Source": "NAVER", "ServiceId": "814946" }, \
{ "Id": 137, "Name": "NatGeo Wild", "DLIVE Name": "Nat Geo Wild", "DLIVECh": 150, "KT Name": "NatGeo Wild", "KTCh": 170, "LG Name": "냇지오 와일드", "LGCh": 134, "SK Name": "Natgeo Wild HD", "SKCh": 266, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/M4nh0Rk.png", "Source": "SK", "ServiceId": "773" }, \
{ "Id": 138, "Name": "NGC", "DLIVE Name": "NGCK", "DLIVECh": 170, "KT Name": "NGC", "KTCh": 168, "LG Name": "내셔널지오그래픽", "LGCh": 131, "SK Name": "NGC", "SKCh": 260, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/mqTSPMN.png", "Source": "SK", "ServiceId": "430" }, \
{ "Id": 139, "Name": "NHK WP", "DLIVE Name": "", "DLIVECh": null, "KT Name": "NHK WP", "KTCh": 199, "LG Name": "NHK World Premium", "LGCh": 143, "SK Name": "NHK World Premium", "SKCh": 278, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ZSHeMzp.png", "Source": "NAVER", "ServiceId": "815432" }, \
{ "Id": 141, "Name": "니켈로디언", "DLIVE Name": "Nickelodeon", "DLIVECh": 203, "KT Name": "니켈로디언", "KTCh": 136, "LG Name": "니켈로디언", "LGCh": 154, "SK Name": "Nickelodeon", "SKCh": 176, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/6pWpFCX.png", "Source": "SK", "ServiceId": "383" }, \
{ "Id": 143, "Name": "NS Shop+", "DLIVE Name": "NS샵+", "DLIVECh": 48, "KT Name": "NS Shop+", "KTCh": 42, "LG Name": "", "LGCh": null, "SK Name": "NS Shop+", "SKCh": 41, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ipGXgEK.png", "Source": "SK", "ServiceId": "341" }, \
{ "Id": 144, "Name": "NS홈쇼핑", "DLIVE Name": "NS홈쇼핑", "DLIVECh": 4, "KT Name": "NS홈쇼핑", "KTCh": 12, "LG Name": "NS홈쇼핑", "LGCh": 13, "SK Name": "NS홈쇼핑", "SKCh": 14, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/UD1yoj1.png", "Source": "NAVER", "ServiceId": "815099" }, \
{ "Id": 147, "Name": "O tvN", "DLIVE Name": "O tvN", "DLIVECh": 50, "KT Name": "O tvn", "KTCh": 45, "LG Name": "O tvN", "LGCh": 71, "SK Name": "O tvN", "SKCh": 84, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/0SMl3O5.png", "Source": "SK", "ServiceId": "527" }, \
{ "Id": 148, "Name": "올리브", "DLIVE Name": "올리브", "DLIVECh": 33, "KT Name": "올리브", "KTCh": 34, "LG Name": "올리브", "LGCh": 82, "SK Name": "올리브", "SKCh": 34, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/5kF0Ypt.png", "Source": "SK", "ServiceId": "431" }, \
{ "Id": 149, "Name": "OBS", "DLIVE Name": "OBS", "DLIVECh": 2, "KT Name": "OBS", "KTCh": 26, "LG Name": "OBS", "LGCh": 26, "SK Name": "OBS", "SKCh": 20, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/DJHN8M6.png", "Source": "SK", "ServiceId": "70" }, \
{ "Id": 150, "Name": "OBS W", "DLIVE Name": "OBS W", "DLIVECh": 160, "KT Name": "OBS W", "KTCh": 81, "LG Name": "OBS W", "LGCh": 137, "SK Name": "OBSW", "SKCh": 219, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/uZkbU7b.png", "Source": "LG", "ServiceId": "648" }, \
{ "Id": 151, "Name": "OCN", "DLIVE Name": "OCN", "DLIVECh": 41, "KT Name": "OCN", "KTCh": 21, "LG Name": "OCN", "LGCh": 38, "SK Name": "OCN", "SKCh": 54, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/IXh6dwo.png", "Source": "SK", "ServiceId": "178" }, \
{ "Id": 153, "Name": "OGN", "DLIVE Name": "OGN", "DLIVECh": 131, "KT Name": "OGN", "KTCh": 123, "LG Name": "OGN", "LGCh": 96, "SK Name": "OGN", "SKCh": 136, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/tLtSTaq.png", "Source": "SK", "ServiceId": "124" }, \
{ "Id": 154, "Name": "온스타일", "DLIVE Name": "온스타일", "DLIVECh": 89, "KT Name": "온스타일", "KTCh": 77, "LG Name": "온스타일", "LGCh": 73, "SK Name": "On style", "SKCh": 210, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/9brMDfR.png", "Source": "NAVER", "ServiceId": "815326" }, \
{ "Id": 156, "Name": "ONT", "DLIVE Name": "ONT", "DLIVECh": 127, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "ONT", "SKCh": 245, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/2lu0ZBu.png", "Source": "SK", "ServiceId": "256" }, \
{ "Id": 157, "Name": "OUN", "DLIVE Name": "OUN", "DLIVECh": 251, "KT Name": "OUN", "KTCh": 160, "LG Name": "OUN", "LGCh": 170, "SK Name": "OUN", "SKCh": 292, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/VimbcIH.png", "Source": "SK", "ServiceId": "220" }, \
{ "Id": 158, "Name": "Outdoor", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "아웃도어 채널", "LGCh": 135, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/4M65Rrv.png", "Source": "LG", "ServiceId": "710" }, \
{ "Id": 159, "Name": "Playboy TV", "DLIVE Name": "플레이보이TV", "DLIVECh": 102, "KT Name": "Playboy TV", "KTCh": 206, "LG Name": "플레이보이", "LGCh": 290, "SK Name": "플레이보이TV", "SKCh": 320, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/NglRqcV.png", "Source": "SK", "ServiceId": "183" }, \
{ "Id": 160, "Name": "리얼TV", "DLIVE Name": "리얼TV", "DLIVECh": 87, "KT Name": "리얼TV", "KTCh": 127, "LG Name": "", "LGCh": null, "SK Name": "리얼TV", "SKCh": 267, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/PYnXtlb.png", "Source": "SK", "ServiceId": "440" }, \
{ "Id": 164, "Name": "SBS", "DLIVE Name": "SBS", "DLIVECh": 5, "KT Name": "SBS", "KTCh": 5, "LG Name": "SBS", "LGCh": 5, "SK Name": "SBS", "SKCh": 5, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/K2ztoDT.png", "Source": "SK", "ServiceId": "14" }, \
{ "Id": 165, "Name": "SBS CNBC", "DLIVE Name": "SBS CNBC", "DLIVECh": 182, "KT Name": "SBS CNBC", "KTCh": 25, "LG Name": "SBS CNBC", "LGCh": 27, "SK Name": "SBS CNBC", "SKCh": 26, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/SfDs4qN.png", "Source": "SK", "ServiceId": "625" }, \
{ "Id": 166, "Name": "SBS funE", "DLIVE Name": "SBS funE", "DLIVECh": 37, "KT Name": "SBS funE", "KTCh": 43, "LG Name": "SBS퍼니", "LGCh": 75, "SK Name": "SBS fun E", "SKCh": 81, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/D1EYJmr.png", "Source": "SK", "ServiceId": "882" }, \
{ "Id": 167, "Name": "SBS GOLF", "DLIVE Name": "SBS골프", "DLIVECh": 111, "KT Name": "SBSGOLF", "KTCh": 57, "LG Name": "SBS골프", "LGCh": 53, "SK Name": "SBS GOLF", "SKCh": 131, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/HdS0GNV.png", "Source": "SK", "ServiceId": "133" }, \
{ "Id": 168, "Name": "SBS MTV", "DLIVE Name": "SBS MTV", "DLIVECh": 75, "KT Name": "SBS MTV", "KTCh": 96, "LG Name": "SBS MTV", "LGCh": 100, "SK Name": "SBS MTV", "SKCh": 230, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/OeSJ9Ik.png", "Source": "SK", "ServiceId": "883" }, \
{ "Id": 169, "Name": "SBS Sports", "DLIVE Name": "SBS Sports", "DLIVECh": 117, "KT Name": "SBS Sports", "KTCh": 58, "LG Name": "SBS스포츠", "LGCh": 58, "SK Name": "SBS Sports", "SKCh": 122, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/j1vHAu6.png", "Source": "SK", "ServiceId": "130" }, \
{ "Id": 170, "Name": "SBS Plus", "DLIVE Name": "SBS플러스", "DLIVECh": 26, "KT Name": "SBS Plus", "KTCh": 37, "LG Name": "SBS플러스", "LGCh": 33, "SK Name": "SBS 플러스", "SKCh": 2, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/asfyrTm.png", "Source": "SK", "ServiceId": "901" }, \
{ "Id": 171, "Name": "스크린", "DLIVE Name": "스크린", "DLIVECh": 45, "KT Name": "스크린", "KTCh": 106, "LG Name": "스크린", "LGCh": 41, "SK Name": "Screen", "SKCh": 56, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/lTK9VD4.png", "Source": "SKB", "ServiceId": "192" }, \
{ "Id": 172, "Name": "SkyA&C", "DLIVE Name": "", "DLIVECh": null, "KT Name": "SkyA&C", "KTCh": 80, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/cljag6U.png", "Source": "NAVER", "ServiceId": "2230374" }, \
{ "Id": 173, "Name": "SkyDrama", "DLIVE Name": "SkyDrama", "DLIVECh": 58, "KT Name": "SkyDrama", "KTCh": 31, "LG Name": "스카이드라마", "LGCh": 79, "SK Name": "sky Drama", "SKCh": 40, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e9BTUAb.png", "Source": "NAVER", "ServiceId": "815552" }, \
{ "Id": 174, "Name": "SkyENT", "DLIVE Name": "SkyEnt", "DLIVECh": 83, "KT Name": "SkyENT", "KTCh": 50, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ALqDHH6.png", "Source": "NAVER", "ServiceId": "814833" }, \
{ "Id": 175, "Name": "Sky힐링", "DLIVE Name": "", "DLIVECh": null, "KT Name": "Sky힐링", "KTCh": 167, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/64SK0o9.png", "Source": "KT", "ServiceId": "167" }, \
{ "Id": 176, "Name": "SkyICT", "DLIVE Name": "SkyICT", "DLIVECh": 174, "KT Name": "SkyICT", "KTCh": 165, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/luZjU1e.png", "Source": "NAVER", "ServiceId": "815562" }, \
{ "Id": 177, "Name": "SkyPetPark", "DLIVE Name": "SkyPetPark", "DLIVECh": 126, "KT Name": "SkyPetPark", "KTCh": 49, "LG Name": "", "LGCh": null, "SK Name": "Sky Petpark", "SKCh": 94, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/RDUfvRG.png", "Source": "SK", "ServiceId": "889" }, \
{ "Id": 178, "Name": "SkySports", "DLIVE Name": "SkySports", "DLIVECh": 119, "KT Name": "SkySports", "KTCh": 54, "LG Name": "스카이스포츠", "LGCh": 57, "SK Name": "sky Sports", "SKCh": 125, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/QHV9bdA.png", "Source": "NAVER", "ServiceId": "815199" }, \
{ "Id": 179, "Name": "SkyTravel", "DLIVE Name": "SkyTravel", "DLIVECh": 136, "KT Name": "SkyTravel", "KTCh": 100, "LG Name": "스카이트래블", "LGCh": 66, "SK Name": "sky Travel", "SKCh": 246, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/fAJgmna.png", "Source": "NAVER", "ServiceId": "815265" }, \
{ "Id": 181, "Name": "Sky UHD", "DLIVE Name": "", "DLIVECh": null, "KT Name": "Sky UHD", "KTCh": 174, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/BNxE2zT.png", "Source": "NAVER", "ServiceId": "5332391" }, \
{ "Id": 182, "Name": "Smile TV", "DLIVE Name": "스마일TV", "DLIVECh": 53, "KT Name": "Smile TV", "KTCh": 84, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/alk8plD.png", "Source": "KT", "ServiceId": "84" }, \
{ "Id": 183, "Name": "SPOTV", "DLIVE Name": "SPOTV", "DLIVECh": 120, "KT Name": "SPOTV", "KTCh": 51, "LG Name": "스포티비", "LGCh": 56, "SK Name": "SPOTV", "SKCh": 120, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/cIpIf6b.png", "Source": "SK", "ServiceId": "125" }, \
{ "Id": 184, "Name": "SPOTV Games", "DLIVE Name": "SPOTV Games", "DLIVECh": 132, "KT Name": "SPOTV Games", "KTCh": 124, "LG Name": "스포티비 게임즈", "LGCh": 109, "SK Name": "SPOTV GAMES", "SKCh": 137, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/5IMfa6N.png", "Source": "NAVER", "ServiceId": "1876333" }, \
{ "Id": 185, "Name": "SPOTV+", "DLIVE Name": "SPOTV+", "DLIVECh": 114, "KT Name": "SPOTV+", "KTCh": 125, "LG Name": "스포티비+", "LGCh": 51, "SK Name": "SPOTV+", "SKCh": 127, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/81Fshnn.png", "Source": "SK", "ServiceId": "134" }, \
{ "Id": 186, "Name": "SPOTV2", "DLIVE Name": "SPOTV2", "DLIVECh": 121, "KT Name": "SPOTV2", "KTCh": 52, "LG Name": "스포티비2", "LGCh": 52, "SK Name": "SPOTV2", "SKCh": 128, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/GNicmGY.png", "Source": "NAVER", "ServiceId": "5286701" }, \
{ "Id": 192, "Name": "Star Sports", "DLIVE Name": "STAR SPORTS", "DLIVECh": 155, "KT Name": "Star Sports", "KTCh": 63, "LG Name": "", "LGCh": null, "SK Name": "Star Sports", "SKCh": 135, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/8ndGr4n.png", "Source": "SK", "ServiceId": "781" }, \
{ "Id": 193, "Name": "STB상생방송", "DLIVE Name": "상생방송", "DLIVECh": 300, "KT Name": "STB상생방송", "KTCh": 261, "LG Name": "상생방송", "LGCh": 187, "SK Name": "STB 상생방송", "SKCh": 308, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/Z3Xknso.png", "Source": "NAVER", "ServiceId": "815113" }, \
{ "Id": 194, "Name": "STN", "DLIVE Name": "STN스포츠", "DLIVECh": 156, "KT Name": "STN", "KTCh": 267, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ZJi3NQy.png", "Source": "KT", "ServiceId": "267" }, \
{ "Id": 196, "Name": "슈퍼액션", "DLIVE Name": "Super액션", "DLIVECh": 78, "KT Name": "슈퍼액션", "KTCh": 32, "LG Name": "수퍼액션", "LGCh": 40, "SK Name": "SUPER ACTION", "SKCh": 55, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/D9PzDBr.png", "Source": "SK", "ServiceId": "179" }, \
{ "Id": 198, "Name": "tbsTV", "DLIVE Name": "tbsTV", "DLIVECh": 234, "KT Name": "tbsTV", "KTCh": 214, "LG Name": "tbsTV", "LGCh": 176, "SK Name": "tbsTV", "SKCh": 272, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/o3MWHfb.png", "Source": "SK", "ServiceId": "420" }, \
{ "Id": 199, "Name": "텔레노벨라", "DLIVE Name": "텔레노벨라", "DLIVECh": 68, "KT Name": "텔레노벨라", "KTCh": 114, "LG Name": "텔레노벨라", "LGCh": 81, "SK Name": "텔레노벨라", "SKCh": 109, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/Ea3Vu8Z.png", "Source": "LG", "ServiceId": "700" }, \
{ "Id": 200, "Name": "The Golf Channel", "DLIVE Name": "Golf Channel Korea", "DLIVECh": 157, "KT Name": "The Golf Channel", "KTCh": 55, "LG Name": "더 골프채널 코리아", "LGCh": 55, "SK Name": "Golf Channel Korea", "SKCh": 133, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/3VlCrJ7.png", "Source": "SK", "ServiceId": "135" }, \
{ "Id": 201, "Name": "THE MOVIE", "DLIVE Name": "더무비", "DLIVECh": 98, "KT Name": "THE MOVIE", "KTCh": 104, "LG Name": "더 무비", "LGCh": 47, "SK Name": "The Movie", "SKCh": 59, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/6vzmEfd.png", "Source": "NAVER", "ServiceId": "815194" }, \
{ "Id": 202, "Name": "Tooniverse", "DLIVE Name": "투니버스", "DLIVECh": 201, "KT Name": "Tooniverse", "KTCh": 132, "LG Name": "투니버스", "LGCh": 152, "SK Name": "Tooniverse", "SKCh": 170, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/VUqSOjx.png", "Source": "SK", "ServiceId": "376" }, \
{ "Id": 203, "Name": "TRENDY", "DLIVE Name": "TRENDY", "DLIVECh": 90, "KT Name": "TRENDY", "KTCh": 251, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/NP2KTGN.png", "Source": "SKB", "ServiceId": "288" }, \
{ "Id": 205, "Name": "TV5MONDE", "DLIVE Name": "", "DLIVECh": null, "KT Name": "TV5MONDE", "KTCh": 198, "LG Name": "", "LGCh": null, "SK Name": "TV5Monde", "SKCh": 279, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/NogUKpP.png", "Source": "NAVER", "ServiceId": "815444" }, \
{ "Id": 207, "Name": "tvN", "DLIVE Name": "tvN", "DLIVECh": 16, "KT Name": "tvN", "KTCh": 17, "LG Name": "tvN", "LGCh": 17, "SK Name": "tvN", "SKCh": 17, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/OJ9A8fZ.png", "Source": "SK", "ServiceId": "872" }, \
{ "Id": 208, "Name": "TV조선", "DLIVE Name": "TV조선", "DLIVECh": 19, "KT Name": "TV조선", "KTCh": 19, "LG Name": "TV조선", "LGCh": 19, "SK Name": "TV조선", "SKCh": 19, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ShJ5joR.png", "Source": "SK", "ServiceId": "243" }, \
{ "Id": 212, "Name": "viki", "DLIVE Name": "VIKI", "DLIVECh": 101, "KT Name": "viki", "KTCh": 204, "LG Name": "비키", "LGCh": 292, "SK Name": "Viki", "SKCh": 322, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ZVD51K9.png", "Source": "SK", "ServiceId": "188" }, \
{ "Id": 213, "Name": "W 쇼핑", "DLIVE Name": "W쇼핑", "DLIVECh": 28, "KT Name": "W 쇼핑", "KTCh": 40, "LG Name": "", "LGCh": null, "SK Name": "W쇼핑", "SKCh": 37, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/Cged9ve.png", "Source": "SK", "ServiceId": "342" }, \
{ "Id": 215, "Name": "WBS원음방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "원음방송", "KTCh": 284, "LG Name": "원음방송", "LGCh": 188, "SK Name": "원음방송", "SKCh": 309, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/mKWQE7z.png", "Source": "SK", "ServiceId": "270" }, \
{ "Id": 216, "Name": "XtvN", "DLIVE Name": "XtvN", "DLIVECh": 81, "KT Name": "XtvN", "KTCh": 76, "LG Name": "XtvN", "LGCh": 72, "SK Name": "XtvN", "SKCh": 85, "Radio Name": "", "RadioCh": null, "Icon_url": "https://i.imgur.com/gGqDBjb.png", "Source": "SK", "ServiceId": "185" }, \
{ "Id": 218, "Name": "YTN", "DLIVE Name": "YTN", "DLIVECh": 24, "KT Name": "YTN", "KTCh": 24, "LG Name": "YTN", "LGCh": 24, "SK Name": "YTN", "SKCh": 24, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ByeeX5e.png", "Source": "SK", "ServiceId": "570" }, \
{ "Id": 219, "Name": "YTN 사이언스", "DLIVE Name": "사이언스TV", "DLIVECh": 95, "KT Name": "YTN 사이언스", "KTCh": 175, "LG Name": "사이언스TV", "LGCh": 25, "SK Name": "YTN 사이언스", "SKCh": 262, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/gwDHYGf.png", "Source": "SKB", "ServiceId": "422" }, \
{ "Id": 220, "Name": "YTN life", "DLIVE Name": "YTN Life", "DLIVECh": 138, "KT Name": "YTN life", "KTCh": 190, "LG Name": "YTN 라이프", "LGCh": 125, "SK Name": "YTN 라이프", "SKCh": 157, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/f66yRT9.png", "Source": "SK", "ServiceId": "632" }, \
{ "Id": 221, "Name": "가요TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "가요TV", "KTCh": 93, "LG Name": "가요TV", "LGCh": 102, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/6ncOOSg.png", "Source": "NAVER", "ServiceId": "814829" }, \
{ "Id": 222, "Name": "국방TV", "DLIVE Name": "국방TV", "DLIVECh": 258, "KT Name": "국방TV", "KTCh": 260, "LG Name": "국방TV", "LGCh": 174, "SK Name": "국방TV", "SKCh": 282, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/yyXkYzJ.png", "Source": "NAVER", "ServiceId": "815082" }, \
{ "Id": 223, "Name": "국회방송", "DLIVE Name": "국회방송", "DLIVECh": 252, "KT Name": "국회방송", "KTCh": 65, "LG Name": "국회방송", "LGCh": 172, "SK Name": "국회방송", "SKCh": 291, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/l1OEn7O.png", "Source": "LG", "ServiceId": "717" }, \
{ "Id": 225, "Name": "내외경제TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "내외경제TV", "KTCh": 285, "LG Name": "", "LGCh": null, "SK Name": "내외경제TV", "SKCh": 164, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/cQHPmCw.png", "Source": "SKB", "ServiceId": "623" }, \
{ "Id": 226, "Name": "다문화티브이", "DLIVE Name": "", "DLIVECh": null, "KT Name": "다문화티브이", "KTCh": 283, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/nZK3lCC.png", "Source": "KT", "ServiceId": "283" }, \
{ "Id": 228, "Name": "대교 어린이TV", "DLIVE Name": "어린이TV", "DLIVECh": 208, "KT Name": "대교 어린이TV", "KTCh": 141, "LG Name": "어린이TV", "LGCh": 156, "SK Name": "어린이TV", "SKCh": 191, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/CETWIE6.png", "Source": "SK", "ServiceId": "374" }, \
{ "Id": 229, "Name": "동아TV", "DLIVE Name": "동아TV", "DLIVECh": 92, "KT Name": "동아TV", "KTCh": 82, "LG Name": "동아TV", "LGCh": 84, "SK Name": "동아TV", "SKCh": 218, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/AlmV8jS.png", "Source": "NAVER", "ServiceId": "815044" }, \
{ "Id": 230, "Name": "드라마H", "DLIVE Name": "드라마H", "DLIVECh": 52, "KT Name": "드라마H", "KTCh": 70, "LG Name": "", "LGCh": null, "SK Name": "드라마H", "SKCh": 46, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/xT7pVuI.png", "Source": "SK", "ServiceId": "875" }, \
{ "Id": 231, "Name": "드라마큐브", "DLIVE Name": "드라마큐브", "DLIVECh": 54, "KT Name": "드라마큐브", "KTCh": 46, "LG Name": "", "LGCh": null, "SK Name": "드라마큐브", "SKCh": 36, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/4ESaIH6.png", "Source": "NAVER", "ServiceId": "815502" }, \
{ "Id": 232, "Name": "드라맥스", "DLIVE Name": "드라맥스", "DLIVECh": 21, "KT Name": "드라맥스", "KTCh": 47, "LG Name": "", "LGCh": null, "SK Name": "드라맥스", "SKCh": 38, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/jcguamX.png", "Source": "NAVER", "ServiceId": "814782" }, \
{ "Id": 233, "Name": "디원", "DLIVE Name": "디원", "DLIVECh": 57, "KT Name": "디원", "KTCh": 115, "LG Name": "디원", "LGCh": 90, "SK Name": "디원", "SKCh": 45, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/8nE7mmk.png", "Source": "LG", "ServiceId": "666" }, \
{ "Id": 234, "Name": "디즈니주니어", "DLIVE Name": "디즈니주니어", "DLIVECh": 207, "KT Name": "디즈니주니어", "KTCh": 151, "LG Name": "디즈니 주니어", "LGCh": 151, "SK Name": "디즈니주니어", "SKCh": 172, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/EGiEKhj.png", "Source": "SK", "ServiceId": "381" }, \
{ "Id": 235, "Name": "Disney Channel", "DLIVE Name": "디즈니", "DLIVECh": 204, "KT Name": "Disney Channel", "KTCh": 130, "LG Name": "디즈니 채널", "LGCh": 150, "SK Name": "디즈니채널", "SKCh": 171, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/qZdqeZo.png", "Source": "SK", "ServiceId": "380" }, \
{ "Id": 237, "Name": "롯데원티비", "DLIVE Name": "롯데 One TV", "DLIVECh": 36, "KT Name": "롯데원티비", "KTCh": 44, "LG Name": "롯데 OneTV", "LGCh": 21, "SK Name": "롯데OneTV", "SKCh": 35, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/vri0qlq.png", "Source": "SK", "ServiceId": "344" }, \
{ "Id": 238, "Name": "롯데홈쇼핑", "DLIVE Name": "롯데홈쇼핑", "DLIVECh": 8, "KT Name": "롯데홈쇼핑", "KTCh": 6, "LG Name": "롯데홈쇼핑", "LGCh": 12, "SK Name": "롯데홈쇼핑", "SKCh": 10, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/2bCfj0y.png", "Source": "NAVER", "ServiceId": "815100" }, \
{ "Id": 239, "Name": "리빙TV", "DLIVE Name": "리빙TV", "DLIVECh": 235, "KT Name": "리빙TV", "KTCh": 276, "LG Name": "", "LGCh": null, "SK Name": "리빙TV", "SKCh": 251, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/0jGCpfx.png", "Source": "SK", "ServiceId": "263" }, \
{ "Id": 240, "Name": "마이펫TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "마이펫TV", "SKCh": 92, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/Oy7qjnW.png", "Source": "SK", "ServiceId": "258" }, \
{ "Id": 241, "Name": "매일경제TV", "DLIVE Name": "매일경제TV", "DLIVECh": 184, "KT Name": "매일경제TV", "KTCh": 182, "LG Name": "매일경제TV", "LGCh": 112, "SK Name": "매일경제TV", "SKCh": 153, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/a4PwnPm.png", "Source": "SK", "ServiceId": "628" }, \
{ "Id": 244, "Name": "미드나잇", "DLIVE Name": "미드나잇", "DLIVECh": 103, "KT Name": "미드나잇", "KTCh": 205, "LG Name": "미드나잇", "LGCh": 291, "SK Name": "미드나잇", "SKCh": 321, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/zLJHU3J.png", "Source": "SK", "ServiceId": "184" }, \
{ "Id": 245, "Name": "바둑TV", "DLIVE Name": "바둑TV", "DLIVECh": 133, "KT Name": "바둑TV", "KTCh": 120, "LG Name": "바둑TV", "LGCh": 97, "SK Name": "바둑TV", "SKCh": 240, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/jbRu8T2.png", "Source": "SK", "ServiceId": "528" }, \
{ "Id": 246, "Name": "법률방송", "DLIVE Name": "법률방송", "DLIVECh": 260, "KT Name": "법률방송", "KTCh": 213, "LG Name": "", "LGCh": null, "SK Name": "법률방송", "SKCh": 280, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/WJaHOAP.png", "Source": "NAVER", "ServiceId": "815085" }, \
{ "Id": 247, "Name": "복지TV", "DLIVE Name": "복지TV", "DLIVECh": 255, "KT Name": "복지TV", "KTCh": 219, "LG Name": "복지TV", "LGCh": 173, "SK Name": "복지TV", "SKCh": 293, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/cTV0rnb.png", "Source": "NAVER", "ServiceId": "815087" }, \
{ "Id": 249, "Name": "부동산토마토", "DLIVE Name": "R토마토", "DLIVECh": 188, "KT Name": "부동산토마토", "KTCh": 188, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ePLp200.png", "Source": "SKB", "ServiceId": "621" }, \
{ "Id": 250, "Name": "부메랑", "DLIVE Name": "부메랑", "DLIVECh": 211, "KT Name": "부메랑", "KTCh": 139, "LG Name": "부메랑", "LGCh": 166, "SK Name": "부메랑", "SKCh": 175, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/GSPRJqf.png", "Source": "NAVER", "ServiceId": "815072" }, \
{ "Id": 251, "Name": "브레인TV", "DLIVE Name": "브레인TV", "DLIVECh": 134, "KT Name": "브레인TV", "KTCh": 122, "LG Name": "브레인TV", "LGCh": 98, "SK Name": "브레인TV", "SKCh": 242, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/1422kP1.png", "Source": "SK", "ServiceId": "279" }, \
{ "Id": 253, "Name": "사회안전방송", "DLIVE Name": "사회안전방송", "DLIVECh": 259, "KT Name": "사회안전방송", "KTCh": 278, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/EEfrqCW.png", "Source": "NAVER", "ServiceId": "815484" }, \
{ "Id": 254, "Name": "생활체육TV", "DLIVE Name": "생활체육TV", "DLIVECh": 153, "KT Name": "생활체육TV", "KTCh": 282, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/gefY5Bw.png", "Source": "NAVER", "ServiceId": "814883" }, \
{ "Id": 255, "Name": "서울경제TV", "DLIVE Name": "서울경제TV", "DLIVECh": 186, "KT Name": "서울경제TV", "KTCh": 184, "LG Name": "서울경제TV", "LGCh": 124, "SK Name": "서울경제TV", "SKCh": 156, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/LBzj77k.png", "Source": "NAVER", "ServiceId": "814917" }, \
{ "Id": 256, "Name": "소비자TV", "DLIVE Name": "소비자TV", "DLIVECh": 254, "KT Name": "소비자TV", "KTCh": 265, "LG Name": "소비자TV", "LGCh": 177, "SK Name": "소비자TV", "SKCh": 275, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/971hUD2.png", "Source": "SKB", "ServiceId": "442" }, \
{ "Id": 257, "Name": "소상공인방송", "DLIVE Name": "소상공인방송", "DLIVECh": 257, "KT Name": "소상공인방송", "KTCh": 255, "LG Name": "소상공인방송", "LGCh": 175, "SK Name": "소상공인방송", "SKCh": 271, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/3k1D4LA.png", "Source": "NAVER", "ServiceId": "815559" }, \
{ "Id": 258, "Name": "쇼핑엔T", "DLIVE Name": "쇼핑엔T", "DLIVECh": 34, "KT Name": "쇼핑엔티", "KTCh": 33, "LG Name": "쇼핑엔티", "LGCh": 76, "SK Name": "쇼핑엔티", "SKCh": 31, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/Q7FHxYB.png", "Source": "SK", "ServiceId": "336" }, \
{ "Id": 260, "Name": "신세계쇼핑", "DLIVE Name": "신세계쇼핑", "DLIVECh": 25, "KT Name": "신세계쇼핑", "KTCh": 2, "LG Name": "신세계쇼핑", "LGCh": 74, "SK Name": "신세계쇼핑", "SKCh": 22, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ZhYaqpt.png", "Source": "SK", "ServiceId": "339" }, \
{ "Id": 262, "Name": "실버아이TV", "DLIVE Name": "실버아이TV", "DLIVECh": 144, "KT Name": "실버아이TV", "KTCh": 266, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/xm1O8eQ.png", "Source": "NAVER", "ServiceId": "815075" }, \
{ "Id": 263, "Name": "아리랑 TV", "DLIVE Name": "아리랑TV", "DLIVECh": 189, "KT Name": "아리랑 TV", "KTCh": 200, "LG Name": "아리랑TV", "LGCh": 141, "SK Name": "아리랑TV", "SKCh": 270, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/nFWWIFP.png", "Source": "NAVER", "ServiceId": "815081" }, \
{ "Id": 264, "Name": "아시아경제TV", "DLIVE Name": "아시아경제TV", "DLIVECh": 187, "KT Name": "아시아경제TV", "KTCh": 186, "LG Name": "아시아경제TV", "LGCh": 113, "SK Name": "아시아경제TV", "SKCh": 154, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/2D6WoS8.png", "Source": "NAVER", "ServiceId": "814927" }, \
{ "Id": 265, "Name": "아임쇼핑", "DLIVE Name": "아임쇼핑", "DLIVECh": 20, "KT Name": "아임쇼핑", "KTCh": 22, "LG Name": "아임쇼핑", "LGCh": 20, "SK Name": "아임쇼핑", "SKCh": 3, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/E3pJ5Jz.png", "Source": "SK", "ServiceId": "332" }, \
{ "Id": 266, "Name": "애니플러스", "DLIVE Name": "애니플러스", "DLIVECh": 212, "KT Name": "애니플러스", "KTCh": 138, "LG Name": "애니플러스", "LGCh": 149, "SK Name": "애니플러스", "SKCh": 178, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/7RKoOZq.png", "Source": "SK", "ServiceId": "377" }, \
{ "Id": 268, "Name": "연합뉴스TV", "DLIVE Name": "연합뉴스TV", "DLIVECh": 23, "KT Name": "연합뉴스TV", "KTCh": 23, "LG Name": "연합뉴스TV", "LGCh": 23, "SK Name": "연합뉴스TV", "SKCh": 23, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/KlTCP8V.png", "Source": "SK", "ServiceId": "571" }, \
{ "Id": 271, "Name": "육아방송", "DLIVE Name": "육아방송", "DLIVECh": 231, "KT Name": "육아방송", "KTCh": 217, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/UEIB4ZG.png", "Source": "NAVER", "ServiceId": "815043" }, \
{ "Id": 272, "Name": "이데일리TV", "DLIVE Name": "이데일리TV", "DLIVECh": 185, "KT Name": "이데일리TV", "KTCh": 183, "LG Name": "이데일리TV", "LGCh": 123, "SK Name": "이데일리TV", "SKCh": 155, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/W90Hw2p.png", "Source": "LG", "ServiceId": "631" }, \
{ "Id": 273, "Name": "이벤트TV", "DLIVE Name": "이벤트TV", "DLIVECh": 143, "KT Name": "이벤트TV", "KTCh": 263, "LG Name": "이벤트TV", "LGCh": 103, "SK Name": "이벤트TV", "SKCh": 238, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/1Eeyijd.png", "Source": "SKB", "ServiceId": "262" }, \
{ "Id": 275, "Name": "인디필름", "DLIVE Name": "인디필름", "DLIVECh": 97, "KT Name": "인디필름", "KTCh": 277, "LG Name": "", "LGCh": null, "SK Name": "인디필름", "SKCh": 61, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/iA5UIJ9.png", "Source": "SKB", "ServiceId": "441" }, \
{ "Id": 278, "Name": "중화TV", "DLIVE Name": "중화TV", "DLIVECh": 65, "KT Name": "중화TV", "KTCh": 110, "LG Name": "중화TV", "LGCh": 87, "SK Name": "중화TV", "SKCh": 104, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/Nal18s5.png", "Source": "SKB", "ServiceId": "186" }, \
{ "Id": 279, "Name": "JJC지방자치TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "JJC지방자치TV", "KTCh": 279, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/AuXr9jA.png", "Source": "NAVER", "ServiceId": "3244879" }, \
{ "Id": 280, "Name": "채널 Ching", "DLIVE Name": "채널칭", "DLIVECh": 63, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "채널 Ching", "SKCh": 105, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/tRqGKcS.png", "Source": "SKB", "ServiceId": "907" }, \
{ "Id": 281, "Name": "채널A", "DLIVE Name": "채널A", "DLIVECh": 17, "KT Name": "채널A", "KTCh": 18, "LG Name": "채널A", "LGCh": 18, "SK Name": "채널A", "SKCh": 18, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/xti35f5.png", "Source": "SK", "ServiceId": "242" }, \
{ "Id": 282, "Name": "채널A 플러스", "DLIVE Name": "채널A플러스", "DLIVECh": 70, "KT Name": "채널A 플러스", "KTCh": 98, "LG Name": "채널A+", "LGCh": 115, "SK Name": "채널A 플러스", "SKCh": 97, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/rRisTh8.png", "Source": "SKB", "ServiceId": "891" }, \
{ "Id": 283, "Name": "채널CGV", "DLIVE Name": "채널CGV", "DLIVECh": 76, "KT Name": "채널CGV", "KTCh": 29, "LG Name": "채널CGV", "LGCh": 39, "SK Name": "Ch CGV", "SKCh": 53, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/fwjRwkx.png", "Source": "SK", "ServiceId": "187" }, \
{ "Id": 284, "Name": "채널i", "DLIVE Name": "", "DLIVECh": null, "KT Name": "채널i", "KTCh": 250, "LG Name": "", "LGCh": null, "SK Name": "채널i", "SKCh": 281, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/X7lXvtw.png", "Source": "NAVER", "ServiceId": "3173174" }, \
{ "Id": 285, "Name": "채널뷰", "DLIVE Name": "CH View", "DLIVECh": 137, "KT Name": "채널뷰", "KTCh": 176, "LG Name": "", "LGCh": null, "SK Name": "채널View", "SKCh": 212, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/lvtWgFL.png", "Source": "NAVER", "ServiceId": "814959" }, \
{ "Id": 286, "Name": "채널차이나", "DLIVE Name": "채널차이나", "DLIVECh": 64, "KT Name": "채널차이나", "KTCh": 102, "LG Name": "채널차이나", "LGCh": 80, "SK Name": "채널차이나", "SKCh": 108, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/jEyoLS1.png", "Source": "NAVER", "ServiceId": "815033" }, \
{ "Id": 287, "Name": "채널해피독", "DLIVE Name": "채널해피독", "DLIVECh": 125, "KT Name": "채널해피독", "KTCh": 203, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ly649tS.png", "Source": "NAVER", "ServiceId": "2296260" }, \
{ "Id": 289, "Name": "카툰네트워크", "DLIVE Name": "카툰네트워크", "DLIVECh": 205, "KT Name": "카툰네트워크", "KTCh": 137, "LG Name": "카툰네트워크", "LGCh": 155, "SK Name": "카툰네트워크", "SKCh": 177, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/cJZHPjr.png", "Source": "SK", "ServiceId": "384" }, \
{ "Id": 290, "Name": "캐치온1", "DLIVE Name": "캐치온1", "DLIVECh": 77, "KT Name": "캐치온1", "KTCh": 66, "LG Name": "캐치온1", "LGCh": 48, "SK Name": "CATCH ON 1", "SKCh": 51, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/qK9KkRd.png", "Source": "SK", "ServiceId": "181" }, \
{ "Id": 291, "Name": "캐치온2", "DLIVE Name": "캐치온2", "DLIVECh": 99, "KT Name": "캐치온2", "KTCh": 67, "LG Name": "캐치온2", "LGCh": 49, "SK Name": "CATCH ON 2", "SKCh": 52, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/sLc2req.png", "Source": "SK", "ServiceId": "182" }, \
{ "Id": 292, "Name": "브릿지TV", "DLIVE Name": "브릿지TV", "DLIVECh": 158, "KT Name": "브릿지TV", "KTCh": 270, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/KATTup7.png", "Source": "SKB", "ServiceId": "446" }, \
{ "Id": 293, "Name": "코미디TV", "DLIVE Name": "코미디TV", "DLIVECh": 22, "KT Name": "코미디TV", "KTCh": 85, "LG Name": "코미디TV", "LGCh": 108, "SK Name": "코미디TV", "SKCh": 87, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/DHbUoDm.png", "Source": "SK", "ServiceId": "906" }, \
{ "Id": 294, "Name": "쿠키건강TV", "DLIVE Name": "쿠키건강TV", "DLIVECh": 233, "KT Name": "쿠키건강TV", "KTCh": 220, "LG Name": "쿠키건강TV", "LGCh": 144, "SK Name": "쿠키건강TV", "SKCh": 269, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/4gl92D1.png", "Source": "SK", "ServiceId": "434" }, \
{ "Id": 296, "Name": "키즈원", "DLIVE Name": "", "DLIVECh": null, "KT Name": "키즈원", "KTCh": 148, "LG Name": "키즈원", "LGCh": 157, "SK Name": "KIDS1", "SKCh": 193, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/f8T1Sw4.png", "Source": "NAVER", "ServiceId": "815020" }, \
{ "Id": 297, "Name": "토마토TV", "DLIVE Name": "토마토TV", "DLIVECh": 183, "KT Name": "토마토TV", "KTCh": 185, "LG Name": "토마토TV", "LGCh": 111, "SK Name": "토마토TV", "SKCh": 150, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/dVWy3Ex.png", "Source": "SK", "ServiceId": "620" }, \
{ "Id": 299, "Name": "핑크하우스", "DLIVE Name": "핑크하우스", "DLIVECh": 105, "KT Name": "핑크하우스", "KTCh": 208, "LG Name": "핑크하우스", "LGCh": 295, "SK Name": "핑크하우스", "SKCh": 324, "Radio Name": "", "RadioCh": null, "Icon_url": "https://i.imgur.com/RnEFpd6.png", "Source": "SK", "ServiceId": "190" }, \
{ "Id": 301, "Name": "가톨릭평화방송", "DLIVE Name": "가톨릭평화방송", "DLIVECh": 308, "KT Name": "가톨릭평화방송", "KTCh": 231, "LG Name": "가톨릭평화방송", "LGCh": 184, "SK Name": "가톨릭평화방송", "SKCh": 307, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/G5fTCL3.png", "Source": "NAVER", "ServiceId": "815372" }, \
{ "Id": 302, "Name": "폴라리스TV", "DLIVE Name": "폴라리스TV", "DLIVECh": 146, "KT Name": "폴라리스TV", "KTCh": 129, "LG Name": "폴라리스 TV", "LGCh": 67, "SK Name": "폴라리스TV", "SKCh": 249, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/aghufJ7.png", "Source": "SK", "ServiceId": "252" }, \
{ "Id": 303, "Name": "한국경제TV", "DLIVE Name": "한국경제TV", "DLIVECh": 180, "KT Name": "한국경제TV", "KTCh": 180, "LG Name": "한국경제TV", "LGCh": 121, "SK Name": "한국경제TV", "SKCh": 151, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ChnD0FT.png", "Source": "NAVER", "ServiceId": "814929" }, \
{ "Id": 305, "Name": "한국승마방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "한국승마방송", "KTCh": 259, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/VZdQLwj.png", "Source": "NAVER", "ServiceId": "814904" }, \
{ "Id": 307, "Name": "한국직업방송", "DLIVE Name": "한국직업방송", "DLIVECh": 256, "KT Name": "한국직업방송", "KTCh": 252, "LG Name": "", "LGCh": null, "SK Name": "한국직업방송", "SKCh": 273, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/WUvf0If.png", "Source": "NAVER", "ServiceId": "814983" }, \
{ "Id": 309, "Name": "허니TV", "DLIVE Name": "허니TV", "DLIVECh": 104, "KT Name": "허니TV", "KTCh": 207, "LG Name": "허니TV", "LGCh": 293, "SK Name": "허니TV", "SKCh": 323, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/KczRCLW.png", "Source": "SK", "ServiceId": "196" }, \
{ "Id": 311, "Name": "헬스메디tv", "DLIVE Name": "헬스메디TV", "DLIVECh": 139, "KT Name": "헬스메디tv", "KTCh": 271, "LG Name": "헬스메디TV", "LGCh": 138, "SK Name": "헬스메디TV", "SKCh": 268, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/vsxRuFh.png", "Source": "NAVER", "ServiceId": "815500" }, \
{ "Id": 312, "Name": "현대홈쇼핑", "DLIVE Name": "현대홈쇼핑", "DLIVECh": 6, "KT Name": "현대홈쇼핑", "KTCh": 10, "LG Name": "현대홈쇼핑", "LGCh": 10, "SK Name": "현대홈쇼핑", "SKCh": 8, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/87fdrA5.png", "Source": "NAVER", "ServiceId": "815101" }, \
{ "Id": 313, "Name": "현대홈쇼핑+샵", "DLIVE Name": "현대홈쇼핑플러스샵", "DLIVECh": 44, "KT Name": "현대홈쇼핑+샵", "KTCh": 36, "LG Name": "현대홈쇼핑+샵", "LGCh": 34, "SK Name": "현대홈쇼핑+Shop", "SKCh": 39, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/COo8Bcm.png", "Source": "SK", "ServiceId": "337" }, \
{ "Id": 314, "Name": "홈&쇼핑", "DLIVE Name": "홈&쇼핑", "DLIVECh": 14, "KT Name": "홈&쇼핑", "KTCh": 14, "LG Name": "홈앤쇼핑", "LGCh": 4, "SK Name": "홈&쇼핑", "SKCh": 4, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/nLxw0LW.png", "Source": "NAVER", "ServiceId": "815524" }, \
{ "Id": 315, "Name": "환경TV", "DLIVE Name": "환경TV", "DLIVECh": 173, "KT Name": "환경TV", "KTCh": 166, "LG Name": "", "LGCh": null, "SK Name": "환경TV", "SKCh": 276, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/eITOr2Y.png", "Source": "NAVER", "ServiceId": "814961" }, \
{ "Id": 316, "Name": "Life U", "DLIVE Name": "LifeU", "DLIVECh": 141, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "Life U", "SKCh": 215, "Radio Name": "", "RadioCh": null, "Icon_url": "https://i.imgur.com/3VJOGoI.png", "Source": "SKB", "ServiceId": "277" }, \
{ "Id": 317, "Name": "디스커버리 아시아", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "디스커버리 아시아", "LGCh": 133, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "https://i.imgur.com/6NdyDW5.png", "Source": "LG", "ServiceId": "610" }, \
{ "Id": 318, "Name": "Celestial Movies", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "Celestial Movies", "SKCh": 62, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/xDXM13Q.png", "Source": "SKB", "ServiceId": "877" }, \
{ "Id": 319, "Name": "UHD Dream TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "UHD Dream TV", "SKCh": 71, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/aLG2rKa.png", "Source": "SKB", "ServiceId": "879" }, \
{ "Id": 320, "Name": "UMAX", "DLIVE Name": "Umax", "DLIVECh": 163, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "UMAX", "SKCh": 73, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/EgVuybQ.png", "Source": "SKB", "ServiceId": "69" }, \
{ "Id": 321, "Name": "NHK World TV", "DLIVE Name": "NHK-W", "DLIVECh": 194, "KT Name": "", "KTCh": null, "LG Name": "NHK World TV", "LGCh": 142, "SK Name": "NHK World TV", "SKCh": 221, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/pCuIQsT.png", "Source": "LG", "ServiceId": "669" }, \
{ "Id": 362, "Name": "CJB 청주방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "CJB 청주방송", "KTCh": 5, "LG Name": "CJB 청주방송", "LGCh": 5, "SK Name": "CJB 청주방송", "SKCh": 5, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/J6zQfQu.png", "Source": "NAVER", "ServiceId": "814684" }, \
{ "Id": 363, "Name": "G1 강원민방", "DLIVE Name": "", "DLIVECh": null, "KT Name": "G1 강원민방", "KTCh": 5, "LG Name": "G1 강원민방", "LGCh": 5, "SK Name": "G1 강원민방", "SKCh": 5, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/aYqGo9x.png", "Source": "NAVER", "ServiceId": "814614" }, \
{ "Id": 364, "Name": "JIBS 제주방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "JIBS 제주방송", "KTCh": 5, "LG Name": "JIBS 제주방송", "LGCh": 5, "SK Name": "JIBS 제주방송", "SKCh": 5, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/7YZ1lkJ.png", "Source": "NAVER", "ServiceId": "814703" }, \
{ "Id": 365, "Name": "JTV 전주방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "JTV 전주방송", "KTCh": 5, "LG Name": "JTV 전주방송", "LGCh": 5, "SK Name": "JTV 전주방송", "SKCh": 5, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/XFKcvaN.png", "Source": "NAVER", "ServiceId": "814661" }, \
{ "Id": 366, "Name": "KBC 광주방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "KBC 광주방송", "KTCh": 5, "LG Name": "KBC 광주방송", "LGCh": 5, "SK Name": "KBC 광주방송", "SKCh": 5, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/3M5UOIH.png", "Source": "NAVER", "ServiceId": "814652" }, \
{ "Id": 367, "Name": "KNN 부산경남방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "KNN 부산경남방송", "KTCh": 5, "LG Name": "KNN 부산경남방송", "LGCh": 5, "SK Name": "KNN 부산경남방송", "SKCh": 5, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/RcViTam.png", "Source": "NAVER", "ServiceId": "814628" }, \
{ "Id": 369, "Name": "TBC 대구방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "TBC 대구방송", "KTCh": 5, "LG Name": "TBC 대구방송", "LGCh": 5, "SK Name": "TBC 대구방송", "SKCh": 5, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/wmF5z8V.png", "Source": "NAVER", "ServiceId": "814639" }, \
{ "Id": 370, "Name": "TJB 대전방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "TJB 대전방송", "KTCh": 5, "LG Name": "TJB 대전방송", "LGCh": 5, "SK Name": "TJB 대전방송", "SKCh": 5, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/9JAy4Bu.png", "Source": "NAVER", "ServiceId": "814671" }, \
{ "Id": 371, "Name": "UBC 울산방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "UBC 울산방송", "KTCh": 5, "LG Name": "UBC 울산방송", "LGCh": 5, "SK Name": "UBC 울산방송", "SKCh": 5, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/qbBR1k5.png", "Source": "NAVER", "ServiceId": "814694" }, \
{ "Id": 372, "Name": "강릉 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "강릉 KBS1", "KTCh": 9, "LG Name": "강릉 KBS1", "LGCh": 9, "SK Name": "강릉 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "NAVER", "ServiceId": "814602" }, \
{ "Id": 373, "Name": "강릉 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "강릉 MBC", "KTCh": 11, "LG Name": "강릉 MBC", "LGCh": 11, "SK Name": "강릉 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814606" }, \
{ "Id": 374, "Name": "경인 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "경인 KBS1", "KTCh": 9, "LG Name": "경인 KBS1", "LGCh": 9, "SK Name": "경인 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "NAVER", "ServiceId": "814597" }, \
{ "Id": 375, "Name": "광주 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "광주 KBS1", "KTCh": 9, "LG Name": "광주 KBS1", "LGCh": 9, "SK Name": "광주 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "NAVER", "ServiceId": "814642" }, \
{ "Id": 376, "Name": "광주 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "광주 MBC", "KTCh": 11, "LG Name": "광주 MBC", "LGCh": 11, "SK Name": "광주 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814644" }, \
{ "Id": 377, "Name": "대구 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "대구 KBS1", "KTCh": 9, "LG Name": "대구 KBS1", "LGCh": 9, "SK Name": "대구 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "NAVER", "ServiceId": "814630" }, \
{ "Id": 378, "Name": "대구 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "대구 MBC", "KTCh": 11, "LG Name": "대구 MBC", "LGCh": 11, "SK Name": "대구 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814633" }, \
{ "Id": 379, "Name": "대전 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "대전 KBS1", "KTCh": 9, "LG Name": "대전 KBS1", "LGCh": 9, "SK Name": "대전 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "NAVER", "ServiceId": "814665" }, \
{ "Id": 380, "Name": "대전 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "대전 MBC", "KTCh": 11, "LG Name": "대전 MBC", "LGCh": 11, "SK Name": "대전 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814668" }, \
{ "Id": 381, "Name": "목포 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "목포 KBS1", "KTCh": 9, "LG Name": "목포 KBS1", "LGCh": 9, "SK Name": "목포 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "LG", "ServiceId": "523" }, \
{ "Id": 382, "Name": "목포 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "목포 MBC", "KTCh": 11, "LG Name": "목포 MBC", "LGCh": 11, "SK Name": "목포 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814646" }, \
{ "Id": 383, "Name": "부산 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "부산 KBS1", "KTCh": 9, "LG Name": "부산 KBS1", "LGCh": 9, "SK Name": "부산 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "NAVER", "ServiceId": "814617" }, \
{ "Id": 384, "Name": "부산 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "부산 MBC", "KTCh": 11, "LG Name": "부산 MBC", "LGCh": 11, "SK Name": "부산 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814621" }, \
{ "Id": 385, "Name": "삼척 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "삼척 MBC", "KTCh": 11, "LG Name": "삼척 MBC", "LGCh": 11, "SK Name": "삼척 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814608" }, \
{ "Id": 386, "Name": "순천 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "순천 KBS1", "KTCh": 9, "LG Name": "순천 KBS1", "LGCh": 9, "SK Name": "순천 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "LG", "ServiceId": "522" }, \
{ "Id": 387, "Name": "안동 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "안동 KBS1", "KTCh": 9, "LG Name": "안동 KBS1", "LGCh": 9, "SK Name": "안동 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "LG", "ServiceId": "517" }, \
{ "Id": 388, "Name": "안동 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "안동 MBC", "KTCh": 11, "LG Name": "안동 MBC", "LGCh": 11, "SK Name": "안동 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814635" }, \
{ "Id": 389, "Name": "여수 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "여수 MBC", "KTCh": 11, "LG Name": "여수 MBC", "LGCh": 11, "SK Name": "여수 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814649" }, \
{ "Id": 390, "Name": "울산 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "울산 KBS1", "KTCh": 9, "LG Name": "울산 KBS1", "LGCh": 9, "SK Name": "울산 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "NAVER", "ServiceId": "814687" }, \
{ "Id": 391, "Name": "울산 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "울산 MBC", "KTCh": 11, "LG Name": "울산 MBC", "LGCh": 11, "SK Name": "울산 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814691" }, \
{ "Id": 392, "Name": "원주 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "원주 KBS1", "KTCh": 9, "LG Name": "원주 KBS1", "LGCh": 9, "SK Name": "원주 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "LG", "ServiceId": "531" }, \
{ "Id": 393, "Name": "원주 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "원주 MBC", "KTCh": 11, "LG Name": "원주 MBC", "LGCh": 11, "SK Name": "원주 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814610" }, \
{ "Id": 394, "Name": "전주 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "전주 KBS1", "KTCh": 9, "LG Name": "전주 KBS1", "LGCh": 9, "SK Name": "전주 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "NAVER", "ServiceId": "814655" }, \
{ "Id": 395, "Name": "전주 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "전주 MBC", "KTCh": 11, "LG Name": "전주 MBC", "LGCh": 11, "SK Name": "전주 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814658" }, \
{ "Id": 396, "Name": "제주 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "제주 KBS1", "KTCh": 9, "LG Name": "제주 KBS1", "LGCh": 9, "SK Name": "제주 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "NAVER", "ServiceId": "814697" }, \
{ "Id": 397, "Name": "제주 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "제주 MBC", "KTCh": 11, "LG Name": "제주 MBC", "LGCh": 11, "SK Name": "제주 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814700" }, \
{ "Id": 398, "Name": "진주 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "진주 KBS1", "KTCh": 9, "LG Name": "진주 KBS1", "LGCh": 9, "SK Name": "진주 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "LG", "ServiceId": "529" }, \
{ "Id": 399, "Name": "진주 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "진주 MBC", "KTCh": 11, "LG Name": "진주 MBC", "LGCh": 11, "SK Name": "진주 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814624" }, \
{ "Id": 400, "Name": "창원 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "창원 KBS1", "KTCh": 9, "LG Name": "창원 KBS1", "LGCh": 9, "SK Name": "창원 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "NAVER", "ServiceId": "814619" }, \
{ "Id": 401, "Name": "청주 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "청주 KBS1", "KTCh": 9, "LG Name": "청주 KBS1", "LGCh": 9, "SK Name": "청주 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "NAVER", "ServiceId": "814675" }, \
{ "Id": 402, "Name": "청주 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "청주 MBC", "KTCh": 11, "LG Name": "청주 MBC", "LGCh": 11, "SK Name": "청주 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814678" }, \
{ "Id": 403, "Name": "춘천 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "춘천 KBS1", "KTCh": 9, "LG Name": "춘천 KBS1", "LGCh": 9, "SK Name": "춘천 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "NAVER", "ServiceId": "814604" }, \
{ "Id": 404, "Name": "춘천 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "춘천 MBC", "KTCh": 11, "LG Name": "춘천 MBC", "LGCh": 11, "SK Name": "춘천 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814612" }, \
{ "Id": 405, "Name": "충주 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "충주 KBS1", "KTCh": 9, "LG Name": "충주 KBS1", "LGCh": 9, "SK Name": "충주 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "LG", "ServiceId": "513" }, \
{ "Id": 406, "Name": "충주 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "충주 MBC", "KTCh": 11, "LG Name": "충주 MBC", "LGCh": 11, "SK Name": "충주 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "LG", "ServiceId": "538" }, \
{ "Id": 407, "Name": "포항 KBS1", "DLIVE Name": "", "DLIVECh": null, "KT Name": "포항 KBS1", "KTCh": 9, "LG Name": "포항 KBS1", "LGCh": 9, "SK Name": "포항 KBS1", "SKCh": 9, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/e31o5gw.png", "Source": "LG", "ServiceId": "518" }, \
{ "Id": 408, "Name": "포항 MBC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "포항 MBC", "KTCh": 11, "LG Name": "포항 MBC", "LGCh": 11, "SK Name": "포항 MBC", "SKCh": 11, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/duig32i.png", "Source": "NAVER", "ServiceId": "814637" }, \
{ "Id": 409, "Name": "DIA TV", "DLIVE Name": "DIA TV", "DLIVECh": 80, "KT Name": "DIA TV", "KTCh": 72, "LG Name": "다이아TV", "LGCh": 93, "SK Name": "DIA TV", "SKCh": 96, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/BH3DnrW.png", "Source": "LG", "ServiceId": "690" }, \
{ "Id": 410, "Name": "메디컬TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "메디컬TV", "KTCh": 254, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/S73ArUy.png", "Source": "KT", "ServiceId": "254" }, \
{ "Id": 411, "Name": "CGTN", "DLIVE Name": "CGTN", "DLIVECh": 195, "KT Name": "CGTN", "KTCh": 194, "LG Name": "CGTN", "LGCh": 119, "SK Name": "CGTN", "SKCh": 161, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/rhZf4Zx.png", "Source": "SKB", "ServiceId": "771" }, \
{ "Id": 412, "Name": "C Music TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "C Music TV", "SKCh": 237, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/hV270KM.png", "Source": "SKB", "ServiceId": "672" }, \
{ "Id": 414, "Name": "SPOTV ON", "DLIVE Name": "SPOTV ON 1", "DLIVECh": 109, "KT Name": "SPOTV ON", "KTCh": 211, "LG Name": "", "LGCh": null, "SK Name": "SPOTV ON", "SKCh": 118, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/Y3eYOc2.png", "Source": "SKB", "ServiceId": "136" }, \
{ "Id": 415, "Name": "SPOTV ON2", "DLIVE Name": "SPOTV ON 2", "DLIVECh": 110, "KT Name": "SPOTV ON2", "KTCh": 212, "LG Name": "", "LGCh": null, "SK Name": "SPOTV ON2", "SKCh": 119, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/jbdurz4.png", "Source": "SKB", "ServiceId": "137" }, \
{ "Id": 416, "Name": "한국선거방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "한국선거방송", "KTCh": 273, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/ZSdcknl.png", "Source": "KT", "ServiceId": "273" }, \
{ "Id": 417, "Name": "EBS 교육방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "EBS 교육방송", "RadioCh": 1001, "Icon_url": "http://i.imgur.com/eEsZJop.png", "Source": "NAVER", "ServiceId": "815452" }, \
{ "Id": 418, "Name": "KBS 1 라디오", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "KBS 1 라디오", "RadioCh": 1002, "Icon_url": "http://i.imgur.com/ikJ7QQn.png", "Source": "NAVER", "ServiceId": "815455" }, \
{ "Id": 419, "Name": "KBS 2 라디오", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "KBS 2 라디오", "RadioCh": 1003, "Icon_url": "http://i.imgur.com/qTrmNld.png", "Source": "NAVER", "ServiceId": "815458" }, \
{ "Id": 420, "Name": "KBS 3 라디오", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "KBS 3 라디오", "RadioCh": 1004, "Icon_url": "http://i.imgur.com/3tHl7QR.png", "Source": "NAVER", "ServiceId": "815460" }, \
{ "Id": 421, "Name": "KBS ClassicFM", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "KBS ClassicFM", "RadioCh": 1005, "Icon_url": "http://i.imgur.com/Z46a05G.png", "Source": "NAVER", "ServiceId": "815454" }, \
{ "Id": 422, "Name": "KBS CoolFM", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "KBS CoolFM", "RadioCh": 1006, "Icon_url": "http://i.imgur.com/0SQrpHZ.png", "Source": "NAVER", "ServiceId": "815457" }, \
{ "Id": 424, "Name": "KBS 한민족방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "KBS 한민족방송", "RadioCh": 1008, "Icon_url": "http://i.imgur.com/S5YVPyx.png", "Source": "NAVER", "ServiceId": "815461" }, \
{ "Id": 425, "Name": "MBC 표준FM", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "MBC 표준FM", "RadioCh": 1009, "Icon_url": "http://i.imgur.com/E9OMdnO.png", "Source": "NAVER", "ServiceId": "815464" }, \
{ "Id": 426, "Name": "MBC FM4U", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "MBC FM4U", "RadioCh": 1010, "Icon_url": "http://i.imgur.com/csdszZD.png", "Source": "NAVER", "ServiceId": "815463" }, \
{ "Id": 428, "Name": "SBS 파워 FM", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "SBS 파워 FM", "RadioCh": 1012, "Icon_url": "http://i.imgur.com/7qcJ4bm.png", "Source": "NAVER", "ServiceId": "815467" }, \
{ "Id": 429, "Name": "SBS 러브 FM", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "SBS 러브 FM", "RadioCh": 1013, "Icon_url": "http://i.imgur.com/XHHHUZ1.png", "Source": "NAVER", "ServiceId": "815465" }, \
{ "Id": 430, "Name": "국악방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "국악방송", "RadioCh": 1014, "Icon_url": "http://i.imgur.com/qpbhUhF.png", "Source": "NAVER", "ServiceId": "2891853" }, \
{ "Id": 431, "Name": "극동방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "극동방송", "RadioCh": 1015, "Icon_url": "http://i.imgur.com/PlqBFtV.png", "Source": "NAVER", "ServiceId": "2074616" }, \
{ "Id": 432, "Name": "BBS 불교방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "BBS 불교방송", "RadioCh": 1016, "Icon_url": "http://i.imgur.com/B34jpmo.png", "Source": "NAVER", "ServiceId": "815448" }, \
{ "Id": 433, "Name": "CBS 표준FM", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "CBS 표준FM", "RadioCh": 1017, "Icon_url": "http://i.imgur.com/yBNo2mS.png", "Source": "NAVER", "ServiceId": "815451" }, \
{ "Id": 434, "Name": "CBS 음악FM", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "CBS 음악FM", "RadioCh": 1018, "Icon_url": "http://i.imgur.com/yBNo2mS.png", "Source": "NAVER", "ServiceId": "815449" }, \
{ "Id": 435, "Name": "KFM 경기방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "KFM 경기방송", "RadioCh": 1019, "Icon_url": "http://i.imgur.com/8hSikAY.png", "Source": "NAVER", "ServiceId": "1974893" }, \
{ "Id": 436, "Name": "cpbc 평화방송", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "cpbc 평화방송", "RadioCh": 1020, "Icon_url": "http://i.imgur.com/G5fTCL3.png", "Source": "NAVER", "ServiceId": "1974894" }, \
{ "Id": 437, "Name": "TBS FM", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "TBS FM", "RadioCh": 1021, "Icon_url": "http://i.imgur.com/9RxxTSi.png", "Source": "NAVER", "ServiceId": "815468" }, \
{ "Id": 438, "Name": "YTN NEWS FM", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "YTN NEWS FM", "RadioCh": 1022, "Icon_url": "http://i.imgur.com/dSC3YPR.png", "Source": "NAVER", "ServiceId": "2074615" }, \
{ "Id": 439, "Name": "원음방송", "DLIVE Name": "원음방송", "DLIVECh": 159, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "원음방송", "RadioCh": 1023, "Icon_url": "http://i.imgur.com/mKWQE7z.png", "Source": "NAVER", "ServiceId": "5534687" }, \
{ "Id": 442, "Name": "AMC", "DLIVE Name": "", "DLIVECh": null, "KT Name": "AMC", "KTCh": 221, "LG Name": "", "LGCh": null, "SK Name": "AMC", "SKCh": 100, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/cJ1B77S.png", "Source": "SKB", "ServiceId": "199" }, \
{ "Id": 443, "Name": "TVA", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/tg99cob.png", "Source": "NAVER", "ServiceId": "814777" }, \
{ "Id": 444, "Name": "채널W", "DLIVE Name": "채널W", "DLIVECh": 148, "KT Name": "채널 W", "KTCh": 226, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/mtiKEkU.png", "Source": "NAVER", "ServiceId": "2097469" }, \
{ "Id": 445, "Name": "INSIGHT TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "INSIGHT TV", "SKCh": 74, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/eFTXeGV.png", "Source": "SKB", "ServiceId": "890" }, \
{ "Id": 446, "Name": "인도어스포츠", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "인도어스포츠", "SKCh": 139, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/cb58LLs.png", "Source": "SKB", "ServiceId": "129" }, \
{ "Id": 447, "Name": "시니어TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "시니어TV", "KTCh": 264, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/XaObZ8I.png", "Source": "KT", "ServiceId": "264" }, \
{ "Id": 450, "Name": "HGTV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "HGTV", "KTCh": 223, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/PR4w3i2.png", "Source": "KT", "ServiceId": "223" }, \
{ "Id": 451, "Name": "다빈치러닝", "DLIVE Name": "", "DLIVECh": null, "KT Name": "다빈치러닝", "KTCh": 224, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/mB9BL31.png", "Source": "KT", "ServiceId": "224" }, \
{ "Id": 452, "Name": "E! Entertainment", "DLIVE Name": "", "DLIVECh": null, "KT Name": "E! 엔터", "KTCh": 225, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/VqIgfE9.png", "Source": "KT", "ServiceId": "225" }, \
{ "Id": 454, "Name": "Discovery Science", "DLIVE Name": "", "DLIVECh": null, "KT Name": "DSC Science", "KTCh": 222, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/3UfATJL.png", "Source": "KT", "ServiceId": "222" }, \
{ "Id": 455, "Name": "Now제주TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "Now제주TV", "KTCh": 94, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/aIvnsge.png", "Source": "KT", "ServiceId": "94" }, \
{ "Id": 456, "Name": "Lifetime", "DLIVE Name": "Lifetime", "DLIVECh": 60, "KT Name": "Lifetime", "KTCh": 78, "LG Name": "라이프타임", "LGCh": 83, "SK Name": "라이프타임", "SKCh": 213, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/vh1U91w.png", "Source": "SKB", "ServiceId": "271" }, \
{ "Id": 457, "Name": "플레이런TV", "DLIVE Name": "플레이런TV", "DLIVECh": 215, "KT Name": "플레이런TV", "KTCh": 155, "LG Name": "플레이런TV", "LGCh": 161, "SK Name": "플레이런TV", "SKCh": 201, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/nrtT3Wm.png", "Source": "NAVER", "ServiceId": "814979" }, \
{ "Id": 458, "Name": "BET", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "BET", "SKCh": 239, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/cb61Swx.png", "Source": "SKB", "ServiceId": "788" }, \
{ "Id": 485, "Name": "etn 연예채널", "DLIVE Name": "EtN연예TV", "DLIVECh": 145, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/0BHBt0h.png", "Source": "NAVER", "ServiceId": "815229" }, \
{ "Id": 492, "Name": "스크린 골프존", "DLIVE Name": "", "DLIVECh": null, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "스크린 골프존", "SKCh": 138, "Radio Name": "", "RadioCh": null, "Icon_url": "https://i.imgur.com/oa1VbRv.png", "Source": "SKB", "ServiceId": "138" }, \
{ "Id": 493, "Name": "신기한나라TV", "DLIVE Name": "신기한나라TV", "DLIVECh": 219, "KT Name": "신기한나라TV", "KTCh": 162, "LG Name": "", "LGCh": null, "SK Name": "신기한나라TV", "SKCh": 188, "Radio Name": "", "RadioCh": null, "Icon_url": "https://i.imgur.com/qYzAXDx.png", "Source": "SKB", "ServiceId": "386" }, \
{ "Id": 494, "Name": "디자이어TV", "DLIVE Name": "디자이어TV", "DLIVECh": 100, "KT Name": "디자이어TV", "KTCh": 209, "LG Name": "디자이어TV", "LGCh": 294, "SK Name": "디자이어TV", "SKCh": 325, "Radio Name": "", "RadioCh": null, "Icon_url": "https://i.imgur.com/AocuvNk.png", "Source": "SKB", "ServiceId": "200" }, \
{ "Id": 495, "Name": "JTBC4", "DLIVE Name": "JTBC4", "DLIVECh": 84, "KT Name": "JTBC4", "KTCh": 128, "LG Name": "JTBC4", "LGCh": 92, "SK Name": "JTBC4", "SKCh": 222, "Radio Name": "", "RadioCh": null, "Icon_url": "https://i.imgur.com/EwThLNU.png", "Source": "SK", "ServiceId": "259" }, \
{ "Id": 496, "Name": "뽀요TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "뽀요TV", "KTCh": 153, "LG Name": "", "LGCh": null, "SK Name": "뽀요TV", "SKCh": 182, "Radio Name": "", "RadioCh": null, "Icon_url": "https://i.imgur.com/i3Eufuy.png", "Source": "SK", "ServiceId": "387" }, \
{ "Id": 497, "Name": "캐리TV", "DLIVE Name": "", "DLIVECh": null, "KT Name": "캐리TV", "KTCh": 143, "LG Name": "", "LGCh": null, "SK Name": "캐리TV", "SKCh": 183, "Radio Name": "", "RadioCh": null, "Icon_url": "https://i.imgur.com/rHO0O4P.png", "Source": "SK", "ServiceId": "388" }, \
{ "Id": 498, "Name": "케이블TV VOD", "DLIVE Name": "케이블TV VOD", "DLIVECh": 0, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "", "ServiceId": "" }, \
{ "Id": 499, "Name": "지역채널", "DLIVE Name": "지역채널", "DLIVECh": 1, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "", "ServiceId": "" }, \
{ "Id": 500, "Name": "TVasia", "DLIVE Name": "TVasia", "DLIVECh": 62, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "", "ServiceId": "" }, \
{ "Id": 501, "Name": "EXF Plus", "DLIVE Name": "EXF Plus", "DLIVECh": 69, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "", "ServiceId": "" }, \
{ "Id": 502, "Name": "Cook TV", "DLIVE Name": "Cook TV", "DLIVECh": 94, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "SKB", "ServiceId": "289" }, \
{ "Id": 503, "Name": "Koreasports", "DLIVE Name": "Koreasports", "DLIVECh": 123, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "", "ServiceId": "" }, \
{ "Id": 504, "Name": "다큐원", "DLIVE Name": "다큐원", "DLIVECh": 149, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "SKB", "ServiceId": "257" }, \
{ "Id": 505, "Name": "Mezzo", "DLIVE Name": "Mezzo", "DLIVECh": 162, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "", "ServiceId": "" }, \
{ "Id": 506, "Name": "LifeU UHD", "DLIVE Name": "LifeU UHD", "DLIVECh": 164, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "https://i.imgur.com/3VJOGoI.png", "Source": "SKB", "ServiceId": "277" }, \
{ "Id": 507, "Name": "SBS Plus UHD", "DLIVE Name": "SBS Plus UHD", "DLIVECh": 166, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "http://i.imgur.com/asfyrTm.png", "Source": "SK", "ServiceId": "901" }, \
{ "Id": 508, "Name": "산업방송", "DLIVE Name": "산업방송", "DLIVECh": 175, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "", "ServiceId": "" }, \
{ "Id": 509, "Name": "아프리카TV", "DLIVE Name": "아프리카TV", "DLIVECh": 177, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "", "ServiceId": "" }, \
{ "Id": 510, "Name": "english gem", "DLIVE Name": "english gem", "DLIVECh": 199, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "SKB", "ServiceId": "390" }, \
{ "Id": 511, "Name": "챔프", "DLIVE Name": "챔프", "DLIVECh": 200, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "", "ServiceId": "" }, \
{ "Id": 512, "Name": "Wee TV", "DLIVE Name": "Wee TV", "DLIVECh": 216, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "", "ServiceId": "" }, \
{ "Id": 513, "Name": "케이블TV VOD 키즈", "DLIVE Name": "케이블TV VOD 키즈", "DLIVECh": 218, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "", "ServiceId": "" }, \
{ "Id": 514, "Name": "한국청소년방송", "DLIVE Name": "한국청소년방송", "DLIVECh": 230, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "", "ServiceId": "" }, \
{ "Id": 515, "Name": "큐피드", "DLIVE Name": "큐피드", "DLIVECh": 232, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "", "ServiceId": "" }, \
{ "Id": 516, "Name": "시민방송", "DLIVE Name": "시민방송", "DLIVECh": 253, "KT Name": "", "KTCh": null, "LG Name": "", "LGCh": null, "SK Name": "", "SKCh": null, "Radio Name": "", "RadioCh": null, "Icon_url": "", "Source": "", "ServiceId": "" } \
] \
'
Settingstring = '\
{ \
    "###_COMMENT_###" : "", \
    "###_COMMENT_###" : "epg 정보를 가져오는 설정 파일", \
    "###_COMMENT_###" : "사용하는 ISP 선택 (ALL, KT, LG, SK, DLIVE)", \
    "MyISP": "ALL", \
    "###_COMMENT_###" : "### # My Channel EPG 정보 가져오는 채널 ID ###", \
    "###_COMMENT_###" : "### 채널 ID를 , 로 구분하여 입력 ###", \
    "MyChannels" : "60, 110, 111, 122, 164", \
    "###_COMMENT_###" : "output 셋팅은 (d, o, s) 셋중에 하나로 선택한다", \
    "###_COMMENT_###" : " d - EPG 정보 화면 출력", \
    "###_COMMENT_###" : " o - EPG 정보 파일로 저장", \
    "###_COMMENT_###" : " s - EPG 정보 소켓으로 출력", \
    "output": "d", \
    "###_COMMENT_###" : "### TV channel icon url (ex : http://www.example.com/Channels) ###", \
    "default_icon_url": "", \
    "###_COMMENT_###" : "### 제목에 재방송 정보 출력 ###", \
    "default_rebroadcast": "n", \
    "###_COMMENT_###" : "#### 제목에 회차정보 출력 ###",      \
    "default_episode" : "y", \
    "###_COMMENT_###" : "### EPG 정보 추가 출력 ###", \
    "default_verbose" : "y", \
    "###_COMMENT_###" : "### XMLTV_NS 정보 추가 출력 ###", \
    "default_xmltvns" : "n", \
    "###_COMMENT_###" : "### epg 데이터 가져오는 기간으로 1에서 7까지 설정가능 ###", \
    "default_fetch_limit" : "2", \
    "###_COMMENT_###" : "### epg 저장시 기본 저장 이름 (ex: /home/tvheadend/xmltv.xml) ###", \
    "default_xml_file" : "xmltv.xml", \
    "###_COMMENT_###" : "### # External XMLTV 사용시 기본 소켓 이름 (ex: /home/tvheadend/xmltv.sock) ###", \
    "default_xml_socket" : "xmltv.sock", \
    "###_COMMENT_###" : "" \
} \
'
try:
    imp.find_module('bs4')
    from bs4 import BeautifulSoup, SoupStrainer
except ImportError:
    print("Error : ", "BeautifulSoup 모듈이 설치되지 않았습니다.", file=sys.stderr)
    sys.exit()
try:
    imp.find_module('lxml')
    from lxml import html
except ImportError:
    print("Error : ", "lxml 모듈이 설치되지 않았습니다.", file=sys.stderr)
    sys.exit()
try:
    imp.find_module('requests')
    import requests
except ImportError:
    print("Error : ", "requests 모듈이 설치되지 않았습니다.", file=sys.stderr)
    sys.exit()

reload(sys)
sys.setdefaultencoding('utf-8')

if not sys.version_info[:2] == (2, 7):
    print("Error : ", "python 2.7 버전이 필요합니다.", file=sys.stderr)
    sys.exit()

# Set variable
__version__ = '1.2.6'
debug = False
today = datetime.date.today()
ua = {'User-Agent': 'Mozilla/5.0 (Windows NT 6.3; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.116'}
timeout = 5
htmlparser = 'lxml'
CHANNEL_ERROR = ' 존재하지 않는 채널입니다.'
CONTENT_ERROR = ' EPG 정보가 없습니다.'
HTTP_ERROR = ' EPG 정보를 가져오는데 문제가 있습니다.'
SOCKET_ERROR = 'xmltv.sock 파일을 찾을 수 없습니다.'
JSON_FILE_ERROR = 'json 파일을 읽을 수 없습니다.'
JSON_SYNTAX_ERROR = 'json 파일 형식이 잘못되었습니다.'

# Get epg data
def getEpg():
    Channelfile = 'Channel.json'
    ChannelInfos = []
    try:
        with open(Channelfile) as f: # Read Channel Information file
            Channeldatajson = json.load(f)
    except EnvironmentError:
        Channeldatajson = json.loads(Channelstring, encoding='utf-8')
    except ValueError:
        Channeldatajson = json.loads(Channelstring, encoding='utf-8')
    print('<?xml version="1.0" encoding="UTF-8"?>')
    print('<!DOCTYPE tv SYSTEM "xmltv.dtd">\n')
    print('<tv generator-info-name="epg2xml ' + __version__ + '">')
# My Channel 정의
    MyChannelInfo = []
    if MyChannels :
        for MyChannel in MyChannels.split(','):
            MyChannelInfo.append(int(MyChannel.strip()))
    for Channeldata in Channeldatajson: #Get Channel & Print Channel info
        if Channeldata['Id'] in MyChannelInfo:
            ChannelId = Channeldata['Id']
            ChannelName = escape(Channeldata['Name'])
            ChannelSource = Channeldata['Source']
            ChannelServiceId = Channeldata['ServiceId']
            ChannelIconUrl = escape(Channeldata['Icon_url'])
            if MyISP != "ALL" and Channeldata[MyISP+'Ch'] is not None:
                ChannelInfos.append([ChannelId,  ChannelName, ChannelSource, ChannelServiceId])
                ChannelNumber = str(Channeldata[MyISP+'Ch']);
                ChannelISPName = escape(Channeldata[MyISP+' Name'])
                print('  <channel id="%s">' % (ChannelId))
                print('    <display-name>%s</display-name>' % (ChannelName))
                print('    <display-name>%s</display-name>' % (ChannelISPName))
                print('    <display-name>%s</display-name>' % (ChannelNumber))
                print('    <display-name>%s</display-name>' % (ChannelNumber+' '+ChannelISPName))
                if IconUrl:
                    print('    <icon src="%s/%s.png" />' % (IconUrl, ChannelId))
                else :
                    print('    <icon src="%s" />' % (ChannelIconUrl))
                print('  </channel>')
            elif MyISP == "ALL":
                ChannelInfos.append([ChannelId,  ChannelName, ChannelSource, ChannelServiceId])
                print('  <channel id="%s">' % (ChannelId))
                print('    <display-name>%s</display-name>' % (ChannelName))
                if IconUrl:
                    print('    <icon src="%s/%s.png" />' % (IconUrl, ChannelId))
                else :
                    print('    <icon src="%s" />' % (ChannelIconUrl))
                print('  </channel>')

    # Print Program Information
    for ChannelInfo in ChannelInfos:
        ChannelId = ChannelInfo[0]
        ChannelName =  ChannelInfo[1]
        ChannelSource =  ChannelInfo[2]
        ChannelServiceId =  ChannelInfo[3]
        if(debug) : printLog(ChannelName + ' 채널 EPG 데이터를 가져오고 있습니다')
        if ChannelSource == 'KT':
            GetEPGFromKT(ChannelInfo)
        elif ChannelSource == 'LG':
            GetEPGFromLG(ChannelInfo)
        elif ChannelSource == 'SK':
            GetEPGFromSK(ChannelInfo)
        elif ChannelSource == 'SKB':
            GetEPGFromSKB(ChannelInfo)
        elif ChannelSource == 'NAVER':
            GetEPGFromNaver(ChannelInfo)
    print('</tv>')

# Get EPG data from KT
def GetEPGFromKT(ChannelInfo):
    ChannelId = ChannelInfo[0]
    ChannelName = ChannelInfo[1]
    ServiceId =  ChannelInfo[3]
    epginfo = []
    url = 'http://tv.kt.com/tv/channel/pSchedule.asp'
    for k in range(period):
        day = today + datetime.timedelta(days=k)
        params = {'ch_type':'1', 'view_type':'1', 'service_ch_no':ServiceId, 'seldate':day.strftime('%Y%m%d')}
        try:
            response = requests.post(url, data=params, headers=ua, timeout=timeout)
            response.raise_for_status()
            html_data = response.content
            data = unicode(html_data, 'euc-kr', 'ignore').encode('utf-8', 'ignore')
            strainer = SoupStrainer('tbody')
            soup = BeautifulSoup(data, htmlparser, parse_only=strainer, from_encoding='utf-8')
            html = soup.find_all('tr') if soup.find('tbody') else ''
            if(html):
                for row in html:
                    for cell in [row.find_all('td')]:
                        startTime = endTime = programName = subprogramName = desc = actors = producers = category = episode = ''
                        rebroadcast = False
                        for minute, program, category in zip(cell[1].find_all('p'), cell[2].find_all('p'), cell[3].find_all('p')):
                            startTime = str(day) + ' ' + cell[0].text.strip() + ':' + minute.text.strip()
                            startTime = datetime.datetime(*(time.strptime(startTime, '%Y-%m-%d %H:%M')[0:6]))
                            startTime = startTime.strftime('%Y%m%d%H%M%S')
                            programName = program.text.replace('방송중 ', '').strip()
                            category = category.text.strip()
                            for image in [program.find_all('img', alt=True)]:
                                rating = 0
                                grade = re.match('([\d,]+)',image[0]['alt'])
                                if not (grade is None): rating = int(grade.group(1))
                                else: rating = 0
                            #ChannelId, startTime, programName, subprogramName, desc, actors, producers, category, episode, rebroadcast, rating
                            epginfo.append([ChannelId, startTime, programName, subprogramName, desc, actors, producers, category, episode, rebroadcast, rating])
                            time.sleep(0.001)
            else:
                if(debug): printError(ChannelName + CONTENT_ERROR)
                else: pass
        except (requests.exceptions.RequestException) as e:
            if(debug): printError(ChannelName + str(e))
            else: pass
    if(epginfo) :
        epgzip(epginfo)

# Get EPG data from LG
def GetEPGFromLG(ChannelInfo):
    ChannelId = ChannelInfo[0]
    ChannelName = ChannelInfo[1]
    ServiceId =  ChannelInfo[3]
    epginfo = []
    url = 'http://www.uplus.co.kr/css/chgi/chgi/RetrieveTvSchedule.hpi'
    for k in range(period):
        day = today + datetime.timedelta(days=k)
        params = {'chnlCd': ServiceId, 'evntCmpYmd': day.strftime('%Y%m%d')}
        try:
            response = requests.post(url, data=params, headers=ua, timeout=timeout)
            response.raise_for_status()
            html_data = response.content
            data = unicode(html_data, 'euc-kr', 'ignore').encode('utf-8', 'ignore')
            data = data.replace('<재>', '&lt;재&gt;').replace(' [..','').replace(' (..', '')
            strainer = SoupStrainer('table')
            soup = BeautifulSoup(data, htmlparser, parse_only=strainer, from_encoding='utf-8')
            html = soup.find('table').tbody.find_all('tr') if soup.find('table') else ''
            if(html):
                for row in html:
                    for cell in [row.find_all('td')]:
                        startTime = endTime = programName = subprogramName = desc = actors = producers = category = episode = ''
                        rebroadcast = False
                        rating = 0
                        startTime = str(day) + ' ' + cell[0].text
                        startTime = datetime.datetime(*(time.strptime(startTime, '%Y-%m-%d %H:%M')[0:6]))
                        startTime = startTime.strftime('%Y%m%d%H%M%S')
                        rating = 0 if cell[1].find('span', {'class': 'tag cte_all'}).text.strip()=="All" else int(cell[1].find('span', {'class': 'tag cte_all'}).text.strip())
                        cell[1].find('span', {'class': 'tagGroup'}).decompose()
                        pattern = '(<재>)?\s?(?:\[.*?\])?(.*?)(?:\[(.*)\])?\s?(?:\(([\d,]+)회\))?$'
                        matches = re.match(pattern, cell[1].text.strip().decode('string_escape'))
                        if not (matches is None):
                            programName = matches.group(2).strip() if matches.group(2) else ''
                            subprogramName = matches.group(3).strip() if matches.group(3) else ''
                            episode = matches.group(4) if matches.group(4) else ''
                            rebroadcast = True if matches.group(1) else False
                        category =  cell[2].text.strip()
                        #ChannelId, startTime, programName, subprogramName, desc, actors, producers, category, episode, rebroadcast, rating
                        epginfo.append([ChannelId, startTime, programName, subprogramName, desc, actors, producers, category, episode, rebroadcast, rating])
                        time.sleep(0.001)
            else:
                if(debug): printError(ChannelName + CONTENT_ERROR)
                else: pass
        except (requests.exceptions.RequestException) as e:
            if(debug): printError(ChannelName + str(e))
            else: pass
    if(epginfo) :
        epgzip(epginfo)

# Get EPG data from SK
def GetEPGFromSK(ChannelInfo):
    ChannelId = ChannelInfo[0]
    ChannelName = ChannelInfo[1]
    ServiceId =  ChannelInfo[3]
    lastday = today + datetime.timedelta(days=period-1)
    url = 'http://m.btvplus.co.kr/common/inc/IFGetData.do'
    params = {'variable': 'IF_LIVECHART_DETAIL', 'pcode':'|^|start_time=' + today.strftime('%Y%m%d') + '00|^|end_time='+ lastday.strftime('%Y%m%d') + '24|^|svc_id=' + str(ServiceId)}
    try:
        response = requests.post(url, data=params, headers=ua, timeout=timeout)
        response.raise_for_status()
        json_data = response.text
        try:
            data = json.loads(json_data, encoding='utf-8')
            if (data['channel'] is None) :
                 if(debug): printError(ChannelName + CONTENT_ERROR)
                 else: pass
            else :
                programs = data['channel']['programs']
                for program in programs:
                    startTime = endTime = programName = subprogramName = desc = actors = producers = category = episode = ''
                    rebroadcast = False
                    rating = 0
                    programName = program['programName'].replace('...', '>').encode('utf-8')
                    pattern = '^(.*?)(?:\s*[\(<]([\d,회]+)[\)>])?(?:\s*<([^<]*?)>)?(\((재)\))?$'
                    matches = re.match(pattern, programName)
                    if not (matches is None):
                        programName = matches.group(1).strip() if matches.group(1) else ''
                        subprogramName = matches.group(3).strip() if matches.group(3) else ''
                        episode = matches.group(2).replace('회', '') if matches.group(2) else ''
                        episode = '' if episode== '0' else episode
                        rebroadcast = True if matches.group(5) else False
                    startTime = datetime.datetime.fromtimestamp(int(program['startTime'])/1000)
                    startTime = startTime.strftime('%Y%m%d%H%M%S')
                    endTime = datetime.datetime.fromtimestamp(int(program['endTime'])/1000)
                    endTime = endTime.strftime('%Y%m%d%H%M%S')
                    desc = program['synopsis'] if program['synopsis'] else ''
                    actors = program['actorName'].replace('...','').strip(', ') if program['actorName'] else ''
                    producers = program['directorName'].replace('...','').strip(', ')  if program['directorName'] else ''
                    if not (program['mainGenreName'] is None) :
                        category = program['mainGenreName']
                    rating = int(program['ratingCd']) if program['ratingCd'] else 0
                    programdata = {'channelId':ChannelId, 'startTime':startTime, 'endTime':endTime, 'programName':programName, 'subprogramName':subprogramName, 'desc':desc, 'actors':actors, 'producers':producers, 'category':category, 'episode':episode, 'rebroadcast':rebroadcast, 'rating':rating}
                    writeProgram(programdata)
                    time.sleep(0.001)
        except ValueError:
            if(debug): printError(ChannelName + CONTENT_ERROR)
            else: pass
    except (requests.exceptions.RequestException) as e:
        if(debug): printError(ChannelName + str(e))
        else: pass

#Get EPG data from SKB
def GetEPGFromSKB(ChannelInfo):
    ChannelId = ChannelInfo[0]
    ChannelName = ChannelInfo[1]
    ServiceId =  ChannelInfo[3]
    url = 'http://m.skbroadband.com/content/realtime/Channel_List.do'
    epginfo = []
    for k in range(period):
        day = today + datetime.timedelta(days=k)
        params = {'key_depth2': ServiceId, 'key_depth3': day.strftime('%Y%m%d')}
        try:
            response = requests.get(url, params=params, headers=ua, timeout=timeout)
            response.raise_for_status()
            html_data = response.content
            data = unicode(html_data, 'euc-kr', 'ignore').encode('utf-8', 'ignore')
	    data = re.sub('EUC-KR', 'utf-8', data)
            data = re.sub('<!--(.*?)-->', '', data, 0, re.I|re.S)
            data = re.sub('<span class="round_flag flag02">(.*?)</span>', '', data)
            data = re.sub('<span class="round_flag flag03">(.*?)</span>', '', data)
            data = re.sub('<span class="round_flag flag04">(.*?)</span>', '', data)
            data = re.sub('<span class="round_flag flag09">(.*?)</span>', '', data)
            data = re.sub('<span class="round_flag flag10">(.*?)</span>', '', data)
            data = re.sub('<span class="round_flag flag11">(.*?)</span>', '', data)
            data = re.sub('<span class="round_flag flag12">(.*?)</span>', '', data)
            data = re.sub('<strong class="hide">프로그램 안내</strong>', '', data)
	    data = re.sub('<p class="cont">(.*)', partial(replacement, tag='p') , data)
	    data = re.sub('<p class="tit">(.*)', partial(replacement, tag='p') , data)
            strainer = SoupStrainer('div', {'id':'uiScheduleTabContent'})
	    soup = BeautifulSoup(data, htmlparser, parse_only=strainer, from_encoding='utf-8')
            html =  soup.find_all('li',{'class':'list'}) if soup.find_all('li') else ''
            if(html):
                for row in html:
                    startTime = endTime = programName = subprogramName = desc = actors = producers = category = episode = ''
                    rebroadcast = False
                    rating = 0
                    startTime = str(day) + ' ' + row.find('p', {'class':'time'}).text
                    startTime = datetime.datetime(*(time.strptime(startTime, '%Y-%m-%d %H:%M')[0:6]))
                    startTime = startTime.strftime('%Y%m%d%H%M%S')
                    cell = row.find('p', {'class':'cont'})
	            grade = row.find('i', {'class':'hide'})
		    if not(grade is None) :
                       rating = int(grade.text.decode('string_escape').replace('세 이상','').strip())

                    if(cell):
                        if cell.find('span'):
                            cell.span.decompose()
                        cell = cell.text.decode('string_escape').strip()
                        pattern = "^(.*?)(\(([\d,]+)회\))?(<(.*)>)?(\((재)\))?$"
                        matches = re.match(pattern, cell)

                        if not(matches is None) :
                            programName = matches.group(1) if matches.group(1) else ''
                            subprogramName = matches.group(5) if matches.group(5) else ''
                            rebroadcast = True if matches.group(7) else False
                            episode = matches.group(3) if matches.group(3) else ''

                    #ChannelId, startTime, programName, subprogramName, desc, actors, producers, category, episode, rebroadcast, rating
                    epginfo.append([ChannelId, startTime, programName, subprogramName, desc, actors, producers, category, episode, rebroadcast, rating])
                    time.sleep(0.001)
            else:
                if(debug): printError(ChannelName + CONTENT_ERROR)
                else: pass
        except (requests.exceptions.RequestException) as e:
            if(debug): printError(ChannelName + str(e))
            else: pass
    if(epginfo) :
        epgzip(epginfo)

# Get EPG data from Naver
def GetEPGFromNaver(ChannelInfo):
    ChannelId = ChannelInfo[0]
    ChannelName = ChannelInfo[1]
    ServiceId =  ChannelInfo[3]
    epginfo = []
    totaldate = []
    url = 'https://search.naver.com/p/csearch/content/batchrender_ssl.nhn'
    for k in range(period):
        day = today + datetime.timedelta(days=k)
        totaldate.append(day.strftime('%Y%m%d'))
    params = {'_callback': 'epg', 'fileKey': 'single_schedule_channel_day', 'pkid': '66', 'u1': 'single_schedule_channel_day', 'u2': ','.join(totaldate), 'u3': today.strftime('%Y%m%d'), 'u4': period, 'u5': ServiceId, 'u6': '1', 'u7': ChannelName + '편성표', 'u8': ChannelName + '편성표', 'where': 'nexearch'}
    try:
        response = requests.get(url, params=params, headers=ua, timeout=timeout)
        response.raise_for_status()
        json_data = re.sub(re.compile("/\*.*?\*/",re.DOTALL ) ,"" ,response.text.split("epg(")[1].strip(");").strip())
        try:
            data = json.loads(json_data, encoding='utf-8')
            for i, date in enumerate(data['displayDates']):
                for j in range(0,24):
                    for program in data['schedules'][j][i]:
                        startTime = endTime = programName = subprogramName = desc = actors = producers = category = episode = ''
                        rebroadcast = False
                        rating = 0
                        programName = unescape(program['title'])
                        startTime = date['date'] + ' ' + program['startTime']
                        startTime = datetime.datetime(*(time.strptime(startTime, '%Y%m%d %H:%M')[0:6]))
                        startTime = startTime.strftime('%Y%m%d%H%M%S')
                        episode = program['episode'].replace('회','')
                        rebroadcast = program['isRerun']
                        rating = program['grade']
                         #ChannelId, startTime, programName, subprogramName, desc, actors, producers, category, episode, rebroadcast, rating
                        epginfo.append([ChannelId, startTime, programName, subprogramName, desc, actors, producers, category, episode, rebroadcast, rating])
                        time.sleep(0.001)
        except ValueError:
             if(debug): printError(ChannelName + CONTENT_ERROR)
             else: pass
    except (requests.RequestException) as e:
        if(debug): printError(ChannelName + str(e))
        else: pass
    if(epginfo) :
        epgzip(epginfo)

# Zip epginfo
def epgzip(epginfo):
    epginfo = iter(epginfo)
    epg1 = next(epginfo)
    for epg2 in epginfo:
        programdata = {}
        ChannelId = epg1[0]
        startTime = epg1[1] if epg1[1] else ''
        endTime = epg2[1] if epg2[1] else ''
        programName = epg1[2] if epg1[2] else ''
        subprogramName = epg1[3] if epg1[3] else ''
        desc = epg1[4] if epg1[4] else ''
        actors = epg1[5] if epg1[5] else ''
        producers = epg1[6] if epg1[6] else ''
        category = epg1[7] if epg1[7] else ''
        episode = epg1[8] if epg1[8] else ''
        rebroadcast = True if epg1[9] else False
        rating = int(epg1[10]) if epg1[10] else 0
        programdata = {'channelId':ChannelId, 'startTime':startTime, 'endTime':endTime, 'programName':programName, 'subprogramName':subprogramName, 'desc':desc, 'actors':actors, 'producers':producers, 'category':category, 'episode':episode, 'rebroadcast':rebroadcast, 'rating':rating}
        writeProgram(programdata)
        epg1 = epg2

# Write Program
def writeProgram(programdata):
    ChannelId = programdata['channelId']
    startTime = programdata['startTime']
    endTime = programdata['endTime']
    programName = escape(programdata['programName']).strip()
    subprogramName = escape(programdata['subprogramName']).strip()
    matches = re.match('(.*) \(?(\d+부)\)?', unescape(programName.encode('utf-8', 'ignore')))
    if not(matches is None):
        programName = escape(matches.group(1)).strip();
        subprogramName = escape(matches.group(2)) + ' ' + subprogramName
        subprogramName = subprogramName.strip()
    if programName is None:
        programName = subprogramName
    actors = escape(programdata['actors'])
    producers = escape(programdata['producers'])
    category = escape(programdata['category'])
    episode = programdata['episode']
    if episode:
        try:
            episode_ns = int(episode) - 1
            episode_ns = '0'+ '.' +  str(episode_ns) + '.' + '0' + '/' + '0'
        except ValueError as ex:
            episode_ns = int(episode.split(',', 1)[0]) - 1
            episode_ns = '0'+ '.' +  str(episode_ns) + '.' + '0' + '/' + '0'
        episode_on = episode
    rebroadcast = programdata['rebroadcast']
    if episode and addepisode  == 'y': programName = programName + ' ('+ str(episode) + '회)'
    if rebroadcast  == True and addrebroadcast == 'y' : programName = programName + ' (재)'
    if programdata['rating'] == 0 :
        rating = '전체 관람가'
    else :
        rating = '%s세 이상 관람가' % (programdata['rating'])
    if addverbose == 'y':
        desc = programName
        if subprogramName : desc = desc + '\n부제 : ' + subprogramName
        if rebroadcast == True and addrebroadcast == 'y' : desc = desc + '\n방송 : 재방송'
        if episode : desc = desc + '\n회차 : ' + str(episode) + '회'
        if category : desc = desc + '\n장르 : ' + category
        if actors : desc = desc + '\n출연 : ' + actors.strip()
        if producers : desc = desc + '\n제작 : ' + producers.strip()
        desc = desc + '\n등급 : ' + rating
    else:
        desc =''
    if programdata['desc'] : desc = desc + '\n' + escape(programdata['desc'])
    desc = re.sub(' +',' ', desc)
    contentTypeDict={'교양':'Arts / Culture (without music)', '만화':'Cartoons / Puppets', '교육':'Education / Science / Factual topics', '취미':'Leisure hobbies', '드라마':'Movie / Drama', '영화':'Movie / Drama', '음악':'Music / Ballet / Dance', '뉴스':'News / Current affairs', '다큐':'Documentary', '라이프':'Documentary', '시사/다큐':'Documentary', '연예':'Show / Game show', '스포츠':'Sports', '홈쇼핑':'Advertisement / Shopping'}
    contentType = ''
    for key, value in contentTypeDict.iteritems():
        if key in category:
            contentType = value
    print('  <programme start="%s +0900" stop="%s +0900" channel="%s">' % (startTime, endTime, ChannelId))
    print('    <title lang="kr">%s</title>' % (programName))
    if subprogramName :
        print('    <sub-title lang="kr">%s</sub-title>' % (subprogramName))
    if addverbose=='y' :
        print('    <desc lang="kr">%s</desc>' % (desc))
        if actors or producers:
            print('    <credits>')
            if actors:
                for actor in actors.split(','):
                    if actor.strip(): print('      <actor>%s</actor>' % (actor.strip()))
            if producers:
                for producer in producers.split(','):
                    if producer.strip(): print('      <producer>%s</producer>' % (producer).strip())
            print('    </credits>')
    if category: print('    <category lang="kr">%s</category>' % (category))
    if contentType: print('    <category lang="en">%s</category>' % (contentType))
    if episode and addxmltvns == 'y' : print('    <episode-num system="xmltv_ns">%s</episode-num>' % (episode_ns))
    if episode and addxmltvns != 'y' : print('    <episode-num system="onscreen">%s</episode-num>' % (episode_on))
    if rebroadcast: print('    <previously-shown />')
    if rating:
        print('    <rating system="KMRB">')
        print('      <value>%s</value>' % (rating))
        print('    </rating>')
    print('  </programme>')

def printLog(*args):
    print(*args, file=sys.stderr)

def printError(*args):
    print("Error : ", *args, file=sys.stderr)

def replacement(match, tag):
    if not(match is None):
        tag = tag.strip()
        programName = unescape(match.group(1)).replace('<','&lt;').replace('>','&gt;').strip()
        programName = '<'+ tag + ' class="cont">' + programName
        return programName
    else:
        return '';

Settingfile = 'epg2xml.json'
ChannelInfos = []
try:
    with open(Settingfile) as f: # Read Channel Information file
        Settings = json.load(f)
        MyISP = Settings['MyISP'] if 'MyISP' in Settings else 'ALL'
        MyChannels = Settings['MyChannels'] if 'MyChannels' in Settings else ''
        default_output = Settings['output'] if 'output' in Settings else 'd'
        default_xml_file = Settings['default_xml_file'] if 'default_xml_file' in Settings else 'xmltv.xml'
        default_xml_socket = Settings['default_xml_socket'] if 'default_xml_socket' in Settings else 'xmltv.sock'
        default_icon_url = Settings['default_icon_url'] if 'default_icon_url' in Settings else None
        default_fetch_limit = Settings['default_fetch_limit'] if 'default_fetch_limit' in Settings else '2'
        default_rebroadcast = Settings['default_rebroadcast'] if 'default_rebroadcast' in Settings else 'y'
        default_episode = Settings['default_episode'] if 'default_episode' in Settings else 'y'
        default_verbose = Settings['default_verbose'] if 'default_verbose' in Settings else 'n'
        default_xmltvns = Settings['default_xmltvns'] if 'default_xmltvns' in Settings else 'n'
except EnvironmentError:
    Settings = json.loads(Settingstring, encoding='utf-8')
    MyISP = Settings['MyISP'] if 'MyISP' in Settings else 'ALL'
    MyChannels = Settings['MyChannels'] if 'MyChannels' in Settings else ''
    default_output = Settings['output'] if 'output' in Settings else 'd'
    default_xml_file = Settings['default_xml_file'] if 'default_xml_file' in Settings else 'xmltv.xml'
    default_xml_socket = Settings['default_xml_socket'] if 'default_xml_socket' in Settings else 'xmltv.sock'
    default_icon_url = Settings['default_icon_url'] if 'default_icon_url' in Settings else None
    default_fetch_limit = Settings['default_fetch_limit'] if 'default_fetch_limit' in Settings else '2'
    default_rebroadcast = Settings['default_rebroadcast'] if 'default_rebroadcast' in Settings else 'y'
    default_episode = Settings['default_episode'] if 'default_episode' in Settings else 'y'
    default_verbose = Settings['default_verbose'] if 'default_verbose' in Settings else 'n'
    default_xmltvns = Settings['default_xmltvns'] if 'default_xmltvns' in Settings else 'n'
except ValueError:
    Settings = json.loads(Settingstring, encoding='utf-8')
    MyISP = Settings['MyISP'] if 'MyISP' in Settings else 'ALL'
    MyChannels = Settings['MyChannels'] if 'MyChannels' in Settings else ''
    default_output = Settings['output'] if 'output' in Settings else 'd'
    default_xml_file = Settings['default_xml_file'] if 'default_xml_file' in Settings else 'xmltv.xml'
    default_xml_socket = Settings['default_xml_socket'] if 'default_xml_socket' in Settings else 'xmltv.sock'
    default_icon_url = Settings['default_icon_url'] if 'default_icon_url' in Settings else None
    default_fetch_limit = Settings['default_fetch_limit'] if 'default_fetch_limit' in Settings else '2'
    default_rebroadcast = Settings['default_rebroadcast'] if 'default_rebroadcast' in Settings else 'y'
    default_episode = Settings['default_episode'] if 'default_episode' in Settings else 'y'
    default_verbose = Settings['default_verbose'] if 'default_verbose' in Settings else 'n'
    default_xmltvns = Settings['default_xmltvns'] if 'default_xmltvns' in Settings else 'n'

parser = argparse.ArgumentParser(description = 'EPG 정보를 출력하는 방법을 선택한다')
argu1 = parser.add_argument_group(description = 'IPTV 선택')
argu1.add_argument('-i', dest = 'MyISP', choices = ['ALL', 'KT', 'LG', 'SK', 'DLIVE'], help = '사용하는 IPTV : ALL, KT, LG, SK, DLIVE', default = MyISP)
argu2 = parser.add_argument_group(description = 'Channel 선택')
argu2.add_argument('-c', dest = 'MyChannels', metavar = 'ID,...', nargs = '?', help = 'EPG 정보 가져오는 채널 ID', default = MyChannels)
argu3 = parser.add_mutually_exclusive_group()
argu3.add_argument('-v', '--version', action = 'version', version = '%(prog)s version : ' + __version__)
argu3.add_argument('-d', '--display', action = 'store_true', help = 'EPG 정보 화면출력')
argu3.add_argument('-o', '--outfile', metavar = default_xml_file, nargs = '?', const = default_xml_file, help = 'EPG 정보 저장')
argu3.add_argument('-s', '--socket', metavar = default_xml_socket, nargs = '?', const = default_xml_socket, help = 'xmltv.sock(External: XMLTV)로 EPG정보 전송')
argu4 = parser.add_argument_group('추가옵션')
argu4.add_argument('--icon', dest = 'icon', metavar = "http://www.example.com/icon", help = '채널 아이콘 URL, 기본값: '+ default_icon_url, default = default_icon_url)
argu4.add_argument('-l', '--limit', dest = 'limit', type=int, metavar = "1-7", choices = range(1,8), help = 'EPG 정보를 가져올 기간, 기본값: '+ str(default_fetch_limit), default = default_fetch_limit)
argu4.add_argument('--rebroadcast', dest = 'rebroadcast', metavar = 'y, n', choices = 'yn', help = '제목에 재방송 정보 출력', default = default_rebroadcast)
argu4.add_argument('--episode', dest = 'episode', metavar = 'y, n', choices = 'yn', help = '제목에 회차 정보 출력', default = default_episode)
argu4.add_argument('--verbose', dest = 'verbose', metavar = 'y, n', choices = 'yn', help = 'EPG 정보 추가 출력', default = default_verbose)

args = parser.parse_args()
if args.MyISP : MyISP = args.MyISP
if args.MyChannels : MyChannels = args.MyChannels
if args.display :
    default_output = "d"
elif args.outfile :
    default_output = "o"
    default_xml_file = args.outfile
elif args.socket :
    default_output = "s"
    default_xml_socket = args.socket
if args.icon : default_icon_url = args.icon
if args.limit : default_fetch_limit = args.limit
if args.rebroadcast : default_rebroadcast = args.rebroadcast
if args.episode : default_episode = args.episode
if args.verbose : default_verbose = args.verbose

if MyISP:
    if not any(MyISP in s for s in ['ALL', 'KT', 'LG', 'SK', 'DLIVE']):
        printError("MyISP는 ALL, KT, LG, SK, DLIVE만 가능합니다.")
        sys.exit()
else :
    printError("epg2xml.json 파일의 MyISP항목이 없습니다.")
    sys.exit()

if default_output :
    if any(default_output in s for s in ['d', 'o', 's']):
        if default_output == "d" :
            output = "display";
        elif default_output == "o" :
            output = "file";
        elif default_output == 's' :
            output = "socket";
    else :
        printError("default_output는 d, o, s만 가능합니다.")
        sys.exit()
else :
    printError("epg2xml.json 파일의 output항목이 없습니다.");
    sys.exit()

IconUrl = default_icon_url

if default_rebroadcast :
    if not any(default_rebroadcast in s for s in ['y', 'n']):
        printError("default_rebroadcast는 y, n만 가능합니다.")
        sys.exit()
    else :
        addrebroadcast = default_rebroadcast
else :
    printError("epg2xml.json 파일의 default_rebroadcast항목이 없습니다.");
    sys.exit()

if default_episode :
    if not any(default_episode in s for s in ['y', 'n']):
        printError("default_episode는 y, n만 가능합니다.")
        sys.exit()
    else :
        addepisode = default_episode
else :
    printError("epg2xml.json 파일의 default_episode항목이 없습니다.");
    sys.exit()

if default_verbose :
    if not any(default_verbose in s for s in ['y', 'n']):
        printError("default_verbose는 y, n만 가능합니다.")
        sys.exit()
    else :
        addverbose = default_verbose
else :
    printError("epg2xml.json 파일의 default_verbose항목이 없습니다.");
    sys.exit()

if default_xmltvns :
    if not any(default_xmltvns in s for s in ['y', 'n']):
        printError("default_xmltvns는 y, n만 가능합니다.")
        sys.exit()
    else :
        addxmltvns = default_xmltvns
else :
    printError("epg2xml.json 파일의 default_verbose항목이 없습니다.");
    sys.exit()

if default_fetch_limit :
    if not any(str(default_fetch_limit) in s for s in ['1', '2', '3', '4', '5', '6', '7']):
        printError("default_fetch_limit 는 1, 2, 3, 4, 5, 6, 7만 가능합니다.")
        sys.exit()
    else :
        period = int(default_fetch_limit)
else :
    printError("epg2xml.json 파일의 default_fetch_limit항목이 없습니다.");
    sys.exit()

if output == "file" :
    if default_xml_file :
        sys.stdout = codecs.open(default_xml_file, 'w+', encoding='utf-8')
    else :
        printError("epg2xml.json 파일의 default_xml_file항목이 없습니다.");
        sys.exit()
elif output == "socket" :
    if default_xml_socket :
        try:
            sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            sock.connect(default_xml_socket)
            sockfile = sock.makefile('w+')
            sys.stdout = sockfile
        except socket.error:
            printError(SOCKET_ERROR)
            sys.exit()
    else :
        printError("epg2xml.json 파일의 default_xml_socket항목이 없습니다.");
        sys.exit()
getEpg()
