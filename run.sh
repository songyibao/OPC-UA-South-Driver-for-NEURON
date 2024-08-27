#!/bin/sh
rm -rf ./build
# shellcheck disable=SC2164
mkdir build && cd build && cmake .. && make
echo "build done,coping plugin ..."
cp -r ./libplugin-OPC-UA.so /software/neuron/build/plugins/
cp -r ./libplugin-OPC-UA.so /opt/neuron/plugins/

cp -r ../OPC-UA.json /opt/neuron/plugins/schema/
cp -r ../OPC-UA.json /software/neuron/build/plugins/schema/
echo "copy done"