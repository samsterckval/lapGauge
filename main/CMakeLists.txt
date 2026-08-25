idf_component_register(
    SRCS "main.c" "tca9554.c" "debug_info_view.cpp" "debug_ui.cpp"
         "imu/qmi8658_sensor.cpp" "imu/ball_view.cpp" "imu/imu_service.cpp" "imu/imu_ui.cpp"
    INCLUDE_DIRS "."
)
