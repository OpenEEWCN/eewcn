# Path settings
deploydir=$PWD/android/assets

if [ ! ./eewcn ];then
  echo No build found.
  exit 1
fi
mkdir $deploydir
# qm files
cp eewcn_*.qm $deploydir/
#cp qt_zh_CN.qm $deploydir/translations/
# Resource files
cp -r Fonts $deploydir/
cp -r Media $deploydir/
cp -r geodata $deploydir/
