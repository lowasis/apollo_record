#!/bin/bash

channel_json_file="../../external/epg2xml/Channel.json"
item_per_line=2

if [ "$#" -ne 3 ]; then
    echo "Usage : $0 [isp_name={skb,kt,lgu,dlive}] [start_channel_number] [end_channel_number]"
    exit
fi

if [ "$1" == "skb" ]; then
    isp="SK"
elif [ "$1" == "kt" ]; then
    isp="KT"
elif [ "$1" == "lgu" ]; then
    isp="LG"
elif [ "$1" == "dlive" ]; then
    isp="DLIVE"
else
    echo "Unknown ISP : $1"
    exit
fi

output_file_name="epg_channel_table_"$1".h"

echo "#ifndef "`echo $output_file_name | tr '[a-z]' '[A-Z]' | tr '.' '_'` > $output_file_name
echo "#define "`echo $output_file_name | tr '[a-z]' '[A-Z]' | tr '.' '_'` >> $output_file_name
echo "" >> $output_file_name
echo "#ifdef __cplusplus" >> $output_file_name
echo "extern \"C\" {" >> $output_file_name
echo "#endif" >> $output_file_name
echo "" >> $output_file_name
echo "#ifndef CHANNEL_TABLE" >> $output_file_name
echo "#define CHANNEL_TABLE" >> $output_file_name
echo "" >> $output_file_name
echo "typedef struct ChannelTable {" >> $output_file_name
echo "    int channel;" >> $output_file_name
echo "    const char *channel_name;" >> $output_file_name
echo "    int epg2xml_id;" >> $output_file_name
echo "} ChannelTable;" >> $output_file_name
echo "#endif" >> $output_file_name
echo "" >> $output_file_name
echo "static const ChannelTable channel_table_$1[] = {" >> $output_file_name

count=0
for ((i=$2;i<=$3;i++)); do
    str="\""$isp"Ch\": "$i", "
    channel_name=`grep "$str" $channel_json_file | line | sed -e 's/[{}]/''/g' | awk -F ', ' '{print $2}' | awk -F '"' '{print $4}'`
    epg2xml_id=`grep "$str" $channel_json_file | line | sed -e 's/[{}]/''/g' | awk -F ', ' '{print $1}' | awk -F ' ' '{print $2}'`
    if [[ -n $epg2xml_id ]]; then
        if [ $(($count%$item_per_line)) -eq 0 ]; then
            echo -n "    {$i, \"$channel_name\", $epg2xml_id}, " >> $output_file_name
        elif [ $(($count%$item_per_line)) -eq $((item_per_line-1)) ]; then
            echo "{$i, \"$channel_name\", $epg2xml_id}," >> $output_file_name
        else
            echo -n "{$i, \"$channel_name\", $epg2xml_id}, " >> $output_file_name
        fi

        count=$(($count+1))
    fi
done

if [ $(($count%$item_per_line)) -ne 0 ]; then
echo "" >> $output_file_name
fi
echo "    {0, 0}" >> $output_file_name
echo "};" >> $output_file_name
echo "" >> $output_file_name
echo "#ifdef __cplusplus" >> $output_file_name
echo "}" >> $output_file_name
echo "#endif" >> $output_file_name
echo "" >> $output_file_name
echo "#endif" >> $output_file_name
