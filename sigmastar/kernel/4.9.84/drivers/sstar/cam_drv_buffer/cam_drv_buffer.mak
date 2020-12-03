#-------------------------------------------------------------------------------
#	Description of some variables owned by the library
#-------------------------------------------------------------------------------
# Library module (lib) or Binary module (bin)
PROCESS = lib
PATH_C +=\
  $(PATH_cam_drv_buffer)/src \
  $(PATH_cam_drv_buffer)/test

PATH_H +=\
  $(PATH_cam_drv_buffer)/pub \
  $(PATH_cam_drv_buffer)/inc

#-------------------------------------------------------------------------------
#	List of source files of the library or executable to generate
#-------------------------------------------------------------------------------
SRC_C_LIST =\
  cam_drv_buffer.c \
  cam_drv_buffer_test.c