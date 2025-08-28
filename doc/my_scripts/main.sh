PROJECT_ROOT=/home/prx/project/luckfox-pico/luckfox-pico/

build_image() {
  source $PROJECT_ROOT/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/env_install_toolchain.sh
  pushd $PROJECT_ROOT
  ./build.sh
  popd
}

burn_image() {

}
