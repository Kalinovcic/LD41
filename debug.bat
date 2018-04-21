@echo off
setlocal

pushd %~dp0

del run_tree\*.pdb > NUL 2> NUL

lk_build lk_build_debug.txt
lk_build lk_build_debug_platform.txt

popd