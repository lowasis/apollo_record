#!/bin/bash

channel_json_file="../../external/epg2xml/Channel.json"
item_per_line=6

if [ "$#" -ne 4 ]; then
    echo "Usage : $0 [isp_name={skb,kt,lgu}] [start_channel_number] [end_channel_number] [output_file_name]"
    exit
fi

if [ "$1" == "skb" ]; then
    isp="SK"
elif [ "$1" == "kt" ]; then
    isp="KT"
elif [ "$1" == "lgu" ]; then
    isp="LG"
else
    echo "Unknown ISP : $1"
    exit
fi

echo "typedef struct ChannelTable {" > $4
echo "    int channel;" >> $4
echo "    int epg2xml_id;" >> $4
echo "} ChannelTable;" >> $4
echo "" >> $4
echo "static const ChannelTable channel_table_$1[] = {" >> $4

count=0
for ((i=$2;i<=$3;i++)); do
    str="\""$isp"Ch\": "$i", "
    res=`grep "$str" $channel_json_file | line | sed -e 's/[{}]/''/g' | awk -F ', ' '{print $1}' | awk -F ' ' '{print $2}'`
    if [[ -n $res ]]; then
        if [ $(($count%$item_per_line)) -eq 0 ]; then
            echo -n "    {$i, $res}, " >> $4
        elif [ $(($count%$item_per_line)) -eq $((item_per_line-1)) ]; then
            echo "{$i, $res}," >> $4
        else
            echo -n "{$i, $res}, " >> $4
        fi

        count=$(($count+1))
    fi
done

if [ $(($count%$item_per_line)) -ne 0 ]; then
echo "" >> $4
fi
echo "    {0, 0}" >> $4
echo "};" >> $4
