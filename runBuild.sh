mkdir -p build
cd build 
# build and set installation root as build for debug
# cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX="../build"
mkdir -p engine
cd engine

cmake ../../engine/ -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX="/usr" -Wall -Wextra
make
sudo make install

cd ..
mkdir -p sandbox
cd sandbox

cmake ../../sandbox -DCMAKE_BUILD_TYPE=Debug
make 

cd ../../
