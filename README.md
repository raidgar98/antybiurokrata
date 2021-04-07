# antybiurokrata

Engeener project for Silesian University of Technology. </br>
Target of this project is to compare diffrent aggregators of scientific publications and print report of missing articles between them

## setup
---
</br>
This setup was tested for Manjaro Linux (arch-based)
</br>
Most likely it will be working for most of linux distros
</br>

---
### Qt
</br>

The best way of installing all required libraries and headers is to use [this installer](https://www.qt.io/download-qt-installer?hsCtaTracking=99d9dd4f-5681-48d2-b096-470725510d34%7C074ddad0-fdef-4e53-8aa8-5e8a876d6ab4). Unfortunatelly it requires logging in.
</br>
</br>
⚠️Tested version of Qt is 5.4 ⚠️
</br>

---
### Boost
</br>

This project uses boost 1.75, which is avaible [here](https://www.boost.org/users/history/version_1_75_0.html)

For build in `/opt/boost` add this line to your `/etc/enviroments`:

```
BOOST_ROOT=/opt/boost
```

also remember to link create symlinks:

```
sudo ln -s $BOOST_ROOT/boost /usr/include/boost
sudo ln -s $BOOST_ROOT/stage/lib /usr/lib/boost
```
---
### [Rang](https://github.com/agauniyal/rang) - collors for terrminal
</br>

```
sudo mkdir /opt/rang
sudo chown $USER:$USER /opt/rang
pushd /opt/rang
git clone https://github.com/agauniyal/rang.git .
push include
sudo ln -s $PWD /usr/include/rang
popd && popd
```
---
### [Dragon](https://github.com/an-tao/drogon) - network library for modern c++
</br>

```
sudo mkdir /opt/drogon
sudo chown $USER:$USER /opt/drogon
pushd /opt/drogon
git clone https://github.com/an-tao/drogon.git .
git submodule update --init --recursive
mkdir build
pushd build
cmake ..
make -j$(nproc)
sudo make install
popd && popd
```
---
### [ut](https://github.com/boost-ext/ut) -  macro-less testing framework

```
sudo mkdir /opt/ut
sudo chown $USER:$USER /opt/ut
pushd /opt/ut
git clone https://github.com/boost-ext/ut.git .
git submodule update --init --recursive
mkdir build
pushd build
cmake ..
make -j$(nproc)
sudo make install
popd && popd
```

In my case i've had to create symlink:

```
ln -s /usr/local/include/ut-*/include/boost/ut.hpp /usr/include/boost/ut.hpp
```	
