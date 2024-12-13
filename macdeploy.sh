# Path settings
qtdir=~/Qt/5.15.2/clang_64
deploydir=$PWD-deploy

if [ ! ./eewcn ];then
  echo No build found.
  exit 1
fi
mkdir $deploydir
#目标文件夹不用写eewcn.app
cp -r build/Desktop_Qt_5_15_2_clang_64bit-Release/eewcn.app $deploydir
$qtdir/bin/macdeployqt $deploydir/eewcn.app -qmldir=.
# qsvg plugins
cp $qtdir/plugins/imageformats/libqsvg.dylib $deploydir/eewcn.app/Contents/PlugIns/imageformats
# sqldriver
cp -r $qtdir/plugins/sqldrivers $deploydir/eewcn.app/Contents/PlugIns/
# qm files
cp eewcn_*.qm $deploydir/
#cp qt_zh_CN.qm $deploydir/translations/
# OpenSSL files
#cp opensslbin/*.so* $deploydir/lib/
# Resource files
cp -r Fonts $deploydir/
cp -r Media $deploydir/
cp -r geodata $deploydir/
#更新说明
cp Installer/data/【更新内容】.txt $deploydir/
cd $deploydir
#zip可以上传蓝奏云但是尺寸太大
#zip -r -9 -y eewcn-mac.zip *
tar -caf eewcn.tar.xz *
#最终文件名注意区分mac和ubuntu版
zip -0 eewcn-mac.tar.xz.zip eewcn.tar.xz
