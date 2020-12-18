#!/bin/bash

make project_clen

make all -j8
make flash
