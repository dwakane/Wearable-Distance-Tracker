#include <mbed.h>
#include "challenge.h"
using namespace std::chrono;


SPI spi(GYRO_MOSI, GYRO_MISO, GYRO_SCLK);

InterruptIn i2_drdy(GYRO_DRDY);

DigitalOut cs(GYRO_CS);

Semaphore sem_drdy(0);
Semaphore sem_rx(0);

volatile int16_t readings[3*SAMPLES];
char * rx_buffer = (char*) readings;
uint32_t buffered_in, fifo_fill;
volatile uint16_t temperature;

DigitalOut led1(LED1);
DigitalOut led2(LED2);
FlashIAP flash;

InterruptIn button(USER_BUTTON);
Timer timer;
Timer button_timer;
Thread thread1;
Thread thread2;
Thread thread3;
Thread thread4;

Semaphore sem_calc(0);
Semaphore sem_main(0);
Semaphore sem_button(0);

char storage_buffer[STORAGE_SIZE];
float *distance_buffer = (float*) &storage_buffer[8];
int64_t *saved_tally = (int64_t*) storage_buffer;
uint32_t saved;

volatile uint32_t feet;
volatile bool lap_flag = false;
volatile bool save_flag = false;
volatile bool print_flag = false;
volatile bool erase_flag = false;
volatile bool button_flag = false;

void rx_thread();
void check_status();
void gyro_config();
void rx_handler(int i);
void i2_handler();
void calc_thread();
void debounce_thread();

void button_rise_handler() {
    sem_button.release();
}

void button_fall_handler() {
    sem_button.release();
}

void i2_handler() {
    sem_drdy.release();
}

void rx_handler(int i) {
    sem_rx.release();
}

const event_callback_t rx_callback= callback(rx_handler);

int main() {
    flash.init();
    /*
    flash.read(storage_buffer,STORAGE_BASE,STORAGE_SIZE);
    if( 0 == *saved_tally) {
        saved = 64;
    }
    else {
        saved = log2(- *saved_tally);
    }
    */

    buffered_in=0;
    feet = 0;
    thread1.start(&rx_thread);
    thread2.start(&calc_thread);
    thread3.start(&debounce_thread);
    button.rise(&button_rise_handler);
    button.fall(&button_fall_handler);
    
    while(1) {
        sem_main.acquire();

        if(lap_flag) {

            if( saved < DISTANCE_BUFFER_SIZE) {
                distance_buffer[saved++] = feet;
                *saved_tally = (*saved_tally) << 1;
            }
            lap_flag = false;
            feet = 0;
        }

        if(save_flag) {
            flash.program(storage_buffer,STORAGE_BASE,STORAGE_SIZE);
            save_flag = false;
        }

        if(print_flag) {
            for( uint32_t i = 0 ; i < saved; i++) {
                printf("Lap: %lu: Distance: %f feet\n", i+1, distance_buffer[i]);
            }
            print_flag = false;
        }

        if(erase_flag) {
            flash.erase(SECTOR, SECTOR_SIZE);
            saved = 0;
            *saved_tally = -1;
            erase_flag = false;
        }



        /*
    
        printf("Buffered: %d\tIn: %lu\tX: %8d\tY: %8d\tZ: %8d\n",
                            n, m, avg_x, avg_y, avg_z);
        */
    }
}
    void debounce_thread() {
        while(1) {
            sem_button.acquire();
            printf("button");
        }
    }

//void debounce_thread() {
//    int button_pushed = button.read();
//    int button_temp;
//    uint32_t button_time = 0;
//    Timer button_timer;
//    if(button_pushed) {
//        timer.reset();
//    }
//    while(1) {
//        printf("debounce,");
//        sem_button.acquire();
//        ThisThread::sleep_for(DEBOUNCE_TIME);
//        button_temp = button.read();
//        if (button_pushed == button_temp) {
//            continue;
//        }
//        if(button_pushed) {
//            button_timer.start();
//        }
//        else {
//            button_time = BUTTON_MILLIS();
//            button_timer.stop();
//            button_timer.reset();
//
//            if(button_time > BUTTON_ABORT_TIME)
//                continue;
//            if(button_time > BUTTON_ERASE_TIME) {
//                erase_flag = true;
//                sem_main.release();
//                continue;
//            }
//            if(button_time > BUTTON_SAVE_TIME) {
//                save_flag = true;
//                sem_main.release();
//                continue;
//            }
//            if(button_time > BUTTON_PRINT_TIME) {
//                print_flag = true;
//                sem_main.release();
//                continue;
//            }
//            lap_flag = true;
//            sem_main.release();
//        }
//    }
//}

void calc_thread() {
    uint32_t avg_abs_w = 0;
    uint32_t avg_fps;
    uint32_t millis;
    timer.start();

    while(1) {
        sem_calc.acquire();

        for (uint32_t i = 0; i < buffered_in; i++) {
            avg_abs_w += abs(readings[i]);
        }

        avg_abs_w/=buffered_in;
        buffered_in=0;
        avg_fps = avg_abs_w*FPS_SCALE;
        millis = duration_cast<milliseconds>(timer.elapsed_time()).count();
        timer.reset();
        feet += avg_fps*millis/1000;
    }

}

void rx_thread() {
    int fifo_src;  
    i2_drdy.rise(&i2_handler);
    cs=1;
    spi.format(8,3);
    spi.frequency();
    gyro_config();
    spi.set_default_write_value(0x00);
    while(1) {
        sem_drdy.try_acquire_for(45ms);                 //missed deadline just go
        printf("hello\n");
        cs=0;
        spi.write(READ|FIFO_SRC_REG);
        fifo_src = spi.write(0x00);
        cs=1;
        fifo_fill = fifo_src & FSS;
        if (fifo_fill==0) continue;
        if (fifo_fill == 31 && fifo_src & OVRN) fifo_fill = 32;
        fifo_fill = min(fifo_fill, SAMPLES-buffered_in);
        cs=0;
        spi.write(READ|MULTI|OUT_X_L);
        spi.transfer(
                        (char *)NULL,
                        0,
                        rx_buffer+ buffered_in*6,
                        6*fifo_fill,
                        rx_callback,
                        SPI_EVENT_COMPLETE
                    );
        sem_rx.acquire();
        cs=1;
        cs=0;
        spi.write(READ|OUT_TEMP);
        temperature = spi.write(0x00);
        cs=1;
        buffered_in += fifo_fill;
//        clear();
        if(buffered_in >= SAMPLES)
            sem_calc.release();
    }
}

void gyro_config() {
    cs=0;
    spi.write(WRITE|CTRL_REG1);                     
    spi.write(DR_800|PD|ZEN|YEN|XEN);                      //CTRL_REG1
    cs=1;
    cs=0;
    spi.write(WRITE|CTRL_REG3);
    spi.write(I2_WTM);                             //CTRL_REG3
    cs=1;
    cs=0;
    spi.write(WRITE|CTRL_REG4);
    spi.write(FS_500);
    cs=1;
    cs=0;
    spi.write(WRITE|CTRL_REG5);
    spi.write(FIFO_EN);                             //CTRL_REG5
    cs=1;
    cs=0;
    spi.write(WRITE|FIFO_CTRL_REG);
    spi.write(BYPASS_MODE);
    cs=1;
    cs=0;
    spi.write(WRITE|FIFO_CTRL_REG);
    spi.write(STREAM_MODE|24);
    cs=1;
}

void check_status() {
    cs=0;
    spi.write(READ|STATUS_REG);
    int status = spi.write(0x00);
    cs=1;
    cs=0;
    spi.write(READ|FIFO_SRC_REG);
    int fifo_src = spi.write(0x00);
    cs=1;
    printf("STATUS: %x\tFIFO: %x\n",status,fifo_src);
}

void clear() {
    cs=0;
    spi.write(WRITE|FIFO_CTRL_REG);
    spi.write(BYPASS_MODE);
    spi.write(FIFO_MODE);
    cs=1;
}

