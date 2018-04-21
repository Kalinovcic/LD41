@echo off
setlocal

pushd %~dp0

del run_tree\*.pdb > NUL 2> NUL

lk_build lk_build_release.txt
lk_build lk_build_release_platform.txt

popd