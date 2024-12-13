appname=eewcn
mkdir $appname.iconset
# Each file name must be beginning with icon_.
sips -z 16 16 $appname.png --out $appname.iconset/icon_16x16.png
sips -z 32 32 $appname.png --out $appname.iconset/icon_16x16@2x.png
sips -z 32 32 $appname.png --out $appname.iconset/icon_32x32.png
sips -z 64 64 $appname.png --out $appname.iconset/icon_32x32@2x.png
sips -z 128 128 $appname.png --out $appname.iconset/icon_128x128.png
sips -z 256 256 $appname.png --out $appname.iconset/icon_128x128@2x.png
sips -z 256 256 $appname.png --out $appname.iconset/icon_256x256.png
sips -z 512 512 $appname.png --out $appname.iconset/icon_256x256@2x.png
sips -z 512 512 $appname.png --out $appname.iconset/icon_512x512.png
sips -z 1024 1024 $appname.png --out $appname.iconset/icon_512x512@2x.png
iconutil -c icns $appname.iconset -o $appname.icns
