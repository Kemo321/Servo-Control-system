#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define GPIO_EXPORT "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define GPIO23_DIR "/sys/class/gpio/gpio23/direction"
#define GPIO23_VAL "/sys/class/gpio/gpio23/value"
#define GPIO24_DIR "/sys/class/gpio/gpio24/direction"
#define GPIO24_VAL "/sys/class/gpio/gpio24/value"

#define PWM_EXPORT "/sys/class/pwm/pwmchip0/export"
#define PWM_PERIOD "/sys/class/pwm/pwmchip0/pwm0/period"
#define PWM_DUTY "/sys/class/pwm/pwmchip0/pwm0/duty_cycle"
#define PWM_ENABLE "/sys/class/pwm/pwmchip0/pwm0/enable"

int write_to_file(const char *path, const char *value) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        fprintf(stderr, "Error opening file for writing: %s\n", path);
        return -1;
    }
    ssize_t n = write(fd, value, strlen(value));
    if (n < 0 || (size_t)n != strlen(value)) {
        fprintf(stderr, "Error writing to file: %s\n", path);
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int read_from_file(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Error opening file for reading: %s\n", path);
        return -1;
    }
    char buf[2];
    ssize_t n = read(fd, buf, 1);
    close(fd);
    if (n != 1 || (buf[0] != '0' && buf[0] != '1')) {
        fprintf(stderr, "Invalid value in file: %s\n", path);
        return -1;
    }
    return buf[0] - '0'; // Convert '0'/'1' to 0/1
}

void cleanup() {
    write_to_file(PWM_ENABLE, "0");
    write_to_file(PWM_EXPORT, "0");
    write_to_file(GPIO_UNEXPORT, "23");
    write_to_file(GPIO_UNEXPORT, "24");
    exit(0);
}

int main() {
    signal(SIGINT, cleanup);

    if (write_to_file(GPIO_EXPORT, "23") < 0 || write_to_file(GPIO_EXPORT, "24") < 0) {
        fprintf(stderr, "Error exporting GPIO\n");
        return 1;
    }
    if (write_to_file(GPIO23_DIR, "in") < 0 || write_to_file(GPIO24_DIR, "in") < 0) {
        fprintf(stderr, "Error setting GPIO direction\n");
        return 1;
    }

    if (write_to_file(PWM_EXPORT, "0") < 0 || 
        write_to_file(PWM_PERIOD, "20000000") < 0 || 
        write_to_file(PWM_ENABLE, "1") < 0) {
        fprintf(stderr, "Error initializing PWM\n");
        return 1;
    }

    int position = 0;
    int prev_clk = read_from_file(GPIO23_VAL);
    if (prev_clk < 0) prev_clk = 0;

    while (1) {
        int clk = read_from_file(GPIO23_VAL);
        int dt = read_from_file(GPIO24_VAL);
        if (clk < 0 || dt < 0) {
            fprintf(stderr, "Error reading GPIO, skipping iteration\n");
            usleep(1000);
            continue;
        }

        printf("CLK: %d, DT: %d, Position: %d\n", clk, dt, position);

        if (clk != prev_clk && clk == 1) {
            if (dt == 0) {
                if (position < 180) position += 5;
            } else {
                if (position > 0) position -= 5;
            }

            int duty_cycle = 500000 + (position * 2000000 / 180);
            if (duty_cycle < 500000) duty_cycle = 500000;
            if (duty_cycle > 2500000) duty_cycle = 2500000;

            printf("Duty Cycle: %d (Position: %d)\n", duty_cycle, position);

            char duty_str[10];
            sprintf(duty_str, "%d", duty_cycle);
            if (write_to_file(PWM_DUTY, duty_str) < 0) {
                fprintf(stderr, "Error writing duty_cycle, continuing\n");
            }
        }

        prev_clk = clk;
        usleep(1000);
    }

    return 0;
}
