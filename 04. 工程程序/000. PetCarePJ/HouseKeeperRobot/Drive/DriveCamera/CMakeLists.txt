# 查找当前目录下的所有源文件
# 并将名称保存到 SRC_LIST 变量
aux_source_directory(. SRC_DRIVE_CAMERA_LIST)
# 生成链接库
add_library (DriveCamera ${SRC_DRIVE_CAMERA_LIST})
