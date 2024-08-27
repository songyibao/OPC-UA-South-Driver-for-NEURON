#
# Copyright 2018 Capitar IT Group BV <info@capitar.com>
# Copyright 2018 Staysail Systems, Inc. <info@staysail.tech>
#
# This software is supplied under the terms of the MIT License, a
# copy of which should be located in the distribution where this
# file was obtained (LICENSE.txt).  A copy of the license may also be
# found online at https://opensource.org/licenses/MIT.

cmake_minimum_required(VERSION 3.13)
set(LIBRARY_OUTPUT_PATH "./")
file(COPY /software/neuron/plugins/opc_ua/OPC-UA.json DESTINATION
        /software/neuron/build/plugins/schema)
project(OPC-UA)


add_library(plugin-OPC-UA SHARED
        opc_ua_plugin.c
        client/ua_client.c
        client/ua_client.h
        ua_tag_hash/ua_tag_hash.c
        ua_tag_handle/ua_tag_handle.c)
target_include_directories(plugin-OPC-UA PRIVATE
        /software/neuron/include/neuron
)
target_link_libraries(plugin-OPC-UA PRIVATE /usr/local/lib/neuron/libneuron-base.so /usr/local/lib/libopen62541.a)
