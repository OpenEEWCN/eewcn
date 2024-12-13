# 注意先安装 CQtDeployer:
# https://github.com/QuasarApp/CQtDeployer

# 路径设置
qtdir=~/Qt/5.15.2/gcc_64
deploydir=$PWD-deploy

if [ ! ./eewcn ];then
  echo No build found.
  exit 1
fi
cqtdeployer -bin build/Desktop_Qt_5_15_2_GCC_64bit-Release/eewcn -qmlDir . -qmake ~/Qt/5.15.2/gcc_64/bin/qmake clear -targetDir $deploydir
# sqldriver
cp -r $qtdir/plugins/sqldrivers $deploydir/plugins/
# qm files
cp eewcn_*.qm $deploydir/
cp qt_zh_CN.qm $deploydir/translations/
# OpenSSL files
#在Ubuntu20.04中貌似不需要了
#cp opensslbin/*.so* $deploydir/lib/
# Resource files
cp -r Fonts $deploydir/
cp -r Media $deploydir/
cp -r geodata $deploydir/
#更新说明
cp Installer/data/【更新内容】.txt $deploydir/
cd $deploydir
#zip可以上传蓝奏云但是尺寸太大
#zip -r -9 -y eewcn-ubuntu.zip *
tar -caf eewcn.tar.xz *
#最终文件名注意区分mac和ubuntu版
zip -0 eewcn-ubuntu.tar.xz.zip eewcn.tar.xz
