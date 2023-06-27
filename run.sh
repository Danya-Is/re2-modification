#!/bin/bash
./diploma -match $1
python3 matcher.py $1
