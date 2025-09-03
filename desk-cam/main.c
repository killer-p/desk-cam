#include "rk_aiq_comm.h"
#include "rk_comm_rc.h"
#include "rk_comm_venc.h"
#include "rk_comm_vi.h"
#include "rk_comm_video.h"
#include "rk_comm_vpss.h"
#include "rk_common.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_venc.h"
#include "rk_mpi_vi.h"
#include "rk_mpi_vpss.h"
#include "rk_type.h"
#include "rtsp_demo.h"
#include "stdio.h"
#include <fcntl.h>
#include <rk_aiq_user_api2_camgroup.h>
#include <rk_aiq_user_api2_imgproc.h>
#include <rk_aiq_user_api2_sysctl.h>
#include <stdbool.h>
#include <unistd.h>

#define WIDTH 1920
#define HEIGH 1080
#define PIXEL_FMT RK_FMT_YUV420SP
#define ENCODE_TYPE RK_VIDEO_ID_AVC

int venc_chn_id = 0;
#define RK_ALIGN(x, a) (((x) + (a) - 1) & ~((a) - 1))
#define RK_ALIGN_2(x) RK_ALIGN(x, 2)

int init_venc(void) {
  VENC_CHN_ATTR_S attr = {0};
  VENC_RC_PARAM_S rc_param = {0};
  VENC_RECV_PIC_PARAM_S recv_param = {0};
  attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
  attr.stRcAttr.stH264Cbr.u32Gop = 60;
  attr.stRcAttr.stH264Cbr.u32BitRate = 4 * 1024;
  attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
  attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = 30;
  attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
  attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = 30;

  attr.stVencAttr.enType = ENCODE_TYPE;
  attr.stVencAttr.enPixelFormat = PIXEL_FMT;
  attr.stVencAttr.u32Profile = 100;
  attr.stVencAttr.u32MaxPicWidth = WIDTH;
  attr.stVencAttr.u32MaxPicHeight = HEIGH;
  attr.stVencAttr.u32PicWidth = WIDTH;
  attr.stVencAttr.u32PicHeight = HEIGH;
  attr.stVencAttr.u32VirWidth = WIDTH;
  attr.stVencAttr.u32VirHeight = HEIGH;
  attr.stVencAttr.u32StreamBufCnt = 3;
  attr.stVencAttr.u32BufSize = WIDTH * HEIGH * 3 / 2;

  attr.stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
  if (RK_SUCCESS != RK_MPI_VENC_CreateChn(venc_chn_id, &attr)) {
    printf("create venc chn fail\n");
    return -1;
  }

  rc_param.stParamH264.u32MinQp = 10;
  rc_param.stParamH264.u32MaxQp = 51;
  rc_param.stParamH264.u32MinIQp = 10;
  rc_param.stParamH264.u32MaxIQp = 51;
  rc_param.stParamH264.u32FrmMinQp = 28;
  rc_param.stParamH264.u32FrmMinIQp = 28;

  if (RK_SUCCESS != RK_MPI_VENC_SetRcParam(venc_chn_id, &rc_param)) {
    printf("set rc param fail\n");
    return -1;
  }
  recv_param.s32RecvPicNum = -1;
  RK_MPI_VENC_StartRecvFrame(venc_chn_id, &recv_param);

  return 0;
}

int deinit_venc(void) {
  RK_MPI_VENC_StopRecvFrame(venc_chn_id);
  RK_MPI_VENC_DestroyChn(venc_chn_id);
}

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

int vi_dev_id = 0;
int vi_pipe_id = 0;
int vi_chn_id = 0;

int init_vi(void) {
  VI_DEV_ATTR_S dev_attr = {0};
  int ret;
  VI_DEV_BIND_PIPE_S bind = {0};
  VI_CHN_ATTR_S chn_attr = {0};

  ret = RK_MPI_VI_GetDevAttr(vi_dev_id, &dev_attr);
  if (ret == RK_ERR_VI_NOT_CONFIG) {
    // the dev_attr has no effect,but we still need to call this funcion
    ret = RK_MPI_VI_SetDevAttr(vi_dev_id, &dev_attr);
    if (ret != RK_SUCCESS) {
      RK_LOGE("set vi dev attr fail 0x%x", ret);
      return ret;
    }
  }
  ret = RK_MPI_VI_GetDevIsEnable(vi_dev_id);
  if (ret != RK_SUCCESS) {
    // start vi
    ret = RK_MPI_VI_EnableDev(vi_dev_id);
    if (ret != RK_SUCCESS) {
      RK_LOGE("enable vi dev fail 0x%x", ret);
      return ret;
    }
    bind.u32Num = 1;
    // pipe id is the same as vi_dev_id
    bind.PipeId[0] = vi_pipe_id;
    ret = RK_MPI_VI_SetDevBindPipe(vi_dev_id, &bind);
    if (ret != RK_SUCCESS) {
      RK_LOGE("bind vi pipe fail 0x%x", ret);
      return ret;
    }
  }

  chn_attr.stSize.u32Width = WIDTH;
  chn_attr.stSize.u32Height = HEIGH;
  chn_attr.stIspOpt.stMaxSize.u32Width = WIDTH;
  chn_attr.stIspOpt.stMaxSize.u32Height = HEIGH;
  chn_attr.stIspOpt.u32BufCount = 3;
  chn_attr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;
  chn_attr.u32Depth = 2;
  chn_attr.enPixelFormat = PIXEL_FMT;
  chn_attr.enCompressMode = COMPRESS_MODE_NONE;
  chn_attr.stFrameRate.s32SrcFrameRate = -1;
  chn_attr.stFrameRate.s32DstFrameRate = -1;

  ret = RK_MPI_VI_SetChnAttr(vi_pipe_id, vi_chn_id, &chn_attr);

  ret |= RK_MPI_VI_EnableChn(vi_pipe_id, vi_chn_id);
  if (ret) {
    RK_LOGE("create vi fail 0x%x", ret);
  }
  return ret;
}

int deinit_vi(void) {
  RK_MPI_VI_DisableChn(vi_pipe_id, vi_chn_id);

  RK_MPI_VI_DisableDev(vi_dev_id);

  return 0;
}

int dump_file(void *data, int len) {
  static int num = 0;
  int fd;
  char path[128] = {0};

  snprintf(path, 128, "./dump_%d", num++);
  fd = open(path, O_RDWR | O_CREAT, 0644);
  if (fd < 0) {
    RK_LOGE("open file %s fail %s", path, strerror(errno));
    return -1;
  }
  write(fd, data, len);
  close(fd);
  return 0;
}

int dump_vi_frame(void) {
  VIDEO_FRAME_INFO_S info;
  int ret;
  void *data;
  ret = RK_MPI_VI_GetChnFrame(vi_pipe_id, vi_chn_id, &info, 5000);
  if (ret != RK_SUCCESS) {
    RK_LOGE("get vi frame fail 0x%x", ret);
    return -1;
  }
  RK_LOGI("private data=%d, calculate size = %d", info.stVFrame.u64PrivateData,
          1920 * 1080 * 3 / 2);
  data = RK_MPI_MB_Handle2VirAddr(info.stVFrame.pMbBlk);
  if (0 == dump_file(data, info.stVFrame.u64PrivateData))
    RK_LOGI("dump vi frame success ");
  RK_MPI_VI_ReleaseChnFrame(vi_pipe_id, vi_chn_id, &info);
  return 0;
}

int vi_bind_vpss(void) {
  MPP_CHN_S stSrcChn, stDestChn;
  // bind vi to venc
  stSrcChn.enModId = RK_ID_VI;
  stSrcChn.s32DevId = vi_dev_id;
  stSrcChn.s32ChnId = vi_chn_id;

  stDestChn.enModId = RK_ID_VPSS;
  stDestChn.s32DevId = 0;
  stDestChn.s32ChnId = 0;

  RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
  return 0;
}

int vi_unbind_vpss(void) {
  MPP_CHN_S stSrcChn, stDestChn;
  stSrcChn.enModId = RK_ID_VI;
  stSrcChn.s32DevId = vi_dev_id;
  stSrcChn.s32ChnId = vi_chn_id;

  stDestChn.enModId = RK_ID_VPSS;
  stDestChn.s32DevId = 0;
  stDestChn.s32ChnId = 0;

  RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
  return 0;
}

int vpss_bind_venc(void) {
  MPP_CHN_S stSrcChn, stDestChn;
  // bind vi to venc
  stSrcChn.enModId = RK_ID_VPSS;
  stSrcChn.s32DevId = vi_dev_id;
  stSrcChn.s32ChnId = vi_chn_id;

  stDestChn.enModId = RK_ID_VENC;
  stDestChn.s32DevId = 0;
  stDestChn.s32ChnId = 0;

  RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
  return 0;
}

int vpss_unbind_venc(void) {
  MPP_CHN_S stSrcChn, stDestChn;
  stSrcChn.enModId = RK_ID_VPSS;
  stSrcChn.s32DevId = vi_dev_id;
  stSrcChn.s32ChnId = vi_chn_id;

  stDestChn.enModId = RK_ID_VENC;
  stDestChn.s32DevId = 0;
  stDestChn.s32ChnId = 0;

  RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
  return 0;
}

int rtsp_port_id = 554;
rtsp_demo_handle rtsp_handle;
rtsp_session_handle session_handle;

int init_rtsp(void) {
  rtsp_handle = create_rtsp_demo(rtsp_port_id);
  session_handle = rtsp_new_session(rtsp_handle, "/live/0");
  rtsp_set_video(session_handle, RTSP_CODEC_ID_VIDEO_H264, NULL, 0);
  rtsp_sync_video_ts(session_handle, rtsp_get_reltime(), rtsp_get_ntptime());
  return 0;
}

int deinit_rtsp(void) {
  rtsp_del_demo(rtsp_handle);
  return 0;
}

int venc_data_process(void) {
  int ret = 0;
  VENC_STREAM_S stream;
  void *data;
  int q = 1000;

  stream.pstPack = malloc(sizeof(VENC_PACK_S));
  while (q--) {
    ret = RK_MPI_VENC_GetStream(venc_chn_id, &stream, 1000);
    if (ret != RK_SUCCESS) {
      RK_LOGW("venc get stream fail 0x%x", ret);
      continue;
    }
    data = RK_MPI_MB_Handle2VirAddr(stream.pstPack->pMbBlk);
    rtsp_tx_video(session_handle, data, stream.pstPack->u32Len,
                  stream.pstPack->u64PTS);
    rtsp_do_event(rtsp_handle);
    RK_MPI_VENC_ReleaseStream(venc_chn_id, &stream);
  }
  free(stream.pstPack);

  return 0;
}

int vpss_group = 0;
int vpss_chn_id = 0;
int init_vpss(void) {
  int ret;
  VPSS_GRP_ATTR_S grp_attr = {0};
  VPSS_CHN_ATTR_S chn_attr = {0};

  grp_attr.u32MaxW = WIDTH;
  grp_attr.u32MaxH = HEIGH;
  grp_attr.enPixelFormat = PIXEL_FMT;
  grp_attr.stFrameRate.s32SrcFrameRate = -1;
  grp_attr.stFrameRate.s32DstFrameRate = -1;
  grp_attr.enCompressMode = COMPRESS_MODE_NONE;

  ret = RK_MPI_VPSS_CreateGrp(vpss_group, &grp_attr);
  if (ret != RK_SUCCESS) {
    RK_LOGE("vpss create group fail 0x%x", ret);
    return -1;
  }
  chn_attr.enChnMode = VPSS_CHN_MODE_PASSTHROUGH;
  chn_attr.enDynamicRange = DYNAMIC_RANGE_SDR8;
  chn_attr.enPixelFormat = PIXEL_FMT;
  chn_attr.stFrameRate.s32SrcFrameRate = -1;
  chn_attr.stFrameRate.s32DstFrameRate = -1;
  chn_attr.u32Width = WIDTH;
  chn_attr.u32Height = HEIGH;
  chn_attr.enCompressMode = COMPRESS_MODE_NONE;

  ret = RK_MPI_VPSS_SetChnAttr(vpss_group, vpss_chn_id, &chn_attr);
  if (ret != RK_SUCCESS) {
    RK_LOGE("fail to set chn %d ret 0x%x", vpss_chn_id, ret);
    return -1;
  }

  ret = RK_MPI_VPSS_EnableChn(vpss_group, vpss_chn_id);
  if (ret != RK_SUCCESS) {
    RK_LOGE("start chn %d fail 0x%x", vpss_chn_id, ret);
    return -1;
  }

  ret = RK_MPI_VPSS_StartGrp(vpss_group);
  if (ret != RK_SUCCESS) {
    RK_LOGE("fail to start group");
  }
  return ret;
}

int deinit_vpss(void) {
  RK_MPI_VPSS_StopGrp(vpss_group);
  RK_MPI_VPSS_DestroyGrp(vpss_group);
  return 0;
}

int main() {
  char input = 0;
  printf("hello world\n");

  init_rtsp();
  RK_MPI_SYS_Init();
  init_aiq();
  run_aiq();
  init_vi();
  dump_vi_frame();
  init_vpss();
  init_venc();
  // do some things
  vi_bind_vpss();
  vpss_bind_venc();
  venc_data_process();
  while (input != 'q') {
    printf("input q to quit\n");
    scanf("%c", &input);
  }
  vpss_unbind_venc();
  vi_unbind_vpss();
  deinit_venc();
  deinit_vpss();
  deinit_vi();
  stop_aiq();
  deinit_aiq();
  RK_MPI_SYS_Exit();
  deinit_rtsp();
  return 0;
}
