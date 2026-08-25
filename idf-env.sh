#!/bin/zsh
# Activates the ESP-IDF v5.4.1 build environment for this project.
# Meant to be sourced, not executed: `source idf-env.sh`
#
# Reuses ESP-IDF's own bundled Python 3.11 venv to satisfy export.sh's
# python version check, instead of relying on the system python3 (which
# may be a newer, incompatible version) or fiddling with PATH ordering.

if [ -z "$IDF_PATH" ]; then
    source ~/.espressif/python_env/idf5.4_py3.11_env/bin/activate
    source ~/esp/v5.4.1/esp-idf/export.sh
fi
