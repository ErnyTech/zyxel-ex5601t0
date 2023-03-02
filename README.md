# MT7986_v7.6.2.1_MP_SDK
===========================

## source codes was prepared by below steps
reference MT7986_Build_SOP_v1.8_20220114.pdf and MT7986_ReleaseNote_v1.1_20220114.pdf

1. merge openwrt and mtk modification

```
$ git clone --branch openwrt-21.02 https://git.openwrt.org/openwrt/openwrt.git
$ cp -rf mtk-wifi-mt7986/* openwrt/
$ cd openwrt
$ git reset --hard 43d105ec2aa2ee35c9687eab965ac8006e6484ce
$ ./autobuild/mt7986-AX6000/lede-branch-build-sanity.sh
```

2. Remove unused directories and files

```
$ rm -rf autobuild autobuild_release prepare_sdk.sh
```

3. Expand feeds into the same git repository

```
$ find feeds/ -name .git | xargs rm -rf
# Remove /feeds and /package/feeds in .gitignore
```

## Prepare build environment

1. Prepare docker on Ubuntu 16.04

```
$ sudo apt-get update
$ sudo apt-get install ca-certificates curl gnupg lsb-release
$ curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg
$ echo   "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
$ sudo apt-get update
$ sudo apt-get install docker-ce docker-ce-cli containerd.io
$ service docker status
```

2. Prepare docker image for MTK build environment

	1. build docker by Dockerfile, `$(TPDC)\SW1\Vendor\MTK\MT7986\source_codes\STUDY\mtk_docker\Dockerfile`

```
copy to <your Ubuntu> with path "~/mtk_docker/"
$ cd ~/mtk_docker/
$ sudo docker build -t="zyxel/ubuntu-18.04-mtk-env" .
$ docker image ls
```

	2. use pre-build docker image, `$(TPDC)\SW1\Vendor\MTK\MT7986\source_codes\STUDY\mtk_docker\ubuntu-18.04-mtk-env.docker.img`

```
copy to <your Ubuntu> with path "~/mtk_docker/"
$ docker load -i ubuntu-18.04-mtk-env.docker.img
$ docker image ls
```

## How to compile

```
$ docker run --name mtkenv -v ~/$(YOUR/SOURCE/PATH):/home/user/mtk_source:z -it zyxel/ubuntu-18.04-mtk-env
user@d618cf118ac5:~$ cd mtk_source
user@d618cf118ac5:~/mtk_source$ make P=mt7986-AX6000
user@d618cf118ac5:~/mtk_source$ ls bin/targets/mediatek/mt7986/openwrt-mediatek-mt7986-mt7986b-ax6000-2500wan-spim-nand-rfb-squashfs-sysupgrade.bin
```
