#!/usr/bin/env bash

## Configuration

TEXTURE_FORMAT=gif

## Paths

if [[ -z "${IMG_PATH}" || -z "${LOG_PATH}" || -z "${BINARY_DIR}" || -z "${OBJECT_DIR}" || -z "${TARGET_DIR}" ]]; then
    echo You need to define IMG_PATH, LOG_PATH, BINARY_DIR, OBJECT_DIR and TARGET_DIR before launching this script. >& 2
    echo Abandon. >& 2
    exit 1
fi

BATTLESCENES_DIR="${TARGET_DIR}"/battlescenes
CHARACTERS_DIR="${TARGET_DIR}"/characters
MONSTERS_DIR="${TARGET_DIR}"/monsters

## Binaries

FFIX_EXTRACT_IMG="${BINARY_DIR}"/ffix-extract-img
FFIX_EXTRACT_DB="${BINARY_DIR}"/ffix-extract-db
FFIX_CONVERT_BS="${BINARY_DIR}"/ffix-convert-bs

## Clean logs

:> "${LOG_PATH}"

## Extract files

extract_all_ff9dbs() {
    local recurse=0

    find "$1" -name '*.ff9db' -print0 | sort -z | while read -r -d $'\0' db; do
        recurse=1

        echo " - ${db}"

        local destination="$(dirname "${db}")"/"$(basename "${db}" .ff9db)"
        if ! ${FFIX_EXTRACT_DB} "${db}" "${destination}" >> "${LOG_PATH}"; then
            echo This file will be removed from the source data directory.
        fi

        rm -f "${db}"
    done

    if [[ "${recurse}" -eq 1 ]]; then
        extract_all_ff9dbs "$1"
    fi
}

if [[ ! -e "${OBJECT_DIR}" || "${FFIX_EXTRACT_IMG}" -nt "${OBJECT_DIR}" || "${FFIX_EXTRACT_DB}" -nt "${OBJECT_DIR}" ]]; then
    echo Extracting image file.
    ${FFIX_EXTRACT_IMG} "${IMG_PATH}" "${OBJECT_DIR}" >> "${LOG_PATH}"
    echo Extracting database files.
    extract_all_ff9dbs "${OBJECT_DIR}"
else
    echo Object directory "(${OBJECT_DIR})" seems newer than your binaries. Skipping extraction.
    echo Remove this folder to force new extraction.
fi

## Battle scenes

echo Extracting battle scenes.
find "${OBJECT_DIR}/06" -mindepth 1 -maxdepth 1 -type d -print0 | sort -z | while read -r -d $'\0' pack; do
    echo " - ${pack}"

    tims="$(find "${pack}" -name '*.tim' -print0 | sort -z | xargs -0 -r -n1 printf " --tim %s")"

    destination="${BATTLESCENES_DIR}/$(basename "${pack}")"
    rm -rf "${destination}"

    if ! ${FFIX_CONVERT_BS} --fake-textures-extension ."${TEXTURE_FORMAT}" "${pack}"/000/000.ff9bs "$destination" ${tims} >> "${LOG_PATH}"; then
        echo This file has not been converted.
    else
        find "${destination}" -name '*.tga' -print0 | while read -r -d $'\0' tga; do
            mogrify -format "${TEXTURE_FORMAT}" "${tga}"
            rm "${tga}"
        done

        zip -r "${destination}".zip "${destination}"
    fi
done
