#!/bin/bash

sudo refmon --recoff
sudo refmon --flush
sudo refmon --off

unlink testenv/hard_link
unlink testenv/symlink
unlink testenv/hl_hl
unlink testenv/hl_sl
unlink testenv/sl_hl
unlink testenv/sl_sl
rm -rf testenv/subfolder/*
rm -rf testenv/*
rmdir testenv/empty-subdir
rmdir testenv