# convert-map-to-png
Convert .map, .pak and .sav to .png

Get project files on your server :
```
git clone https://github.com/Thaldos/convert-map-to-png.git
```

Install libpng-12 :
```
sudo apt update
sudo apt install wget
wget -q -O /tmp/libpng12.deb http://mirrors.kernel.org/ubuntu/pool/main/libp/libpng/libpng12-0_1.2.54-1ubuntu1_amd64.deb \
&& sudo dpkg -i /tmp/libpng12.deb \
&& rm /tmp/libpng12.deb
```

Usage :
Put .map, or .sav files in input folder.
```
devilbox@php-7.4.20 in /shared/httpd $ cd convert-map-to-png
devilbox@php-7.4.20 in /shared/httpd/convert-map-to-png $ php index.php
```
The png should be in output folder.


