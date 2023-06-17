#!/bin/bash
./diploma -match $1
python3 python_re/matcher.py $1
