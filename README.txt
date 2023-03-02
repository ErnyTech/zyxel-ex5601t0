=========================== ENV ===============================

1.Use ubuntu:18.04 LTS 64-bit

2.Install the required tools:
sudo apt-get install -y uuid-dev
sudo apt-get install -y gcc-aarch64-linux-gnu
sudo apt-get install -y clang-6.0
sudo apt-get install ca-certificates curl gnupg lsb-release
sudo apt-get update
sudo apt-get install python3.6
sudo apt-get install build-essential libpq-dev libssl-dev openssl libffi-dev zlib1g-dev
sudo apt-get install python3-pip python3-dev
sudo apt-get install build-essential gettext bison libfile-slurp-perl gawk libncurses-dev autoconf flex doxygen
sudo apt-get install bin86 bison ccache doxygen flex gcc-multilib genisoimage gettext g++-multilib gperf
sudo apt-get install libx11-dev linux-libc-dev nfs-common nfs-kernel-server pkg-config
sudo apt-get install python-beaker python-dev python-formencode python-lxml python-mako python-rdflib python-setuptools python-simplejson
sudo apt-get install python-pysqlite2 python-sqlobject python-svn python-tk
sudo apt-get install subversion tofrodos tftpd x11proto-core-dev zip zlib1g-dev
sudo apt-get install coreutils curl desktop-file-utils gcc g++ groff libgl1-mesa-dev libglu1-mesa-dev libssl-dev libxml-parser-perl
sudo apt-get install  rpm sed socat texi2html vim
sudo apt-get install gcc g++ binutils patch bzip2 flex make gettext pkg-config unzip zlib1g-dev libc6-dev subversion libncurses5-dev gawk sharutils curl libxml-parser-perl ocaml-nox ocaml-nox ocaml ocaml-findlib  libpcre3-dev binutils-gold python-yaml
sudo apt-get -y install subversion build-essential libncurses5-dev zlib1g-dev gawk git ccache gettext libssl-dev xsltproc sharutils
sudo apt-get install --reinstall subversion build-essential libncurses5-dev zlib1g-dev gawk git ccache gettext libssl-dev xsltproc sharutils
sudo apt-get install uuid uuid-dev
sudo apt-get install uboot-mkimage  device-tree-compiler u-boot-tools
sudo apt-get install pigz
sudo apt-get install pbzip2
sudo apt install net-tools
sudo apt-get install psmisc



=========================== usage ===============================

1. tar zxvf ex5601t0_release_0C0.tar.gz 
2. cd ex5601t0_release/
3. make P=EX5601-T0
4. Firmware : bin/targets/mediatek/ex5601t0/openwrt-mediatek-ex5601t0-ex5601-t0-squashfs-sysupgrade.bin.

