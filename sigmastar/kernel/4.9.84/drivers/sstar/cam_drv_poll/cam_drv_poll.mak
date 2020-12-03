
#-------------------------------------------------------------------------------
#	Description of some variables owned by the library
#-------------------------------------------------------------------------------
# Library module (lib) or Binary module (bin)
PROCESS = lib
PATH_C +=\
  $(PATH_cam_drv_poll)/src \
  $(PATH_cam_drv_poll)/sample_driver/src/drv/common \
  $(PATH_cam_drv_poll)/sample_driver/src/drv/rtk \
  $(PATH_cam_drv_poll)/sample_driver/test/camdrvpolltest

PATH_H +=\
  $(PATH_cam_os_wrapper)/pub \
  $(PATH_cam_dev_wrapper)/pub \
  $(PATH_cam_drv_poll)/pub \
  $(PATH_cam_drv_poll)/sample_driver/inc \
  $(PATH_cam_drv_poll)/sample_driver/pub

#-------------------------------------------------------------------------------
#	List of source files of the library or executable to generate
#-------------------------------------------------------------------------------
SRC_C_LIST =\
  cam_drv_poll.c \
  drv_pollsample_dev.c \
  drv_pollsample_module.c \
  cam_drv_poll_test.c
