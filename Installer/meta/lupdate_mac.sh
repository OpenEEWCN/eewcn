LUPDATE=~/Qt/5.15.2/clang_64/bin/lupdate

LST=

for f in `ls *.ui *.qs`
do
    LST="$LST $f"
done

$LUPDATE $LST -target-language ja_JP -ts ja.ts
$LUPDATE $LST -target-language zh_CN -ts zh_CN.ts
$LUPDATE $LST -target-language zh_TW -ts zh_TW.ts
