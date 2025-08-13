// Code was taken from https://www.a1k0n.net/2011/07/20/donut-math.html
// and adapted to C++

#include <iostream>
#include <windows.h>
#include <cmath>
#include <vector>
#include <conio.h>

// If on Windows machine
#ifdef _WIN32
    #include <windows.h>
// If on Linux for example
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
    #include <cstdio>
#endif 

using namespace std;

// Window size
int width = 40;
int height = 40;

const float theta_interval = 0.07f;
const float phi_interval = 0.03f;

// Inner torus' circle radius
const int R1 = 1;
// Distance between hypotetical center of torus and center of in inner torus' circle
const int R2 = 2;
// Distance between viewer and torus
const int K2 = 5;

// Calculate K1 based on screen size: the maximum x-distance occurs
// roughly at the edge of the torus, which is at x=R1+R2, z=0.  we
// want that to be displaced 3/8ths of the width of the screen, which
// is 3/4th of the way from the center to the side of the screen.
// screen_width*3/8 = K1*(R1+R2)/(K2+0)
// screen_width*K2*3/(8*(R1+R2)) = K1
const float K1 = height * K2 * 3 / (8 * (R1 + R2));

const float pi = 3.14f;


void SetConsoleWindowSize() {
    #ifdef _WIN32
        // Get reference to console window
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        // Set new size
        COORD bufferSize = { static_cast<SHORT>(height), static_cast<SHORT>(width) };
        // Apply changes
        SetConsoleScreenBufferSize(consoleHandle, bufferSize);

        SMALL_RECT windowSize = { 0, 0, static_cast<SHORT>(height - 1), static_cast<SHORT>(width - 1) };
        SetConsoleWindowInfo(consoleHandle, TRUE, &windowSize);

    #else
        // Deepseek generated, not tested
        struct winsize ws;
        ws.ws_col = height;  // Ширина в символах
        ws.ws_row = width;  // Высота в символах

        if (ioctl(STDOUT_FILENO, TIOCSWINSZ, &ws) == -1) {
            perror("Ошибка ioctl");
        }

        // Альтернатива: escape-последовательность (если ioctl не сработал)
        cout << "\033[8;" << width << ";" << height << "t";
    #endif

}
void renderFrame(float directionA, float directionB) {
    // Precalculations
    float cosA = cos(directionA);
    float sinA = sin(directionA);
    float cosB = cos(directionB);
    float sinB = sin(directionB);

    vector<vector<char>> output(width, vector<char>(height, ' '));
    vector<vector<float>> zBuffer(width, vector<float>(height, 0));
    for (float theta = 0; theta < 2 * pi; theta += theta_interval) {
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);
        for (float phi = 0; phi < 2 * pi; phi += phi_interval) {
            float cosPhi = cos(phi);
            float sinPhi = sin(phi);

            // the x,y coordinate of the circle, before revolving (factored
            // out of the above equations)
            float circlex = R2 + R1 * cosTheta;
            float circley = R1 * sinTheta;

            // final 3D (x,y,z) coordinate after rotations, directly from
            // our math above
            float x = circlex * (cosB * cosPhi + sinA * sinB * sinPhi) - circley * cosA * sinB;
            float y = circlex * (sinB * cosPhi - sinA * cosB * sinPhi) + circley * cosA * cosB;
            float z = K2 + cosA * circlex * sinPhi + circley * sinA;
            // For faster calculations
            float oneOverZ = 1 / z;

            // x and y projection.  note that y is negated here, because y
            // goes up in 3D space but down on 2D displays.
            int xp = (int)(width / 2 + K1 * oneOverZ * x - 1);
            int yp = (int)(height / 2 - K1 * oneOverZ * y - 1);

            // calculate luminance.  ugly, but correct.
            float L = cosPhi * cosTheta * sinB - cosA * cosTheta * sinPhi - sinA * sinTheta +
                cosB * (cosA * sinTheta - cosTheta * sinA * sinPhi);
            // L ranges from -sqrt(2) to +sqrt(2).  If it's < 0, the surface
            // is pointing away from us, so we won't bother trying to plot it.
            // test against the z-buffer.  larger 1/z means the pixel is
            // closer to the viewer than what's already plotted.
            if ((xp >= width or yp >= height)) {
                std::cout << xp << " " << yp << '\n';
            }
            else if (L > 0 and oneOverZ > zBuffer[xp][yp]) {
                zBuffer[xp][yp] = oneOverZ;
                int luminanceIndex = L * 8;
                // luminance_index is now in the range 0..11 (8*sqrt(2) = 11.3)
                // now we lookup the character corresponding to the
                // luminance and plot it in our output:
                output[xp][yp] = ".,-~:;=!*#$@"[luminanceIndex];
            }
        }
    }

    std::cout << "\x1b[H";
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            cout << output[i][j];
        }
        cout << '\n';
    }
}

int main()
{
    SetConsoleWindowSize();
    float oX = 0, oY = 0;
    float speed = 0.05f;
    renderFrame(oX, oY);
    while (true) {
        char ch = _getch();
        switch (ch) {
            case 'w':
                oY += speed;
                break;
            case 's':
                oY -= speed;
                break;
            case 'a':
                oX -= speed;
                break;
            case 'd':
                oX += speed;
                break;
            case 'q':
                cout << "Program finished!\n";
                return 0;
        }
        renderFrame(oX, oY);
    }
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
