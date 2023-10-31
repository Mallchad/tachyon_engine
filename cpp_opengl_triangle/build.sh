#!/usr/bin/env bash

# shellcheck "$0"
self="$0"
self_dir="$( readlink --canonicalize "${self%/*}" )"

# export NELUA_PATH="${NELUA_PATH};${self_dir}/lib/?.lua"
# export LUA_PATH="${LUA_PATH};${self_dir}/lib/?.lua"

# if which "nelua" ;then
if which nelua > /dev/null ; then
    echo "Found nelua, using nelua for build script";
    echo "-----------------------------------------"
    echo
    nelua -w "./build.nelua" "$@" ;
elif which "lua" ;then
    echo "Falling back to lua for build script";
    echo "-----------------------------------------"
    echo
    lua ./build.lua "$@" ;
fi
