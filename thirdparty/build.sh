#!/usr/bin/env bash

set -eo pipefail

cd $(dirname $0)
CWD=$(pwd)
TP_DIR=${CWD}

PARALLEL=$(getconf _NPROCESSORS_ONLN)
PACKAGE=""

function usage() {
    echo "Usage:"
    echo "    $0 [OPTIONS]..."
    echo ""
    echo "Options:"
    echo "    --j [parallel]"
    echo "        Specify the number of parallelism."
    echo "    --package [PACKAGE]"
    echo "        Specify the package, build all packages if empty."
    echo "    --help"
    echo "        Show help message and exit."
    exit 1
}

while test $# -gt 0; do
    case $1 in
    --j)
        PARALLEL=$2
        shift 2
        ;;
    --package)
        PACKAGE=$2
        shift 2
        ;;
    --help)
        usage
        ;;
    *)
        echo Invalid parameters \"$@\".
        usage
        ;;
    esac
done

echo PARALLEL=$PARALLEL
echo PACKAGE=$PACKAGE

mkdir -p "${TP_DIR}/src"
mkdir -p "${TP_DIR}/installed/lib64"
pushd "${TP_DIR}/installed"/
ln -sf lib64 lib
popd

TP_SOURCE_DIR="${TP_DIR}/src"
TP_INSTALL_DIR="${TP_DIR}/installed"
TP_INCLUDE_DIR="${TP_INSTALL_DIR}/include"
TP_LIB_DIR="${TP_INSTALL_DIR}/lib"
TP_PATCH_DIR="${TP_DIR}/patches"

echo "SOURCE_DIR: ${TP_SOURCE_DIR}"
echo "INSTALL_DIR: ${TP_INSTALL_DIR}"
echo "INCLUDE_DIR: ${TP_INCLUDE_DIR}"
echo "LIB_DIR: ${TP_LIB_DIR}"
echo "PATCH_DIR: ${TP_PATCH_DIR}"

function check_md5() {
    local FILE=$1
    local EXPECT=$2

    md5="$(md5 -q "${FILE}")"
    if [[ "${md5}" != "${EXPECT}" ]]; then
        echo "${FILE} md5sum check failed!"
        echo -e "except-md5 ${EXPECT} \nactual-md5 ${md5}"
        exit 1
    else
        echo "${FILE} md5sum check passed!"
    fi
}

function build_gtest() {
    local URL="https://gh.llkk.cc/https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz"
    local FILE=googletest-1.14.0.tar.gz
    local DIR=googletest-1.14.0
    local MD5SUM="c8340a482851ef6a3fe618a082304cfc"

    [ -f ${TP_SOURCE_DIR}/${FILE} ] || wget $URL -O ${TP_SOURCE_DIR}/${FILE}
    check_md5 ${TP_SOURCE_DIR}/${FILE} $MD5SUM
    [ -d ${TP_SOURCE_DIR}/${DIR} ] || tar xf ${TP_SOURCE_DIR}/${FILE} -C ${TP_SOURCE_DIR}

    cd ${TP_SOURCE_DIR}/${DIR}
    cmake -B build . -DCMAKE_INSTALL_PREFIX="${TP_INSTALL_DIR}" \
                    -DBUILD_SHARED_LIBS=OFF \
                    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    make -C build -j ${PARALLEL} install
}

function build_mongo_c_driver() {
    local URL="https://gh.llkk.cc/https://github.com/mongodb/mongo-c-driver/releases/download/1.24.4/mongo-c-driver-1.24.4.tar.gz"
    local FILE=mongo-c-driver-1.24.4.tar.gz
    local DIR=mongo-c-driver-1.24.4
    local MD5SUM="2ac7208c23b58b09f091a345ff3ce718"

    [ -f ${TP_SOURCE_DIR}/${FILE} ] || wget $URL -O ${TP_SOURCE_DIR}/${FILE}
    check_md5 ${TP_SOURCE_DIR}/${FILE} $MD5SUM
    [ -d ${TP_SOURCE_DIR}/${DIR} ] || tar xf ${TP_SOURCE_DIR}/${FILE} -C ${TP_SOURCE_DIR}

    cd ${TP_SOURCE_DIR}/${DIR}
    cmake -B build . -DCMAKE_INSTALL_PREFIX="${TP_INSTALL_DIR}" \
                    -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF \
                    -DENABLE_STATIC=ON \
                    -DENABLE_SHARED=OFF \
                    -DENABLE_TESTS=OFF \
                    -DENABLE_EXAMPLES=OFF
    make -C build -j ${PARALLEL} install
}

function build_mongocxx() {
    local URL="https://gh.llkk.cc/https://github.com/mongodb/mongo-cxx-driver/archive/refs/tags/r3.8.0.tar.gz"
    local FILE=mongo-cxx-driver-r3.8.0.tar.gz
    local DIR=mongo-cxx-driver-r3.8.0
    local MD5SUM="ebe66fcdec9ef2afe46e8630187c8b6a"

    [ -f ${TP_SOURCE_DIR}/${FILE} ] || wget $URL -O ${TP_SOURCE_DIR}/${FILE}
    check_md5 ${TP_SOURCE_DIR}/${FILE} $MD5SUM
    [ -d ${TP_SOURCE_DIR}/${DIR} ] || tar xf ${TP_SOURCE_DIR}/${FILE} -C ${TP_SOURCE_DIR}

    cd ${TP_SOURCE_DIR}/${DIR}
    git init
    git add .
    git commit -m "Initial commit"
    
    cmake -B build . -DCMAKE_INSTALL_PREFIX="${TP_INSTALL_DIR}" \
                    -DBUILD_SHARED_LIBS=OFF \
                    -DCMAKE_PREFIX_PATH="${TP_INSTALL_DIR}" \
                    -DBSONCXX_POLY_USE_BOOST=OFF \
                    -DENABLE_TESTS=OFF \
                    -DENABLE_EXAMPLES=OFF \
                    -DBUILD_VERSION=3.8.0
    make -C build -j ${PARALLEL} install
}

if [[ "${PACKAGE}" == "" ]]; then
    build_gtest
    build_mongo_c_driver
    build_mongocxx
else
    build_${PACKAGE}
fi