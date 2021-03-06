#ifndef EPG_CHANNEL_TABLE_LGU_H
#define EPG_CHANNEL_TABLE_LGU_H

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

static const ChannelTable channel_table_lgu[] = {
    {2, "UXN", 39}, {3, "KBS JOY", 104},
    {4, "홈&쇼핑", 314}, {5, "SBS", 164},
    {6, "GS SHOP", 85}, {7, "KBS2", 111},
    {8, "CJ오쇼핑", 40}, {9, "KBS1", 110},
    {10, "현대홈쇼핑", 312}, {11, "MBC", 122},
    {12, "롯데홈쇼핑", 238}, {13, "NS홈쇼핑", 144},
    {14, "EBS", 60}, {15, "JTBC", 97},
    {16, "MBN", 129}, {17, "tvN", 207},
    {18, "채널A", 281}, {19, "TV조선", 208},
    {20, "아임쇼핑", 265}, {21, "롯데원티비", 237},
    {22, "Mnet", 121}, {23, "연합뉴스TV", 268},
    {24, "YTN", 218}, {25, "YTN 사이언스", 219},
    {26, "OBS", 149}, {27, "SBS CNBC", 165},
    {28, "SK stoa", 11}, {29, "MBC Every1", 123},
    {30, "GS MY SHOP", 84}, {31, "KBS DRAMA", 103},
    {32, "CJ오쇼핑 플러스", 41}, {33, "SBS Plus", 170},
    {34, "현대홈쇼핑+샵", 313}, {35, "MBC Dramanet", 128},
    {38, "OCN", 151}, {39, "채널CGV", 283},
    {40, "슈퍼액션", 196}, {41, "스크린", 171},
    {42, "cineF", 38}, {44, "FOX", 74},
    {45, "AXN", 10}, {46, "mplex", 134},
    {47, "THE MOVIE", 201}, {48, "캐치온1", 290},
    {49, "캐치온2", 291}, {50, "JTBC3", 100},
    {51, "SPOTV+", 185}, {52, "SPOTV2", 186},
    {53, "SBS GOLF", 167}, {54, "JTBC Golf", 98},
    {55, "The Golf Channel", 200}, {56, "SPOTV", 183},
    {57, "SkySports", 178}, {58, "SBS Sports", 169},
    {59, "KBS N Sports", 107}, {60, "MBC SPORTS+", 126},
    {61, "MBC SPORTS+2", 127}, {62, "IB SPORTS", 93},
    {63, "빌리어즈티비", 20}, {64, "FTV", 78},
    {65, "FISHING TV", 72}, {66, "SkyTravel", 179},
    {67, "폴라리스TV", 302}, {68, "Edge TV", 62},
    {69, "마운틴TV", 133}, {71, "O tvN", 147},
    {72, "XtvN", 216}, {73, "온스타일", 154},
    {74, "신세계쇼핑", 260}, {75, "SBS funE", 166},
    {76, "쇼핑엔T", 258}, {77, "KBS W", 108},
    {79, "SkyDrama", 173}, {80, "채널차이나", 286},
    {81, "텔레노벨라", 199}, {82, "올리브", 148},
    {83, "Lifetime", 456}, {84, "동아TV", 229},
    {85, "CNTV", 48}, {86, "C TIME", 26},
    {87, "중화TV", 278}, {88, "AsiaN", 8},
    {89, "Dog TV", 52}, {90, "디원", 233},
    {91, "하이라이트TV", 89}, {92, "JTBC4", 495},
    {93, "DIA TV", 409}, {94, "JTBC2", 99},
    {95, "EBS2", 61}, {96, "OGN", 153},
    {97, "바둑TV", 245}, {98, "브레인TV", 251},
    {99, "MBC MUSIC", 124}, {100, "SBS MTV", 168},
    {101, "GMTV", 82}, {102, "가요TV", 221},
    {103, "이벤트TV", 273}, {104, "E채널", 55},
    {105, "K STAR", 101}, {106, "아이넷TV", 92},
    {107, "K-바둑", 118}, {108, "코미디TV", 293},
    {109, "SPOTV Games", 184}, {110, "히어로액션", 87},
    {111, "토마토TV", 297}, {112, "매일경제TV", 241},
    {113, "아시아경제TV", 264}, {115, "채널A 플러스", 282},
    {116, "MBN Plus", 130}, {117, "CNN International", 46},
    {118, "CNBC", 45}, {119, "CGTN", 411},
    {120, "CCTV4", 31}, {121, "한국경제TV", 303},
    {122, "머니투데이방송", 135}, {123, "이데일리TV", 272},
    {124, "서울경제TV", 255}, {125, "YTN life", 220},
    {126, "BBC WN", 18}, {130, "BBC Earth", 15},
    {131, "NGC", 138}, {132, "History", 90},
    {133, "디스커버리 아시아", 317}, {134, "NatGeo Wild", 137},
    {135, "Outdoor", 158}, {136, "GTV", 86},
    {137, "OBS W", 150}, {138, "헬스메디tv", 311},
    {139, "예술 TV아르떼", 6}, {140, "MBC NET", 125},
    {141, "아리랑 TV", 263}, {142, "NHK World TV", 321},
    {143, "NHK WP", 139}, {144, "쿠키건강TV", 294},
    {145, "채널 J", 35}, {146, "CLASSICA", 42},
    {148, "애니박스", 2}, {149, "애니플러스", 266},
    {150, "Disney Channel", 235}, {151, "디즈니주니어", 234},
    {152, "Tooniverse", 202}, {153, "애니원", 5},
    {154, "니켈로디언", 141}, {155, "카툰네트워크", 289},
    {156, "대교 어린이TV", 228}, {157, "키즈원", 296},
    {158, "키즈톡톡 플러스", 112}, {159, "JEI 재능TV", 96},
    {160, "JEI EnglishTV", 95}, {161, "플레이런TV", 457},
    {162, "EBS English", 56}, {163, "EBS PLUS1", 58},
    {164, "EBS PLUS2", 59}, {165, "edu TV", 63},
    {166, "부메랑", 250}, {167, "ANIMAX", 4},
    {168, "EBS kids", 57}, {169, "KBS kids", 105},
    {170, "OUN", 157}, {171, "KTV", 117},
    {172, "국회방송", 223}, {173, "복지TV", 247},
    {174, "국방TV", 222}, {175, "소상공인방송", 257},
    {176, "tbsTV", 198}, {177, "소비자TV", 256},
    {178, "9colors", 1}, {180, "CTS기독교TV", 49},
    {181, "CBS", 28}, {182, "C channel", 24},
    {183, "CGNTV", 32}, {184, "가톨릭평화방송", 301},
    {185, "BTN불교TV", 23}, {186, "BBS 불교방송", 19},
    {187, "STB상생방송", 193}, {188, "WBS원음방송", 215},
    {290, "Playboy TV", 159}, {291, "미드나잇", 244},
    {292, "viki", 212}, {293, "허니TV", 309},
    {294, "디자이어TV", 494}, {295, "핑크하우스", 299},
    {0, 0}
};

#ifdef __cplusplus
}
#endif

#endif
