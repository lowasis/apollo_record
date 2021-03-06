#ifndef EPG_CHANNEL_TABLE_DLIVE_H
#define EPG_CHANNEL_TABLE_DLIVE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CHANNEL_TABLE
#define CHANNEL_TABLE

typedef struct ChannelTable {
    int channel;
    const char *channel_name;
    int epg2xml_id;
} ChannelTable;
#endif

static const ChannelTable channel_table_dlive[] = {
    {0, "MBC Every1", 123}, {1, "지역채널", 499},
    {2, "OBS", 149}, {3, "KBS DRAMA", 103},
    {4, "NS홈쇼핑", 144}, {5, "SBS", 164},
    {6, "현대홈쇼핑", 312}, {7, "KBS2", 111},
    {8, "롯데홈쇼핑", 238}, {9, "KBS1", 110},
    {10, "GS SHOP", 85}, {11, "MBC", 122},
    {12, "CJ오쇼핑", 40}, {13, "EBS", 60},
    {14, "홈&쇼핑", 314}, {15, "JTBC", 97},
    {16, "tvN", 207}, {17, "채널A", 281},
    {18, "MBN", 129}, {19, "TV조선", 208},
    {20, "공영쇼핑", 265}, {21, "드라맥스", 232},
    {22, "코미디TV", 293}, {23, "연합뉴스TV", 268},
    {24, "YTN", 218}, {25, "신세계쇼핑", 260},
    {26, "SBS Plus", 170}, {27, "W 쇼핑", 213},
    {28, "K STAR", 101}, {29, "SK stoa", 11},
    {30, "KBS JOY", 104}, {31, "K쇼핑", 119},
    {32, "OCN", 151}, {33, "롯데원티비", 237},
    {34, "Mnet", 121}, {35, "쇼핑엔T", 258},
    {36, "SBS funE", 166}, {37, "현대홈쇼핑+샵", 313},
    {38, "E채널", 55}, {39, "GS MY SHOP", 84},
    {40, "O tvN", 147}, {41, "CJ오쇼핑 플러스", 41},
    {42, "MBC Dramanet", 128}, {43, "NS Shop+", 143},
    {44, "AXN", 10}, {45, "스크린", 171},
    {46, "케이블TV VOD", 498}, {47, "TVCHOSUN2", 26},
    {48, "JTBC2", 99}, {49, "Lifetime", 456},
    {50, "CNTV", 48}, {51, "드라마H", 230},
    {52, "드라마큐브", 231}, {53, "Smile TV", 182},
    {54, "올리브", 148}, {55, "Edge TV", 62},
    {56, "KBS W", 108}, {57, "디원", 233},
    {58, "SkyDrama", 173}, {59, "하이라이트TV", 89},
    {60, "GTV", 86}, {61, "History", 90},
    {62, "TVasia", 500}, {63, "채널 Ching", 280},
    {64, "채널차이나", 286}, {65, "중화TV", 278},
    {66, "채널 J", 35}, {67, "FOX채널", 74},
    {68, "텔레노벨라", 199}, {69, "EXF Plus", 501},
    {70, "채널A 플러스", 282}, {71, "MBN Plus", 130},
    {72, "HQ+", 91}, {73, "MBC MUSIC", 124},
    {74, "CUBE TV", 50}, {75, "SBS MTV", 168},
    {76, "채널CGV", 283}, {77, "캐치온1", 290},
    {78, "슈퍼액션", 196}, {79, "cineF", 38},
    {80, "DIA TV", 409}, {81, "XtvN", 216},
    {82, "FUN TV", 79}, {83, "SkyENT", 174},
    {84, "JTBC4", 495}, {85, "CMC TV", 43},
    {86, "FX", 80}, {87, "리얼TV", 160},
    {88, "아이넷", 92}, {89, "온스타일", 154},
    {90, "TRENDY", 203}, {91, "Fashion N", 71},
    {92, "동아TV", 229}, {93, "Fox life", 75},
    {94, "Cook TV", 502}, {95, "사이언스TV", 219},
    {96, "mplex", 134}, {97, "인디필름", 275},
    {98, "THE MOVIE", 201}, {99, "캐치온2", 291},
    {100, "디자이어TV", 494}, {101, "viki", 212},
    {102, "Playboy TV", 159}, {103, "미드나잇", 244},
    {104, "허니TV", 309}, {105, "핑크하우스", 299},
    {109, "SPOTV ON1", 414}, {110, "SPOTV ON2", 415},
    {111, "SBS GOLF", 167}, {112, "JTBC Golf", 98},
    {113, "JTBC3 FOXSPORTS", 100}, {114, "SkySports", 178},
    {115, "MBC SPORTS+2", 127}, {116, "MBC SPORTS+", 126},
    {117, "SBS Sports", 169}, {118, "KBS N Sports", 107},
    {119, "SPOTV", 183}, {120, "SPOTV2", 186},
    {121, "SPOTV+", 185}, {122, "IB SPORTS", 93},
    {123, "Koreasports", 503}, {124, "생활체육TV", 254},
    {125, "SkyPetPark", 177}, {126, "채널해피독", 287},
    {127, "ONT", 156}, {128, "마운틴TV", 133},
    {129, "FTV", 78}, {130, "FISHING TV", 72},
    {131, "OGN", 153}, {132, "SPOTV Games", 184},
    {133, "바둑TV", 245}, {134, "브레인TV", 251},
    {135, "K-바둑", 118}, {136, "SkyTravel", 179},
    {137, "채널뷰", 285}, {138, "YTN life", 220},
    {139, "헬스메디tv", 311}, {140, "아르떼TV", 6},
    {141, "Life U", 316}, {142, "GMTV", 82},
    {143, "이벤트TV", 273}, {144, "실버아이TV", 262},
    {145, "EtN연예TV", 485}, {146, "폴라리스TV", 302},
    {147, "AsiaN", 8}, {148, "채널W", 444},
    {149, "다큐원", 504}, {150, "NatGeo Wild", 137},
    {151, "CNN US", 47}, {152, "CLASSICA", 42},
    {153, "시니어TV", 447}, {154, "빌리어즈티비", 20},
    {155, "Star Sports", 192}, {156, "STN스포츠", 194},
    {157, "Golf Channel Korea", 200}, {158, "브릿지TV", 292},
    {159, "WBS원음방송", 439}, {160, "OBS W", 150},
    {161, "CMTV", 44}, {162, "Mezzo", 505},
    {163, "UMAX", 320}, {164, "LifeU UHD", 506},
    {165, "UXN", 39}, {166, "SBS Plus UHD", 507},
    {170, "NGCK", 138}, {171, "Discovery", 51},
    {172, "BBC Earth", 15}, {173, "한국농업방송", 517},
    {174, "SkyICT", 176}, {175, "채널i", 508},
    {176, "MBC NET", 125}, {177, "아프리카TV", 509},
    {180, "한국경제TV", 303}, {181, "머니투데이방송", 135},
    {182, "SBS CNBC", 165}, {183, "토마토TV", 297},
    {184, "매일경제TV", 241}, {185, "이데일리TV", 272},
    {186, "서울경제TV", 255}, {187, "아시아경제TV", 264},
    {188, "R토마토", 249}, {189, "아리랑 TV", 263},
    {190, "CNN", 46}, {191, "BBC World News", 18},
    {192, "CNBC", 45}, {193, "Bloomberg", 21},
    {194, "NHK-W", 321}, {195, "CGTN", 411},
    {196, "CCTV4", 31}, {199, "english gem", 510},
    {200, "챔프", 511}, {201, "Tooniverse", 202},
    {202, "ANIMAX", 4}, {203, "니켈로디언", 141},
    {204, "디즈니", 235}, {205, "카툰네트워크", 289},
    {206, "JEI 재능TV", 96}, {207, "KBS kids", 105},
    {208, "대교어린이TV", 228}, {209, "신기한나라TV", 493},
    {210, "애니박스", 2}, {211, "부메랑", 250},
    {212, "애니플러스", 266}, {213, "키즈톡톡 플러스", 112},
    {214, "JEI EnglishTV", 95}, {215, "플레이런TV", 457},
    {216, "디즈니주니어", 234}, {217, "대교베이비TV", 14},
    {218, "케이블TV VOD 키즈", 513}, {219, "EBS kids", 57},
    {220, "EBS PLUS1", 58}, {221, "EBS PLUS2", 59},
    {222, "EBS English", 56}, {224, "Wee TV", 512},
    {230, "한국청소년방송", 514}, {231, "육아방송", 271},
    {232, "채널큐피드", 515}, {233, "쿠키건강TV", 294},
    {234, "tbsTV", 198}, {235, "리빙TV", 239},
    {250, "KTV", 117}, {251, "OUN", 157},
    {252, "국회방송", 223}, {253, "시민방송", 516},
    {254, "소비자TV", 256}, {255, "복지TV", 247},
    {256, "한국직업방송", 307}, {257, "소상공인방송", 257},
    {258, "국방TV", 222}, {259, "SAFE TV", 253},
    {260, "법률방송", 246}, {300, "상생방송", 193},
    {301, "CTS기독교TV", 49}, {302, "CBS TV", 28},
    {303, "Good TV", 83}, {304, "CGNTV", 32},
    {305, "C channel", 24}, {306, "BTN불교TV", 23},
    {307, "BBS 불교방송", 19}, {308, "가톨릭평화방송", 301},
    {0, 0}
};

#ifdef __cplusplus
}
#endif

#endif
