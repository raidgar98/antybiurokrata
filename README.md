# antybiurokrata

Engeener project for Silesian University of Technology. </br>
Goal of this project is to compare diffrent aggregators of scientific publications and print report of missing articles between them

## Goals
---
</br>

### Code

- sticks to C++ 20 standard
- is cross-platform
- is macro-free
- is well documented with DOXYGEN

### Program

- connects to multiple sites and download required data
- provides comprasion between theese sites
- prints report of missing publications

</br>

## TODO
---

<!-- - add object representation for: -->
<!-- - [BN](https://data.bn.org.pl/bibs) // no sense: bn does not support science artticles, only books -->
<!-- - [Web Of Science](https://developer.clarivate.com/apis/wos) // => failed: cannot access API -->
	🞎 add more tests
	🞎 design UI
	🞎 refactor orm (exclude abstract classes for jsons, now it's ugly)
	🞎 implement configs
	🞎 implement report generation

## Currently done
---

	🗹 depolishing names and surnames of authors
	🗹 added scripts to generate libraries / windowses
	🗹 properly linked all libraries in cmake with DRY methodology
	🗹 added documentation generation
	🗹 added few patterns for future usage
	🗹 added manual on how to install
	🗹 preimplemented data gathering from `dorobek`
	🗹 recognition of `ORCID` usage
	🗹 implemented assertion and exception mechanism
	🗹 added networking mechanism
	🗹 add conversion between object representation to comparable objects (objects with higher abstraction)

	🗹 added object representation for:
		🗹 [dorobek](https://www.bg.polsl.pl/expertus/new/bib/)
		🗹 [ORCID](https://pub.orcid.org/v3.0/)
		🗹 [Scopus](https://dev.elsevier.com/)


## Setup
---
</br>
This setup was tested for Manjaro Linux (arch-based)
</br>
Most likely it will be working for most of linux distros
</br>

---
### 1. Qt
</br>

The best way of installing all required libraries and headers is to use [this installer](https://www.qt.io/download-qt-installer?hsCtaTracking=99d9dd4f-5681-48d2-b096-470725510d34%7C074ddad0-fdef-4e53-8aa8-5e8a876d6ab4). Unfortunatelly it requires logging in.
</br>
</br>
⚠️Tested version of Qt is 5.4 ⚠️
</br>

---
### 2. Boost
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
### 3. [rang](https://github.com/agauniyal/rang) - collors for terrminal
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
### 4. [Dragon](https://github.com/an-tao/drogon) - network library for modern c++
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
### 5. [boost-ex/ut](https://github.com/boost-ext/ut) -  macro-free testing framework

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
---
