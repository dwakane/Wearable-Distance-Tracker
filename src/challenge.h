

#define WHO_AM_I 0x0F
#define WRITE 0x00
#define READ 0x80
#define SINGLE 0x00
#define MULTI 0x40

#define CTRL_REG1 0x20
#define DR_100 0x00
#define DR_200 0x40
#define DR_400 0x80
#define DR_800 0xC0
#define PD 0x08
#define ZEN 0x04
#define YEN 0x02
#define XEN 0x01

#define CTRL_REG3 0x22
#define I2_WTM 0x04
#define I2_ORUN 0x02
#define I2_DRDY 0x08

#define CTRL_REG4 0x23
#define FS_245 0x00
#define FS_500 0x10
#define FS_2000 0x20
#define NORMAL_MODE 0x00
#define SELF_TEST_0 0x02
#define SELF_TEST_1 0x06

#define CTRL_REG5 0x24
#define BOOT 0x80
#define FIFO_EN 0x40

#define OUT_TEMP 0x26

#define STATUS_REG 0x27

#define OUT_X_L 0x28
#define OUT_X_H 0x29
#define OUT_Y_L 0x2A
#define OUT_Y_H 0x2B
#define OUT_Z_L 0x2C
#define OUT_Z_H 0x2D

#define FIFO_CTRL_REG 0x2E
#define BYPASS_MODE 0x00
#define FIFO_MODE 0x20
#define STREAM_MODE 0x40

#define FIFO_SRC_REG 0x2F
#define WTM 0x80
#define OVRN 0x40
#define EMPTY 0x20
#define FSS 0x1F

#define GYRO_MOSI PF_9
#define GYRO_MISO PF_8
#define GYRO_SCLK PF_7
#define GYRO_CS PC_1
#define GYRO_INT1 PA_1
#define GYRO_INT2 PA_2
#define GYRO_DRDY PA_2

#define ODR 840                         //Hz
#define DURATION 1/2                    //Seconds
#define SAMPLES ODR*DURATION

#define FPS_SCALE 0.0013351785325656327 //
#define STORAGE_BASE 0x81E0000  
#define SECTOR FLASH_SECTOR_23
#define SECTOR_SIZE 0x20000 
#define DISTANCE_BUFFER_SIZE 64
#define STORAGE_SIZE DISTANCE_BUFFER_SIZE*4+8

#define DEBOUNCE_TIME 300ms
#define BUTTON_PRINT_TIME 2000
#define BUTTON_SAVE_TIME 5000
#define BUTTON_ERASE_TIME 10000
#define BUTTON_ABORT_TIME 12000
#define BUTTON_MILLIS() duration_cast<milliseconds>(button_timer.elapsed_time()).count()


