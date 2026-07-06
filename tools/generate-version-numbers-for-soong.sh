#!/bin/bash

source $1

version_and_extra=( ${VERSION//-/ } )
version_only=${version_and_extra[0]}

semver=( ${version_only//./ } )
major=${semver[0]}
minor=${semver[1]}
patch=${semver[2]}

sed "s/@CONFIG_BARQ_NATIVE_VERSION_MAJOR@/$major/g; s/@CONFIG_BARQ_NATIVE_VERSION_MINOR@/$minor/g; s/@CONFIG_BARQ_NATIVE_VERSION_PATCH@/$patch/g; s/@CONFIG_BARQ_NATIVE_VERSION@/$VERSION/g" $2