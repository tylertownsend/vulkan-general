mkdir -p build
cd build 
cmake -DCMAKE_BUILD_TYPE=Release .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON #-DCMAKE_INSTALL_PREFIX=.. 
 
make
make install
cd ..