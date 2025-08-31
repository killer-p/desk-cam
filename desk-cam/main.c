#include "rk_aiq_comm.h"
#include "rk_comm_vi.h"
#include "rk_comm_video.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_vi.h"
#include "rk_type.h"
#include "stdio.h"
#include <rk_aiq_user_api2_camgroup.h>
#include <rk_aiq_user_api2_imgproc.h>
#include <rk_aiq_user_api2_sysctl.h>
#include <stdbool.h>

int vi_dev_id = 0;
int vi_pipe_id = 0;
int vi_chn_id = 0;
const char *iq_file_path = "/etc/iqfiles/";
rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;

rk_aiq_sys_ctx_t *aiq_ctx;
int init_aiq(void) {

  rk_aiq_static_info_t aiq_static_info;
  rk_aiq_uapi2_sysctl_enumStaticMetasByPhyId(0, &aiq_static_info);

  printf("ID: 0, sensor_name is %s, iqfiles is %s\n",
         aiq_static_info.sensor_info.sensor_name, iq_file_path);

  aiq_ctx = rk_aiq_uapi2_sysctl_init(aiq_static_info.sensor_info.sensor_name,
                                     iq_file_path, NULL, NULL);
  return 0;
}

int deinit_aiq(void) {
  rk_aiq_uapi2_sysctl_deinit(aiq_ctx);
  return 0;
}

int run_aiq(void) {
  rk_aiq_uapi2_sysctl_prepare(aiq_ctx, 0, 0, hdr_mode);
  rk_aiq_uapi2_sysctl_start(aiq_ctx);
  return 0;
}

int stop_aiq(void) {
  rk_aiq_uapi2_sysctl_stop(aiq_ctx, false);
  return 0;
}

int init_vi(void) {
  VI_DEV_ATTR_S dev_attr = {0};
  int ret;
  VI_DEV_BIND_PIPE_S bind;
  VI_CHN_ATTR_S chn_attr = {0};

  // the dev_attr has no effect,but we still need to call this funcion
  RK_MPI_VI_SetDevAttr(vi_dev_id, &dev_attr);

  ret = RK_MPI_VI_GetDevIsEnable(vi_dev_id);
  if (ret != RK_SUCCESS) {
    // start vi
    RK_MPI_VI_EnableDev(vi_dev_id);
    bind.u32Num = 1;
    // pipe id is the same as vi_dev_id
    bind.PipeId[0] = vi_pipe_id;
    RK_MPI_VI_SetDevBindPipe(vi_dev_id, &bind);
  }

  chn_attr.stSize.u32Width = 1920;
  chn_attr.stSize.u32Height = 1080;
  chn_attr.stIspOpt.u32BufCount = 2;
  chn_attr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;
  chn_attr.u32Depth = 1;
  chn_attr.enPixelFormat = RK_FMT_YUV420SP;
  chn_attr.enCompressMode = COMPRESS_MODE_NONE;
  chn_attr.stFrameRate.s32SrcFrameRate = -1;
  chn_attr.stFrameRate.s32DstFrameRate = -1;

  RK_MPI_VI_SetChnAttr(vi_pipe_id, vi_chn_id, &chn_attr);

  RK_MPI_VI_EnableChn(vi_pipe_id, vi_chn_id);

  return 0;
}

int deinit_vi(void) {
  RK_MPI_VI_DisableChn(vi_pipe_id, vi_chn_id);

  RK_MPI_VI_DisableDev(vi_dev_id);

  return 0;
}

int main() {
  printf("hello world\n");

  RK_MPI_SYS_Init();
  init_aiq();
  run_aiq();
  init_vi();
  stop_aiq();
  deinit_aiq();
  deinit_vi();
  RK_MPI_SYS_Exit();
  return 0;
}
