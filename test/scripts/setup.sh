#!/bin/bash

sudo refmon --flush
sudo refmon --off
sudo rm -r testenv/*
rmdir testenv

mkdir testenv
echo "test">testenv/file.txt
ln testenv/file.txt testenv/hard_link
ln -s "$PWD"/testenv/file.txt testenv/symlink

ln testenv/hard_link testenv/hl_hl
ln -s "$PWD"/testenv/hard_link testenv/hl_sl
ln testenv/symlink testenv/sl_hl
ln -s "$PWD"/testenv/symlink testenv/sl_sl

mkdir  testenv/subfolder
echo "other_test">testenv/subfolder/other_file.txt
mkdir  testenv/empty-subdir

