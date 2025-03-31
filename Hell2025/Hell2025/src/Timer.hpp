#include <iostream>
#include <chrono>
#include <unordered_map>
#include <format>

struct TimerResult {
    float allTimes = 0;
    float sampleCount = 0;
};
inline std::unordered_map<std::string, TimerResult> g_timerResults;

struct Timer {

    std::chrono::time_point<std::chrono::steady_clock> m_startTime;
    std::string m_name;

    Timer(const std::string& name) {
        m_startTime = std::chrono::steady_clock::now();
        m_name = name;
    }

    ~Timer() {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float, std::milli> duration = now - m_startTime;
        float time = duration.count();

        std::string spacing;
        int extraSpaces = 50 - static_cast<int>(m_name.length());
        for (int i = 0; i < extraSpaces; i++) {
            spacing += " ";
        }

        g_timerResults[m_name].allTimes += time;
        g_timerResults[m_name].sampleCount++;
        float avg = g_timerResults[m_name].allTimes / g_timerResults[m_name].sampleCount;

        std::cout << m_name << ":" << spacing
            << std::format("{:.4f}", time) << "ms      average: "
            << std::format("{:.4f}", avg) << "ms\n";
    }
};