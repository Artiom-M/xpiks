#!/bin/bash
# simple script for unix base system for vendors preparation
BUILD_MODE=${1}

case ${BUILD_MODE} in
    release)
        TARGET="release"
        ;;
    debug|travis-ci)
        TARGET="debug"
        ;;
    fullrelease)
        TARGET="release"
        ;;
    fulldebug)
        TARGET="debug"
        ;;
    *)
        echo "$(basename ${0}) error: Invalid command arguments"
        echo "    usage: $(basename ${0}) <debug|release|travis-ci> [optional make flags]"
        exit 1
        ;;
esac

shift
MAKE_FLAGS="${*}" # do we need argument validation?

### output prefix
GREEN_COLOR='\033[0;32m'
RESET_COLOR='\033[0m'
PRINT_PREFIX="${GREEN_COLOR}[VENDORS]${RESET_COLOR}"

echo -e "${PRINT_PREFIX} Start vendors preparation for ${BUILD_MODE} with make flags: ${MAKE_FLAGS}"

### find root dir
ROOT_DIR=$(git rev-parse --show-toplevel)
cd ${ROOT_DIR}

### translations
echo -e "${PRINT_PREFIX} Generating translations..."
cd ${ROOT_DIR}/src/xpiks-qt/deps/translations/
make ${MAKE_FLAGS}
echo -e "${PRINT_PREFIX} Generating translations... - done."

if [ ${BUILD_MODE} == "fulldebug" ] || [ ${BUILD_MODE} == "fullrelease" ]
then
    echo "Full mode"
    ### libthmbnlr
    echo -e "${PRINT_PREFIX} Building libthmbnlr..."
    cd ${ROOT_DIR}/../libthmbnlr/src/libthmbnlr
    qmake "CONFIG+=${TARGET}" libthmbnlr.pro
    make ${MAKE_FLAGS}	
	cp libthmbnlr.a ${ROOT_DIR}/libs/${TARGET}
    echo -e "${PRINT_PREFIX} Building libthmbnlr... - done."
    
    ### libxpks
    echo -e "${PRINT_PREFIX} Building libxpks..."
    cd ${ROOT_DIR}/../libxpks/src/xpks
    qmake "CONFIG+=${TARGET}" xpks.pro
    make ${MAKE_FLAGS}
    cp libxpks.a ${ROOT_DIR}/libs/${TARGET}
    echo -e "${PRINT_PREFIX} Building libxpks... - done."
else
    echo "Normal mode"
    ### libthmbnlr
    echo -e "${PRINT_PREFIX} Building libthmbnlr..."
    cd ${ROOT_DIR}/vendors/libthmbnlr
    qmake "CONFIG+=${TARGET}" thmbnlr.pro
    make ${MAKE_FLAGS}
    echo -e "${PRINT_PREFIX} Building libthmbnlr... - done."
		
    ### libxpks
    echo -e "${PRINT_PREFIX} Building libxpks..."
    cd ${ROOT_DIR}/src/libxpks_stub
    qmake "CONFIG+=${TARGET}" libxpks_stub.pro
    make ${MAKE_FLAGS}
    cp libxpks.a ${ROOT_DIR}/libs/${TARGET}
    echo -e "${PRINT_PREFIX} Building libxpks... - done."
fi

### tiny-aes
echo -e "${PRINT_PREFIX} Renaming tiny-aes..."
cd ${ROOT_DIR}/vendors/tiny-aes
mv aes.c aes.cpp
echo -e "${PRINT_PREFIX} Renaming tiny-aes... - done."

### cpp-libface
echo -e "${PRINT_PREFIX} Building cpp-libface..."
cd ${ROOT_DIR}/vendors/cpp-libface/libface-project
qmake "CONFIG+=${TARGET}" libface.pro
make ${MAKE_FLAGS}
cp libface.so.1.0.0 ${ROOT_DIR}/libs/${TARGET}
cd ${ROOT_DIR}/libs/${TARGET}
ln -s libface.so.1.0.0 libface.so
ln -s libface.so.1.0.0 libface.so.1
ln -s libface.so.1.0.0 libface.so.1.0
echo -e "${PRINT_PREFIX} Building cpp-libface... - done."

### chillout
echo -e "${PRINT_PREFIX} Building chillout..."
cd ${ROOT_DIR}/vendors/chillout/src/chillout
qmake "CONFIG+=${TARGET}" libchillout.pro
make ${MAKE_FLAGS}
cp libchillout.a ${ROOT_DIR}/libs/${TARGET}
echo -e "${PRINT_PREFIX} Building chillout... - done."

### ssdll
echo -e "${PRINT_PREFIX} Building ssdll..."
cd ${ROOT_DIR}/vendors/ssdll/src/ssdll/
qmake "CONFIG+=${TARGET}" ssdll.pro
make ${MAKE_FLAGS}
cp libssdll.so.1.0.0 ${ROOT_DIR}/libs/${TARGET}
cd ${ROOT_DIR}/libs/${TARGET}
ln -s libssdll.so.1.0.0 libssdll.so
ln -s libssdll.so.1.0.0 libssdll.so.1
ln -s libssdll.so.1.0.0 libssdll.so.1.0
echo -e "${PRINT_PREFIX} Building ssdll... - done."

### quazip
echo -e "${PRINT_PREFIX} Building quazip..."
cd ${ROOT_DIR}/vendors/quazip/quazip/
qmake "CONFIG+=${TARGET}" quazip.pro
make ${MAKE_FLAGS}
cp libquazip.so.1.0.0 ${ROOT_DIR}/libs/${TARGET}
cd ${ROOT_DIR}/libs/${TARGET}
ln -s libquazip.so.1.0.0 libquazip.so
ln -s libquazip.so.1.0.0 libquazip.so.1
ln -s libquazip.so.1.0.0 libquazip.so.1.0
echo -e "${PRINT_PREFIX} Building quazip... - done."

### hunspell
echo -e "${PRINT_PREFIX} Building hunspell..."
cd ${ROOT_DIR}/vendors/hunspell/
qmake "CONFIG+=${TARGET}" hunspell.pro
make ${MAKE_FLAGS}
echo -e "${PRINT_PREFIX} Building hunspell... - done."

### recoverty
echo -e "${PRINT_PREFIX} Building recoverty..."
cd ${ROOT_DIR}/src/recoverty/
qmake "CONFIG+=${TARGET}" recoverty.pro
make ${MAKE_FLAGS}
RECOVERTY_DIR=${ROOT_DIR}/src/xpiks-qt/deps/recoverty
mkdir -p ${RECOVERTY_DIR}
cp Recoverty ${RECOVERTY_DIR}/
echo -e "${PRINT_PREFIX} Building recoverty... - done."

echo -e "${PRINT_PREFIX} Vendors preparation for ${BUILD_MODE}: done."
