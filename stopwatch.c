#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <signal.h>
#include <fcntl.h>

// ============================================================================
// CUSTOMIZATION OPTIONS
// ============================================================================

// You can change the color by modifying the ANSI escape code below.
// Some examples:
// "\033[30m" = Black
// "\033[31m" = Red
// "\033[32m" = Green
// "\033[33m" = Yellow
// "\033[34m" = Blue
// "\033[36m" = Cyan (Default)
// "\033[1;37m" = Bright White
#define COLOR_NUMBERS "\033[36m"
#define COLOR_RESET   "\033[0m"

// The character used to draw the numbers. A solid block works best.
#define CHAR_FILL "█" 

// ============================================================================

#define MAX_ROWS 500
#define MAX_COLS 1000

char screen[MAX_ROWS][MAX_COLS];
struct termios orig_termios;
volatile sig_atomic_t running = 1;

// 7-segment definitions for digits 0-9
// Segments: 0:Top, 1:Top-L, 2:Top-R, 3:Mid, 4:Bot-L, 5:Bot-R, 6:Bot
const int segs[10][7] = {
    {1, 1, 1, 0, 1, 1, 1}, // 0
    {0, 0, 1, 0, 0, 1, 0}, // 1
    {1, 0, 1, 1, 1, 0, 1}, // 2
    {1, 0, 1, 1, 0, 1, 1}, // 3
    {0, 1, 1, 1, 0, 1, 0}, // 4
    {1, 1, 0, 1, 0, 1, 1}, // 5
    {1, 1, 0, 1, 1, 1, 1}, // 6
    {1, 0, 1, 0, 0, 1, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 0, 1, 1}  // 9
};

// --- Terminal & Signal Management ---

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    printf("\033[?25h"); // Show cursor
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    
    // Non-blocking stdin
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    
    printf("\033[?25l"); // Hide cursor
}

void handle_sigint(int sig) {
    running = 0;
}

long get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000L) + (tv.tv_usec / 1000L);
}

// --- Drawing Functions ---

void clear_screen_buffer() {
    for (int r = 0; r < MAX_ROWS; r++) {
        for (int c = 0; c < MAX_COLS; c++) {
            screen[r][c] = ' ';
        }
    }
}

void draw_digit(int digit, int x, int y, int w, int h) {
    if (digit < 0 || digit > 9) return;
    const int *s = segs[digit];
    int mid_y = y + h / 2;

    if (s[0]) for (int i = 0; i < w; i++) if(y < MAX_ROWS && x+i < MAX_COLS) screen[y][x+i] = '#';
    if (s[1]) for (int i = 0; i < h/2 + 1; i++) if(y+i < MAX_ROWS && x < MAX_COLS) screen[y+i][x] = '#';
    if (s[2]) for (int i = 0; i < h/2 + 1; i++) if(y+i < MAX_ROWS && x+w-1 < MAX_COLS) screen[y+i][x+w-1] = '#';
    if (s[3]) for (int i = 0; i < w; i++) if(mid_y < MAX_ROWS && x+i < MAX_COLS) screen[mid_y][x+i] = '#';
    if (s[4]) for (int i = h/2; i < h; i++) if(y+i < MAX_ROWS && x < MAX_COLS) screen[y+i][x] = '#';
    if (s[5]) for (int i = h/2; i < h; i++) if(y+i < MAX_ROWS && x+w-1 < MAX_COLS) screen[y+i][x+w-1] = '#';
    if (s[6]) for (int i = 0; i < w; i++) if(y+h-1 < MAX_ROWS && x+i < MAX_COLS) screen[y+h-1][x+i] = '#';
}

void draw_colon(int x, int y, int h) {
    int p1 = y + h / 3;
    int p2 = y + (2 * h) / 3;
    if (p1 < MAX_ROWS && x+1 < MAX_COLS) { screen[p1][x] = '#'; screen[p1][x+1] = '#'; }
    if (p2 < MAX_ROWS && x+1 < MAX_COLS) { screen[p2][x] = '#'; screen[p2][x+1] = '#'; }
}

void draw_dot(int x, int y, int h) {
    int p = y + h - 1;
    if (p < MAX_ROWS && x+1 < MAX_COLS) { screen[p][x] = '#'; screen[p][x+1] = '#'; }
}

// --- Main Program ---

int main() {
    enable_raw_mode();
    signal(SIGINT, handle_sigint);

    long start_time = get_time_ms();
    long last_lap_time = start_time;

    char laps[1000][128];
    int lap_count = 0;

    // Clear physical terminal once cleanly
    printf("\033[2J\033[H");

    while (running) {
        long current = get_time_ms();
        long total_ms = current - start_time;
        long delta_ms = current - last_lap_time;

        // Process Input
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) {
            if (c == 'p' || c == 'P') {
                int t_m = (total_ms / 60000) % 100;
                int t_s = (total_ms / 1000) % 60;
                int t_cs = (total_ms % 1000) / 10;
                
                int d_m = (delta_ms / 60000) % 100;
                int d_s = (delta_ms / 1000) % 60;
                int d_cs = (delta_ms % 1000) / 10;
                
                snprintf(laps[lap_count++], 128, 
                    "Timestamp: %02d:%02d:%02d, delta: %02d:%02d:%02d", 
                    t_m, t_s, t_cs, d_m, d_s, d_cs);
                last_lap_time = current;
            }
        }

        // Fetch terminal size
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int rows = w.ws_row;
        int cols = w.ws_col;
        if (rows > MAX_ROWS) rows = MAX_ROWS;
        if (cols > MAX_COLS) cols = MAX_COLS;

        // Calculate layout dynamically
        int W_L = (cols - 20) / 5; 
        int H_L = W_L * 2;
        if (H_L > rows - 10) { 
            H_L = rows - 10;
            W_L = H_L / 2;
        }
        if (W_L < 3) W_L = 3;
        if (H_L < 5) H_L = 5;

        int W_S = W_L / 2;
        if (W_S < 2) W_S = 2;
        int H_S = H_L / 2;
        if (H_S < 3) H_S = 3;

        int total_width = 4*W_L + 2*W_S + 18;
        int start_x = (cols - total_width) / 2;
        if (start_x < 0) start_x = 0;
        int start_y = 1; 

        // Current time breakdown
        int m1 = ((total_ms / 60000) % 100) / 10;
        int m2 = ((total_ms / 60000) % 100) % 10;
        int s1 = ((total_ms / 1000) % 60) / 10;
        int s2 = ((total_ms / 1000) % 60) % 10;
        int cs1 = ((total_ms % 1000) / 10) / 10;
        int cs2 = ((total_ms % 1000) / 10) % 10;

        clear_screen_buffer();

        // Draw Clock to buffer
        int x = start_x;
        draw_digit(m1, x, start_y, W_L, H_L); x += W_L + 2;
        draw_digit(m2, x, start_y, W_L, H_L); x += W_L + 2;
        draw_colon(x, start_y, H_L);          x += 4;
        draw_digit(s1, x, start_y, W_L, H_L); x += W_L + 2;
        draw_digit(s2, x, start_y, W_L, H_L); x += W_L + 2;
        draw_dot(x, start_y, H_L);            x += 4;
        
        // Small decimals align to the bottom of the large numbers
        int small_y = start_y + (H_L - H_S);
        draw_digit(cs1, x, small_y, W_S, H_S); x += W_S + 2;
        draw_digit(cs2, x, small_y, W_S, H_S);

        // Render buffer to screen
        printf("\033[H"); // Move cursor to top-left
        for (int r = 0; r < H_L + 2; r++) {
            printf("%s", COLOR_NUMBERS);
            for (int c = 0; c < cols; c++) {
                if (screen[r][c] == '#') fputs(CHAR_FILL, stdout);
                else putchar(' ');
            }
            printf("%s\033[K\n", COLOR_RESET); // Clear to end of line, then newline
        }

        // Draw Lap Logs below the clock
        int laps_to_show = rows - (H_L + 4); 
        if (laps_to_show > 0) {
            int start_idx = (lap_count > laps_to_show) ? lap_count - laps_to_show : 0;
            for (int i = start_idx; i < lap_count; i++) {
                printf(" %s\033[K\n", laps[i]);
            }
        }
        
        printf("\033[J"); // Clear remaining screen space down to the bottom
        fflush(stdout);
        usleep(16000); // ~60 FPS
    }

    // Ctrl+C Pressed - Final Exit Sequence
    long final_time = get_time_ms();
    long total_ms = final_time - start_time;
    long delta_ms = final_time - last_lap_time;

    int t_m = (total_ms / 60000) % 100;
    int t_s = (total_ms / 1000) % 60;
    int t_cs = (total_ms % 1000) / 10;
    
    int d_m = (delta_ms / 60000) % 100;
    int d_s = (delta_ms / 1000) % 60;
    int d_cs = (delta_ms % 1000) / 10;

    printf("\n\nTimestamp: %02d:%02d:%02d, delta: %02d:%02d:%02d\n", 
            t_m, t_s, t_cs, d_m, d_s, d_cs);
    
    return 0;
}
