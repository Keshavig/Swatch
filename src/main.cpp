#include <atomic>
#include <chrono>
#include <string>
#include <mutex>
#include <cassert>
#include <condition_variable>

#include <cstdio>
#include <cstdlib>
#include <thread>

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "convert.h"

constexpr const char* normalTimeStr = "H:MM:SS:CCC";
constexpr const int normalTimeStrLength = 11;

constexpr const float WIDTH = 500;
constexpr const float HEIGHT = 300;
constexpr const char* background_color = "#181818";
constexpr const float fontSizeRate = 0.25f;
constexpr const float borderThickness = 4.0f;
constexpr const char* borderColor = "#51afef";

constexpr const float buttonHorizontalPadding = 20.0f;
constexpr const float buttonVerticalPadding   = 16.0f;

constexpr const size_t NORMAL_MONITORS_WIDTH = 1920;
constexpr const size_t NORMAL_MONITORS_HEIGHT = 1080;

constexpr const size_t START = 0;
constexpr const size_t STOP = 1;
constexpr const size_t CONTINUE = 2;

constexpr const char* buttonText[] = { "Start", "Stop", "Continue" };
std::string currentButtonText = buttonText[START];

std::atomic<bool> running = false;
std::atomic<bool> exitThread = false;
std::condition_variable cv;
std::mutex mtx;

size_t centiseconds = 0;
size_t seconds = 0;
size_t minutes = 0;
size_t hours = 0;

void inline LOG(const char* x) {
    fprintf(stderr, "%s\n", x);
}

void count(void)
{
    while (!exitThread)
    {
        auto before = std::chrono::steady_clock::now();

        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, std::chrono::milliseconds(10), [] { return !running || exitThread; } );

        if (exitThread) { break; }
        if (!running) { continue; }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now-before).count() >= 10) 
        {
            before = now;
            centiseconds += 1;
            if (centiseconds < 100)
                continue;

            centiseconds = 0;
            seconds = seconds + 1;
            if (seconds < 60)
                continue;

            seconds = 0;
            minutes += 1;

            if (minutes < 60) 
                continue;

            minutes = 0;
            hours += 1; 
        }
    }
}

inline void changeCurrentText(void)
{
    if (currentButtonText == buttonText[START])          currentButtonText = buttonText[STOP];
    else if (currentButtonText == buttonText[STOP])      currentButtonText = buttonText[CONTINUE];
    else if (currentButtonText == buttonText[CONTINUE])  currentButtonText = buttonText[STOP];
}

void controlKeyboardInput(GLFWwindow* mainWindow, std::thread& theTimerThread)
{
    if (ImGui::IsKeyPressed(ImGuiKey_Space))
    {
        running = !running;
        cv.notify_one();

        changeCurrentText();
    }

    else if (ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyPressed(ImGuiKey_Q))
    {
        // Close that SHIT OFF
        exitThread = true;
        theTimerThread.join();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();

        ImGui::DestroyContext();

        glfwDestroyWindow(mainWindow);
        glfwTerminate();

        exit(0);
    }

    else if ((ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))
            && ImGui::IsKeyPressed(ImGuiKey_F))
    {
        // TODO: Does not work
        // glfwSetWindowMonitor(window, monitor, 0, 0, primaryMonitorsInfo->width, primaryMonitorsInfo->height, primaryMonitorsInfo->refreshRate);
    }

}

// TODO: Fix the broken, scaled(?) and small cursor
void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW ERROR: %d: %s\n", error, description);
}

int main() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return EXIT_FAILURE;

    const char* glsl_version = "#version 130";

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    glfwWindowHintString(GLFW_WAYLAND_APP_ID, "Swatch - A simple Stopwatch");

    // Get Primary Monitors Info
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* primaryMonitorsInfo = glfwGetVideoMode(primary);

    // NOTE: Doing this bcuz in my small old 1366:768 laptop with KDE plasma 110% scaling, this looked terrible
    float winScale = 1.0f;
    if (primaryMonitorsInfo->width >= NORMAL_MONITORS_WIDTH && primaryMonitorsInfo->height >= NORMAL_MONITORS_HEIGHT)
        winScale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

    GLFWwindow* window = glfwCreateWindow((int)(WIDTH * winScale), (int)(HEIGHT * winScale), "Swatch - Simple Stopwatch", nullptr, nullptr);
    if (!window)  return EXIT_FAILURE;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

#ifdef DECORATIONS_ENABLE
#else
    // Get rid of that closing minizing bar shit
    glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
#endif

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGuiStyle& style = ImGui::GetStyle();

    // Looks terrible, cant lie
    style.ScaleAllSizes(winScale);
    style.FontScaleDpi = winScale;

    style.WindowRounding = 0.0f;
    style.WindowBorderSize = borderThickness;
    style.Colors[ImGuiCol_Border] = colorV4(borderColor);
    style.Colors[ImGuiCol_WindowBg] = colorV4(background_color, 1.0);

    ImGuiIO& appIO = ImGui::GetIO();
    ImFont* jetbrains = appIO.Fonts->AddFontFromFileTTF("../assests/JetBrainsMonoNerdFont-Regular.ttf",
            fontSizeRate*(primaryMonitorsInfo->height));

    // Start the counter thread
    std::thread counterThread (count);

    float timerTextFontSize = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Get rid of that Opengl Window
        ImGui::SetNextWindowPos({0.0f, 0.0f});
        ImGui::SetNextWindowSize(appIO.DisplaySize);

        controlKeyboardInput(window, counterThread);
        ImGui::Begin("Cwatch - Timer", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

        GLFWmonitor* monitor = glfwGetWindowMonitor(window);

        ImVec2 winPos  = ImGui::GetWindowPos();
        ImVec2 winSize = ImGui::GetWindowSize();

        float winWidth  = winSize.x;
        float winHeight = winSize.y;

        // change font size depending on the windowsHeight
        timerTextFontSize = fontSizeRate*winHeight;
        ImGui::PushFont(jetbrains, timerTextFontSize);


        std::string stopwatchTime;
        if (hours <= 9) {
            stopwatchTime.append("0");
        }

        stopwatchTime += std::to_string(hours) + ":";

        if (minutes <= 9) {
            stopwatchTime.append("0");
        }

        stopwatchTime += std::to_string(minutes) + ":";

        if (seconds <= 9) {
            stopwatchTime.append("0");
        }

        stopwatchTime += std::to_string(seconds) + ":";

        if (centiseconds <= 9) {
            stopwatchTime.append("00");
        } else if (centiseconds >= 10 || centiseconds <= 99) {
            stopwatchTime.append("0");
        }

        stopwatchTime += std::to_string(centiseconds);

        ImVec2 timerTextFontDimensions;
        if (stopwatchTime.length() <= normalTimeStrLength) {
            timerTextFontDimensions = ImGui::CalcTextSize(normalTimeStr);
        } else {
            timerTextFontDimensions = ImGui::CalcTextSize(stopwatchTime.c_str());
        }


        /* NOTE: Make so the text fits in all width and height of a window */
        while (timerTextFontDimensions.x > winWidth)
        {

            ImGui::PopFont();
            timerTextFontSize -= fontSizeRate;
            ImGui::PushFont(jetbrains, timerTextFontSize);
            timerTextFontDimensions = ImGui::CalcTextSize(normalTimeStr);

            if (timerTextFontDimensions.x < winWidth) {
                break;
            } 
        }

        ImGui::SetCursorPos({winWidth/2 - timerTextFontDimensions.x/2, winHeight/2 - (timerTextFontDimensions.y)/2});
        ImGui::PushStyleColor(ImGuiCol_Text, colorV4("#51afef"));
        ImGui::TextUnformatted(stopwatchTime.c_str());

        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::PushFont(jetbrains, timerTextFontSize/3);

        ImGui::PushStyleColor(ImGuiCol_Text, colorV4("#16161e"));
        ImGui::PushStyleColor(ImGuiCol_Button, colorV4("#98be65"));

        const ImVec2 biggestTextSize        = ImGui::CalcTextSize(buttonText[CONTINUE]);
        const ImVec2 hourPminTextSize       = ImGui::CalcTextSize("HH:MM");
        const ImVec2 secPcentisTextSize     = ImGui::CalcTextSize("SS:CCC");

        const float buttonWidth = biggestTextSize.x + buttonHorizontalPadding;
        const float buttonHeight = biggestTextSize.y + buttonVerticalPadding;

        const float statusButtonPosx = winWidth/2 - biggestTextSize.x - buttonHorizontalPadding/2 - hourPminTextSize.x/3;
        const float statusButtonPosy = winHeight/2 + (timerTextFontDimensions.y)/2 + biggestTextSize.y/2;

        bool drawButton = true;
        ImGui::SetCursorPos(ImVec2(statusButtonPosx, statusButtonPosy));

        if (drawButton && ImGui::Button(currentButtonText.c_str(), ImVec2(buttonWidth, buttonHeight)))
        {
            running = !running;
            cv.notify_one();
            changeCurrentText();
        }

        const ImVec2 resetTextSize = ImGui::CalcTextSize("Reset");
        ImGui::SetCursorPos(ImVec2(winWidth/2 - buttonHorizontalPadding + resetTextSize.x/2 + secPcentisTextSize.x/4, statusButtonPosy));

        ImGui::PushStyleColor(ImGuiCol_Button, colorV4("#ff6c6b"));
        if (ImGui::Button("Reset", ImVec2(resetTextSize.x + 2*buttonHorizontalPadding, resetTextSize.y + buttonVerticalPadding)))
        {
            centiseconds = 0;
            seconds      = 0;
            minutes      = 0;
            hours        = 0;

            if (running) {
                running = !running;
                cv.notify_one();
            }

            currentButtonText = buttonText[START];
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::End();
        ImGui::Render();

        int display_w, display_h;

        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    exitThread = true;
    counterThread.join();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
